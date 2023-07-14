import config
from utils import analysis_helper

import os
import base64
import time
import json
import random
import pandas as pd
import requests
import logging
import logging.handlers
import datetime
from bs4 import BeautifulSoup
from threading import Thread
from clickhouse_driver import Client

# Arguments
output_folder_path = config.step5_output_folder_path
n_threads = config.step5_n_threads

class data:
    def __init__(self, n, k):
        self.cid_list = [None]*n
        self.tit_list = [None]*n
        self.ass_list = [None]*n
        self.inv_list = [None]*n
        self.pri_list = [None]*n
        self.fil_list = [None]*n
        self.pub_list = [None]*n
        self.gra_list = [None]*n
        self.res_list = [None]*n
        self.rep_list = [None]*n
        self.abstr_list = [None]*n
        self.claims_list = [None]*n
        self.clas_list = [None]*n

        self.id_list = [[] for _ in range(k)]
        self.pub_number_list = [[] for _ in range(k)]
        self.pri_date_list = [[] for _ in range(k)]
        self.pub_date_list = [[] for _ in range(k)]
        self.assignee_list = [[] for _ in range(k)]
        self.title_list = [[] for _ in range(k)]

def save_file(d: data):
    # save common
    output_file = analysis_helper.generate_path_name(output_folder_path, 'replenish', '-common')
    df = pd.DataFrame({
        'patent_id': d.cid_list,
        'title': d.tit_list,
        'assignee': d.ass_list,
        'inventor_author': d.inv_list,
        'priority_date': d.pri_list,
        'filing_creation_date': d.fil_list,
        'publication_date': d.pub_list,
        'grant_date': d.gra_list,
        'result_link': d.res_list,
        'representative_figure_link': d.rep_list,
        'abstract': d.abstr_list,
        'claims_num': d.claims_list,
        'classifications': d.clas_list
    })
    df.to_csv(output_file, index=False, encoding='utf-8')

    # save cite
    output_file = analysis_helper.generate_path_name(output_folder_path, 'replenish', '-cite')
    df = pd.DataFrame({
        'Id': d.id_list,
        'Publication number': d.pub_number_list,
        'Priority date': d.pri_date_list,
        'Publication date': d.pub_date_list,
        'Assignee': d.assignee_list,
        'Title': d.title_list
    })
    df.to_csv(output_file, index=False, encoding='utf-8')

def get_soup(id, logger): 
    code = 0
    count = 1
    while count < 100:  # 循环去请求网站
        time.sleep(1)
        try:
            header = random.choice(config.headers_list)
            url = f"https://patents.google.com/xhr/parse?text={id}&cursor=14&exp="
            response = requests.get(url, headers=header, timeout=10)  
            response.encoding = 'utf-8'
            if json.loads(response.text)['error_no_patents_found'] == True:
                cnt = str(datetime.datetime.now()) + ": Error in get {}, error_no_patents_found".format(url)
                logger.warning(cnt)
                return -1, -1
            url = "https://patents.google.com/{}".format(json.loads(response.text)['results'][0]['result']['id'])

            response = requests.get(url, headers=header, timeout=10)  
            code = response.status_code
            response.encoding = 'utf-8'
            soup = BeautifulSoup(response.text, 'html.parser')
            if (code == 200) and (analysis_helper.get_abstract(soup) != "" or analysis_helper.get_claims_num(soup) != "" or analysis_helper.get_classifications(soup) != ""):
                break
        except:
            # print("Error:", code)
            cnt = str(datetime.datetime.now()) + ": Error in get {}, retry... ({})".format(url, count)
            # print(cnt)
            logger.warning(cnt)
        count += 1
    if count >= 100:
        cnt = str(datetime.datetime.now()) + ": Error in get {}, give up.".format(url)
        # print(cnt)
        logger.warning(cnt)
        return -1, -1
    return soup, url

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

