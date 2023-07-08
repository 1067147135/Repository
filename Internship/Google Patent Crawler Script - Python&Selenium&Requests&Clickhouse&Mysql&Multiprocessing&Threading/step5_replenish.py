import os
import re
import csv
import codecs
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
import multiprocessing
from lxml import etree
from clickhouse_driver import Client

#----------------------------------------------------------------------------------------------------------------------------------------------------------
print("get data")

client = Client(host='xx.xx.x.22', port='9000', database='xxx', user='xxx', password='xxx')

# 从数据库获取数据并存入set
query_cited_by_id = "select distinct cited_by_patent_id from xxx.google_patent_data_cite where cited_by_patent_id like 'CN%'"
df_cited_by_id = client.execute(query_cited_by_id)
cited_by_id_set = {id[0] for id in df_cited_by_id}

query_common_id = 'select distinct patent_id from his_data_snaps.google_patent_data_common'
df_common_id = client.execute(query_common_id)
common_id_set = {id[0] for id in df_common_id}

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

output_folder_path = '/home/swl/bopu/extra'

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

# get functions
def get_title(soup):
    # 查找第一个符合条件的<meta>元素
    meta_element = soup.find('meta', {'name': 'DC.title'})
    if meta_element:
        # 获取content属性的值
        return meta_element.get('content').strip()
    else:
        return ""

def get_assignee(soup):
    meta_elements = soup.find_all('meta', {'scheme': 'assignee'})
    if meta_elements:
        # 获取content属性的值
        ass_list = []
        for ele in meta_elements:
            ass_list.append(ele.get('content').strip())
        return ", ".join(ass_list)
    else:
        return ""

def get_inventor(soup):
    meta_elements = soup.find_all('meta', {'scheme': 'inventor'})
    if meta_elements:
        # 获取content属性的值
        inv_list = []
        for ele in meta_elements:
            inv_list.append(ele.get('content').strip())
        return ", ".join(inv_list)
    else:
        return ""

def get_pri_date(soup):
    priority_date_element = soup.find(string = re.compile(r"(\s\w+)?riority to(\s\w+)?"))
    if priority_date_element:
        priority_date = priority_date_element.find_previous('time').text.strip()
        return priority_date
    else:
        return ""
    
def get_fil_date(soup):
    filing_date_element = soup.find(string = re.compile(r"(\s\w+)?filed by(\s\w+)?"))
    if filing_date_element:
        filing_date = filing_date_element.find_previous('time').text.strip()
        return filing_date
    else:
        return ""

def get_pub_date(soup, id):
    publication_date_element = soup.find(string = re.compile(r"(\s\w+)?ublication of {}(\s\w+)?".format(id.replace('-', ''))))
    if publication_date_element:
        publication_date = publication_date_element.find_previous('time').text.strip()
        return publication_date
    else:
        return ""

def get_gra_date(soup):
    grant_date_element = soup.find(string = re.compile(r"(\s\w+)?granted(\s\w+)?"))
    if grant_date_element:
        grant_date = grant_date_element.find_previous('time').text.strip()
        return grant_date
    else:
        return ""

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

def filter(pub_number):
    if '-' in pub_number:
        return pub_number
    cty =  pub_number[:2]
    idx = 2
    while pub_number[idx].isdigit():
        idx += 1
    num = pub_number[2:idx]
    ed = pub_number[idx:]
    return cty + '-' + num + '-' + ed

def generate_path_name(path, name, middle = ''):
    return os.path.join(path, name + middle + '.csv')

def save_file(d: data):
    # save common
    output_file = generate_path_name(output_folder_path, 'replenish', '-common')
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
    output_file = generate_path_name(output_folder_path, 'replenish', '-cite')
    df = pd.DataFrame({
        'Id': d.id_list,
        'Publication number': d.pub_number_list,
        'Priority date': d.pri_date_list,
        'Publication date': d.pub_date_list,
        'Assignee': d.assignee_list,
        'Title': d.title_list
    })
    df.to_csv(output_file, index=False, encoding='utf-8')

def get_soup(id, logger): # 
    code = 0
    count = 1
    while count < 100:  # 循环去请求网站
        time.sleep(1)
        try:
            header = random.choice(headers_list)
            url = f"https://patents.google.com/xhr/parse?text={id}&cursor=14&exp="
            response = requests.get(url, headers=header, timeout=10)  
            response.encoding = 'utf-8'
            url = "https://patents.google.com/{}".format(json.loads(response.text)['results'][0]['result']['id'])

            response = requests.get(url, headers=header, timeout=10)  
            code = response.status_code
            response.encoding = 'utf-8'
            soup = BeautifulSoup(response.text, 'html.parser')
            if (code == 200) and (get_abstract(soup) != "" or get_claims_num(soup) != "" or get_classifications(soup) != ""):
                break
        except:
            # print("Error:", code)
            cnt = str(datetime.datetime.now()) + ": Error in get {}, retry... ({})".format(url, count)
            # print(cnt)
            logger.warning(cnt)
        count += 1
    if count >= 100:
        cnt = str(datetime.datetime.now()) + "Error in get {}, give up.".format(url)
        # print(cnt)
        logger.warning(cnt)
        return -1
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
            d.res_list[i] = url
            d.rep_list[i] = ""
            d.abstr_list[i] = ""
            d.claims_list[i] = ""
            d.clas_list[i] = ""
            cnt = str(datetime.datetime.now()) + ': thread ' + str(tid) + ': skip line ' + str(i)
            logger.info(cnt)
            continue

        # operate common
        d.cid_list[i] = id
        d.tit_list[i] = get_title(soup)
        d.ass_list[i] = get_assignee(soup)
        d.inv_list[i] = get_inventor(soup)
        d.pri_list[i] = get_pri_date(soup)
        d.fil_list[i] = get_fil_date(soup)
        d.pub_list[i] = get_pub_date(soup, id)
        d.gra_list[i] = get_gra_date(soup)
        d.res_list[i] = url
        d.rep_list[i] = ""
        d.abstr_list[i] = get_abstract(soup)
        d.claims_list[i] = get_claims_num(soup)
        d.clas_list[i] = get_classifications(soup)
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

# to_add = ['CN-114640860-A', 'CN-114181065-A', 'CN-109371072-A', 'CN-114946382-A']
n_threads = 8
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