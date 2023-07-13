import os
import csv
import codecs
import time
import random
import pandas as pd
import requests
import logging
import logging.handlers
import datetime
from bs4 import BeautifulSoup
from threading import Thread
import multiprocessing
from clickhouse_driver import Client

# Arguments
input_folder_path = 'xxx'   # path to folder
output_folder_path = 'xxx' 
n_thread = 4
n_processes = 16


class data:
    def __init__(self, n, k):
        self.abstr_list = [None]*n
        self.claims_list = [None]*n
        self.clas_list = [None]*n

        self.id_list = [[] for _ in range(k)]
        self.pub_number_list = [[] for _ in range(k)]
        self.pri_date_list = [[] for _ in range(k)]
        self.pub_date_list = [[] for _ in range(k)]
        self.assignee_list = [[] for _ in range(k)]
        self.title_list = [[] for _ in range(k)]

headers_list = [
    {
        'user-agent': 'Mozilla/5.0 (iPhone; CPU iPhone OS 13_2_3 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/13.0.3 Mobile/15E148 Safari/604.1'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 8.0.0; SM-G955U Build/R16NW) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.141 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 10; SM-G981B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.162 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (iPad; CPU OS 13_3 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) CriOS/87.0.4280.77 Mobile/15E148 Safari/604.1'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 8.0; Pixel 2 Build/OPD3.170816.012) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.109 Safari/537.36 CrKey/1.54.248666'
    }, {
        'user-agent': 'Mozilla/5.0 (X11; Linux aarch64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.188 Safari/537.36 CrKey/1.54.250320'
    }, {
        'user-agent': 'Mozilla/5.0 (BB10; Touch) AppleWebKit/537.10+ (KHTML, like Gecko) Version/10.0.9.2372 Mobile Safari/537.10+'
    }, {
        'user-agent': 'Mozilla/5.0 (PlayBook; U; RIM Tablet OS 2.1.0; en-US) AppleWebKit/536.2+ (KHTML like Gecko) Version/7.2.1.0 Safari/536.2+'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; U; Android 4.3; en-us; SM-N900T Build/JSS15J) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; U; Android 4.1; en-us; GT-N7100 Build/JRO03C) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; U; Android 4.0; en-us; GT-I9300 Build/IMM76D) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 7.0; SM-G950U Build/NRD90M) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.84 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 8.0.0; SM-G965U Build/R16NW) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/63.0.3239.111 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 8.1.0; SM-T837A) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.80 Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; U; en-us; KFAPWI Build/JDQ39) AppleWebKit/535.19 (KHTML, like Gecko) Silk/3.13 Safari/535.19 Silk-Accelerated=true'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; U; Android 4.4.2; en-us; LGMS323 Build/KOT49I.MS32310c) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/102.0.0.0 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Windows Phone 10.0; Android 4.2.1; Microsoft; Lumia 550) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2486.0 Mobile Safari/537.36 Edge/14.14263'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 6.0.1; Moto G (4)) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 6.0.1; Nexus 10 Build/MOB31T) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 4.4.2; Nexus 4 Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 8.0.0; Nexus 5X Build/OPR4.170623.006) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 7.1.1; Nexus 6 Build/N6F26U) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 8.0.0; Nexus 6P Build/OPP3.170518.006) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 6.0.1; Nexus 7 Build/MOB30X) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (compatible; MSIE 10.0; Windows Phone 8.0; Trident/6.0; IEMobile/10.0; ARM; Touch; NOKIA; Lumia 520)'
    }, {
        'user-agent': 'Mozilla/5.0 (MeeGo; NokiaN9) AppleWebKit/534.13 (KHTML, like Gecko) NokiaBrowser/8.5.0 Mobile Safari/534.13'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 9; Pixel 3 Build/PQ1A.181105.017.A1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/66.0.3359.158 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 10; Pixel 4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 11; Pixel 3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.181 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 5.0; SM-G900P Build/LRX21T) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 8.0; Pixel 2 Build/OPD3.170816.012) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (Linux; Android 8.0.0; Pixel 2 XL Build/OPD1.170816.004) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Mobile Safari/537.36'
    }, {
        'user-agent': 'Mozilla/5.0 (iPhone; CPU iPhone OS 10_3_1 like Mac OS X) AppleWebKit/603.1.30 (KHTML, like Gecko) Version/10.0 Mobile/14E304 Safari/602.1'
    }, {
        'user-agent': 'Mozilla/5.0 (iPhone; CPU iPhone OS 13_2_3 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/13.0.3 Mobile/15E148 Safari/604.1'
    }, {
        'user-agent': 'Mozilla/5.0 (iPad; CPU OS 11_0 like Mac OS X) AppleWebKit/604.1.34 (KHTML, like Gecko) Version/11.0 Mobile/15A5341f Safari/604.1'
    }
]


