import config
from utils import analysis_helper

import os
import csv
import codecs
import logging
import logging.handlers
import datetime
from threading import Thread
import multiprocessing

# Arguments
input_folder_path = config.step2_input_folder_path
output_folder_path = config.step2_output_folder_path
n_thread = config.step2_n_threads
n_processes = config.step2_n_processes

def operate(pid, tid, rows, chunks, file_name, logger, d: analysis_helper.data):
   
    for i in range(chunks[tid][0], chunks[tid][1]):
        url = rows[i][8]
        id = rows[i][0]

        # get from website
        soup = analysis_helper.get_soup(url, logger)
        if soup == -1:
            d.abstr_list[i] = ""
            d.claims_list[i] = ""
            d.clas_list[i] = ""
            cnt = str(datetime.datetime.now()) + ': process '+ str(pid) + ' - thread ' + str(tid) + ' -> ' + file_name + ': skip line ' + str(i)
            logger.info(cnt)
            continue
        
        # operate common
        d.abstr_list[i] = analysis_helper.get_abstract(soup)
        d.claims_list[i] = analysis_helper.get_claims_num(soup)
        d.clas_list[i] = analysis_helper.get_classifications(soup)

        # operate cite
        try:
            cited = soup.find_all('tr', {'itemprop': "forwardReferencesFamily"})
            if (len(cited) == 0):
                cited = soup.find_all('tr', {'itemprop': "forwardReferencesOrig"})
            for item in cited:
                pub_number = analysis_helper.filter(item.find('span', {"itemprop":"publicationNumber"}).text.strip())
                pri_date = item.find('td', {"itemprop":"priorityDate"}).text.strip()
                pub_date = item.find('td', {"itemprop":"publicationDate"}).text.strip()
                assignee = item.find('span', {"itemprop":"assigneeOriginal"}).text.strip()
                title = item.find('td', {"itemprop":"title"}).text.strip()

                d.id_list[tid].append(id)
                d.pub_number_list[tid].append(pub_number)
                d.pri_date_list[tid].append(pri_date)
                d.pub_date_list[tid].append(pub_date)
                d.assignee_list[tid].append(assignee)
                d.title_list[tid].append(title)
            # print('line', i, 'has cite')
        except:
            pass
        cnt = str(datetime.datetime.now()) + ': process '+ str(pid) + ' - thread ' + str(tid) + ' -> ' + file_name + ': finish line ' + str(i) + " - " + id + " (from website)"
        logger.info(cnt)

        # print('thread', tid, '->', file_name,': finish line', i)
        
    cnt = str(datetime.datetime.now()) + ': process '+ str(pid) + ' - thread ' + str(tid) + ' -> ' + file_name + ': finish job.'
    logger.info(cnt)

def generate_chunks(n, k):
    chunk_size = n // k
    r = n % k
    chunks = []
    start = 0
    end = 0
    for i in range(r):
        start = end
        end += chunk_size + 1
        chunks.append((start, end))
    for i in range(r, k):
        start = end
        end += chunk_size
        chunks.append((start, end))
    return chunks

def combine(list_of_lists):
    res = []
    for l in list_of_lists:
        res.extend(l)
    return res

def worker(pid, queue):
    # print(pid, "start working")
    try:
        logger_pool = [None] * n_thread
        # initialize logging
        logging.basicConfig()
        for tid in range (n_thread):
            # initialize logger
            logger_pool[tid] = logging.getLogger('logger{}'.format(10 * pid + tid))
            # write in file，maximum 1MB，back up 5 files。
            handler = logging.handlers.RotatingFileHandler(
                'log/process_{}.log'.format(10 * pid + tid), maxBytes=1e6, backupCount=5)
            logger_pool[tid].setLevel(logging.DEBUG)
            logger_pool[tid].addHandler(handler)
    except Exception as e:
        print(pid, ": An error occurred :", str(e))
    print(pid, "finish setting logger")

    while not queue.empty():
        file_name = queue.get()
        file_path = os.path.join(input_folder_path, file_name)   # construct the full path
        
        if os.path.isfile(file_path):   # Test whether a path is a regular file
            num_data = 0
            with codecs.open(file_path, "r", "utf-8") as f:
                reader = csv.reader(f)
                rows = list(reader)[2:]
                num_data = len(rows) # number of lines of data 
            if num_data <= 0:
                continue
            d = analysis_helper.data(num_data, n_thread)
            # print("new d: ", len(d.id_list), len(d.pub_number_list))
            chunks = generate_chunks(num_data, n_thread)
            # print(num_data)
            # print(chunks)

            threads_pool = [None] * n_thread
            
            for i in range(n_thread):
                threads_pool[i] = Thread(target=operate, args=(pid, i, rows, chunks, file_name, logger_pool[i], d))
                threads_pool[i].start()

            for thread in threads_pool:
                thread.join()

            cnt = str(datetime.datetime.now()) + ': ' + file_name + ': finish analysis, save - common'
            logger_pool[0].info(cnt)
            analysis_helper.save_common(d.abstr_list, d.claims_list, d.clas_list, file_path, output_folder_path)
            
            d.id_list = combine(d.id_list)
            d.pub_number_list = combine(d.pub_number_list)
            d.pri_date_list = combine(d.pri_date_list)
            d.pub_date_list = combine(d.pub_date_list)
            d.assignee_list = combine(d.assignee_list)
            d.title_list = combine(d.title_list)
            cnt = str(datetime.datetime.now()) + ': ' + file_name + ': finish analysis, save - cite'
            logger_pool[0].info(cnt)
            analysis_helper.save_cite(d.id_list, d.pub_number_list, d.pri_date_list, d.pub_date_list, d.assignee_list, d.title_list, file_path, output_folder_path)


if __name__ == '__main__':
    try:
        os.mkdir('log/') 
    except Exception as e:
        print('errer',e)
    file_names = os.listdir(input_folder_path)
    # file_names = ['US_2021/20210101-20210101-US.csv']

    print('set multirocessing...')

    pool = multiprocessing.Pool(processes=n_processes)
    manager = multiprocessing.Manager()
    queue = manager.Queue()

    for ele in file_names:
        queue.put(ele)

    for i in range(n_processes):
        pool.apply_async(worker, (i, queue))

    pool.close()
    pool.join()