def operate(tid, to_add, chunks, d: data):
    # initialize logging
    logging.basicConfig()
    # initialize logger
    logger = logging.getLogger('logger{}'.format(tid))
    # write in file，maximum 1MB，back up 5 files。
    handler = logging.handlers.RotatingFileHandler(
        'log/process_{}.log'.format(tid), maxBytes=1e6, backupCount=5)
    logger.setLevel(logging.DEBUG)
    logger.addHandler(handler)

    for i in range(chunks[tid][0], chunks[tid][1]):
        id = to_add[i]
        soup, url = get_soup(id, logger)

        if soup == -1:
            d.cid_list[i] = id
            d.tit_list[i] = ""
            d.ass_list[i] = ""
            d.inv_list[i] = ""
            d.pri_list[i] = ""
            d.fil_list[i] = ""
            d.pub_list[i] = ""
            d.gra_list[i] = ""
            d.res_list[i] = ""
            d.rep_list[i] = ""
            d.abstr_list[i] = ""
            d.claims_list[i] = ""
            d.clas_list[i] = ""
            cnt = str(datetime.datetime.now()) + ': thread ' + str(tid) + ': skip line ' + str(i)
            logger.info(cnt)
            continue

        # operate common
        d.cid_list[i] = id
        d.tit_list[i] = analysis_helper.get_title(soup)
        d.ass_list[i] = analysis_helper.get_assignee(soup)
        d.inv_list[i] = analysis_helper.get_inventor(soup)
        d.pri_list[i] = analysis_helper.get_pri_date(soup)
        d.fil_list[i] = analysis_helper.get_fil_date(soup)
        d.pub_list[i] = analysis_helper.get_pub_date(soup, id)
        d.gra_list[i] = analysis_helper.get_gra_date(soup)
        d.res_list[i] = url
        d.rep_list[i] = ""
        d.abstr_list[i] = analysis_helper.get_abstract(soup)
        d.claims_list[i] = analysis_helper.get_claims_num(soup)
        d.clas_list[i] = analysis_helper.get_classifications(soup)
        # print(f"d.cid_list[i] = {d.cid_list[i]}, d.tit_list[i] = {d.tit_list[i]}, d.ass_list[i] = {d.ass_list[i]}, d.inv_list[i] = {d.inv_list[i]}, d.pri_list[i] = {d.pri_list[i]}, d.fil_list[i] = {d.fil_list[i]}, d.pub_list[i] = {d.pub_list[i]}, d.gra_list[i] = {d.gra_list[i]}, d.res_list[i] = {d.res_list[i]}, d.rep_list[i] = {d.rep_list[i]}, d.abstr_list[i] = {d.abstr_list[i]}, d.claims_list[i] = {d.claims_list[i]}, d.clas_list[i] = {d.clas_list[i]}")

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
        cnt = str(datetime.datetime.now()) + ': thread ' + str(tid) + ': finish line ' + str(i) + " - " + id
        logger.info(cnt)

if __name__ == '__main__':
    #----------------------------------------------------------------------------------------------------------------------------------------------------------
    print("get data")

    client = Client(host=config.host22, port=config.port22, database=config.database22, user=config.user22, password=base64.b64decode(config.password22).decode('utf-8'))

    # 从数据库获取数据并存入set
    query_cited_by_id = "select distinct cited_by_patent_id from his_data_snaps.google_patent_data_cite where cited_by_patent_id like 'CN%'"
    df_cited_by_id = client.execute(query_cited_by_id)
    cited_by_id_set = {id[0] for id in df_cited_by_id}

    query_common_id = 'select distinct patent_id from his_data_snaps.google_patent_data_common'
    df_common_id = client.execute(query_common_id)
    common_id_set = {id[0] for id in df_common_id}

    client.disconnect()

    print("start check")

    count = 0
    to_add = []

    # check: cite表的cited_by_patent_id中以CN开头的部分是否在common表都有
    for id in cited_by_id_set:
        if id not in common_id_set:
            count += 1
            to_add.append(id)
            print('check 2 error:', id, ", count =", count)

    #----------------------------------------------------------------------------------------------------------------------------------------------------------
    print("start crawling")
    # to_add = ['CN-114640860-A', 'CN-114181065-A', 'CN-109371072-A', 'CN-114946382-A']
    
    try:
        os.mkdir('log/') 
    except Exception as e:
        print('errer',e) 

    num_data = len(to_add) # number of lines of data 
    d = data(num_data, n_threads)
    chunks = generate_chunks(num_data, n_threads)

    threads_pool = [None] * n_threads
    for i in range(n_threads):
        threads_pool[i] = Thread(target=operate, args=(i, to_add, chunks, d))
        threads_pool[i].start()

    for thread in threads_pool:
        thread.join()
    d.id_list = combine(d.id_list)
    d.pub_number_list = combine(d.pub_number_list)
    d.pri_date_list = combine(d.pri_date_list)
    d.pub_date_list = combine(d.pub_date_list)
    d.assignee_list = combine(d.assignee_list)
    d.title_list = combine(d.title_list)

    # print(d.cid_list)
    save_file(d)