# helper functions
# get abstract
def get_abstract(soup):
    abstract = soup.find('div', {'class': 'abstract'})
    if abstract:
        return abstract.text.strip()
    else:
        return ""

def get_claims_num(soup):
    claims = soup.find('section', {'itemprop': 'claims'})
    if claims:
        num = claims.find('span', {'itemprop': "count"})
        return num.text.strip()
    else:
        return ""

def get_classifications(soup):
    uls = soup.find_all('ul', {'itemprop': 'cpcs'})
    if uls:
        classification_list = []
        for ul in uls:
            lis = ul.find_all('li')
            last_li = lis[-1].text
            classification_list.append(last_li.split('—')[0].strip())
        return ", ".join(classification_list)
    else:
        return ""

def get_date_ctr(input_file):
    f = open(input_file,'r', encoding="utf-8")
    url = f.readline().split(',')[1]
    cuts = url.split('=priority:')
    ctr = url.split('country=')[1][:2]
    f.close()
    return cuts[2][:8] + '-' + cuts[1][:8] + '-' + ctr

def get_date(input_file):
    """
    return: e.g. 2022-01-01
    """
    f = open(input_file,'r', encoding="utf-8")
    url = f.readline().split(',')[1]
    cuts = url.split('=priority:')
    return cuts[2][:4] + '-' + cuts[2][4:6] + '-' + cuts[2][6:8]

def filter(pub_number):
    if '-' in pub_number:
        return pub_number
    # pub_number = pub_number.replace('*', '') # 去除*
    # pub_number = pub_number.replace('†', '') # 去除†
    # pub_number = pub_number.strip() # 去除首尾空格
    cty =  pub_number[:2]
    idx = 2
    while pub_number[idx].isdigit():
        idx += 1
    num = pub_number[2:idx]
    ed = pub_number[idx:]
    return cty + '-' + num + '-' + ed

def save_common(abstr_list, claims_list, clas_list, input_file):
    file_name = get_date_ctr(input_file)
    output_file = generate_path_name(output_folder_path, file_name, '-common')
    f = pd.read_csv(input_file, skiprows = 1, encoding="utf-8")
    f['abstract'] = abstr_list
    f['claims_num'] = claims_list
    f['classifications'] = clas_list
    f.to_csv(output_file, index=False)

def save_cite(id_list, pub_number_list, pri_date_list, pub_date_list, assignee_list, title_list, input_file):
    file_name = get_date_ctr(input_file)
    output_file = generate_path_name(output_folder_path, file_name, '-cite')
    df = pd.DataFrame({
        'Id': id_list,
        'Publication number': pub_number_list,
        'Priority date': pri_date_list,
        'Publication date': pub_date_list,
        'Assignee': assignee_list,
        'Title': title_list
    })
    df.to_csv(output_file, index=False, encoding='utf-8')
    # print("finish save to", output_file)

def generate_path_name(path, name, middle = ''):
    return os.path.join(path, name + middle + '.csv')

def get_soup(url, logger):
    code = 0
    count = 1
    while count < 100:  # 循环去请求网站
        time.sleep(1)
        try:
            header = random.choice(headers_list)
            response = requests.get(url, headers=header, timeout=10)  
            code = response.status_code
            response.encoding = 'utf-8'
            soup = BeautifulSoup(response.text, 'html.parser')
            if (code == 200) and (get_abstract(soup) != "" or get_claims_num(soup) != "" or get_classifications(soup) != ""):
                break
        except:
            # print("Error:", code)
            cnt = str(datetime.datetime.now()) + ": Error in get {}, retry... ({})".format(url, count)
            logger.warning(cnt)
        count += 1
    if count >= 100:
        cnt = str(datetime.datetime.now()) + "Error in get {}, give up.".format(url)
        logger.warning(cnt)
        return -1
    return soup

def db2dict_common(date_str):
    """
    get common data from database and store them in the dict

    date_str: e.g. 2022-01-01
    """
    client = Client(host='xx.xx.x.xx', port='xxx', database='xxx', user='xxx', password='xxx')
    query_common = f"select * from xxx.google_patent_data_common where priority_date = '{date_str}'"
    data_common = client.execute(query_common)
    dict_common = {}
    for line in data_common:
        dict_common[line[8]] = line
    return dict_common

def db2dict_cite():
    """
    get cite data
    """
    client = Client(host='xx.xx.x.x', port='xxx', database='xxx', user='xxx', password='xxx')

    query_cite_id = "select distinct patent_id from xxx.google_patent_data_cite where patent_id like 'CN%'"
    data_cite_id = client.execute(query_cite_id)
    dict_cite = {}
    for id in data_cite_id:
        dict_cite[id[0]] = []

    query_cite = "select * from xxx.google_patent_data_cite where patent_id like 'CN%'"
    data_cite = client.execute(query_cite)
    
    for line in data_cite:
        dict_cite[line[0]].append(line)
    return dict_cite

def operate(pid, tid, rows, chunks, file_name, logger, dict_cite, dict_common, d: data):
   
    for i in range(chunks[tid][0], chunks[tid][1]):
        url = rows[i][8]
        id = rows[i][0]

        # get from database
        if url in dict_common.keys():
            d.abstr_list[i] = dict_common[url][10]
            d.claims_list[i] = dict_common[url][11]
            d.clas_list[i] = dict_common[url][12]

            if id in dict_cite.keys():
                for line in dict_cite[id]:
                    d.id_list[tid].append(id)
                    d.pub_number_list[tid].append(line[1])
                    d.pri_date_list[tid].append(line[2])
                    d.pub_date_list[tid].append(line[3])
                    d.assignee_list[tid].append(line[4])
                    d.title_list[tid].append(line[5])
            cnt = str(datetime.datetime.now()) + ': process '+ str(pid) + ' - thread ' + str(tid) + ' -> ' + file_name + ': finish line ' + str(i) + " - " + id + " (from database)"
            
            logger.info(cnt)
            continue

        # get from website
        soup = get_soup(url, logger)
        if soup == -1:
            d.abstr_list[i] = ""
            d.claims_list[i] = ""
            d.clas_list[i] = ""
            cnt = str(datetime.datetime.now()) + ': process '+ str(pid) + ' - thread ' + str(tid) + ' -> ' + file_name + ': skip line ' + str(i)
            logger.info(cnt)
            continue
        
        # operate common
        d.abstr_list[i] = get_abstract(soup)
        d.claims_list[i] = get_claims_num(soup)
        d.clas_list[i] = get_classifications(soup)

        # operate cite
        try:
            cited = soup.find_all('tr', {'itemprop': "forwardReferencesFamily"})
            if (len(cited) == 0):
                cited = soup.find_all('tr', {'itemprop': "forwardReferencesOrig"})
            for item in cited:
                pub_number = filter(item.find('span', {"itemprop":"publicationNumber"}).text.strip())
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
        print(pid, ": An error occurred:", str(e))
    print(pid, "finish setting logger")

    dict_cite = db2dict_cite()

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
            d = data(num_data, n_thread)
            dict_common = db2dict_common(get_date(file_path))
            # print("new d: ", len(d.id_list), len(d.pub_number_list))
            chunks = generate_chunks(num_data, n_thread)
            # print(num_data)
            # print(chunks)

            threads_pool = [None] * n_thread
            
            for i in range(n_thread):
                threads_pool[i] = Thread(target=operate, args=(pid, i, rows, chunks, file_name, logger_pool[i], dict_cite, dict_common, d))
                threads_pool[i].start()

            for thread in threads_pool:
                thread.join()

            cnt = str(datetime.datetime.now()) + ': ' + file_name + ': finish analysis, save - common'
            logger_pool[0].info(cnt)
            save_common(d.abstr_list, d.claims_list, d.clas_list, file_path)
            
            d.id_list = combine(d.id_list)
            d.pub_number_list = combine(d.pub_number_list)
            d.pri_date_list = combine(d.pri_date_list)
            d.pub_date_list = combine(d.pub_date_list)
            d.assignee_list = combine(d.assignee_list)
            d.title_list = combine(d.title_list)
            cnt = str(datetime.datetime.now()) + ': ' + file_name + ': finish analysis, save - cite'
            logger_pool[0].info(cnt)
            save_cite(d.id_list, d.pub_number_list, d.pri_date_list, d.pub_date_list, d.assignee_list, d.title_list, file_path)


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


