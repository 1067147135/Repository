import config

import base64
import pymysql
import datetime
from clickhouse_driver import Client
import pandas as pd
import re
import os
import warnings
import logging
import logging.handlers
import multiprocessing
from multiprocessing import Process, Array

warnings.filterwarnings("ignore")

# Arguments
output_path = config.step6_output_folder_path
n_processes = config.step6_n_threads

def has_chinese(string):
    """
    判断字符串是否包含汉字 -> bool
    """
    pattern = re.compile(r'[\u4e00-\u9fa5]')
    match = pattern.search(string)
    return match is not None

def contains_digit(s):
    pattern = r"\d"
    match = re.search(pattern, s)
    return match is not None

def split_assignee(string):
    """
    当存在多个assignee的时候进行切分 -> list
    英文转小写版本
    """
    res = []
    if has_chinese(string):
        # Company name in Chinese
        string = string.replace('（Cn）', '')
        string = string.replace('（', '(')
        string = string.replace('）', ')')

        res = string.split(", ")
        
    else:
        # Company name in English
        string = string.lower().strip()
        
        string = string.replace(', inc.', '')
        string = string.replace(', inc', '')
        string = string.replace(', llc', '')
        string = string.replace(', n.a.', '')
        string = string.replace(', ltd.', '')
        string = string.replace(', l.p.', '')
        string = string.replace(' co., ltd.', '')
        string = string.replace(' co., ltd', '')
        string = string.replace(' co.,ltd.', '')
        string = string.replace(' co.,ltd', '')
        string = string.replace(' co, ltd.', '')
        string = string.replace(' co, ltd', '')

        res = string.split(", ")
    for i in range(len(res)):
        res[i] = res[i].replace(' corp', '')
        res[i] = res[i].replace(' co.ltd.', '')
        res[i] = res[i].replace(' co ltd', '')
        res[i] = res[i].replace(' co', '')
        res[i] = res[i].replace(' ltd', '')
        res[i] = res[i].strip()
    return res

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

def save(ids, codes, chunk):
    df = pd.DataFrame({
        'patent_id': ids,
        'company_code': codes
    })
    file_name = os.path.join(output_path, str(chunk[0]) + "-" + str(chunk[1]) + '.csv')
    df.to_csv(file_name, index=False, encoding='utf-8')

def choose(target, keys):
    if ('university' in target) or ('大学' in target):
        for key in keys:
            # print(key)
            if target == split_assignee(key)[0]:
                return key
        return ''
    if (len(target) < 5) and has_chinese(target):
        return ''    

    candidate = ''
    lenth = 1e9
    for key in keys:
        # print(key)
        if target in key:
            if (len(key) < lenth):
                candidate = key
                lenth = len(candidate)
    return candidate

def worker(pid, chunk, id_list, ass_list, des_dict):
    logging.basicConfig()
    logger = logging.getLogger(f'logger{pid}')
    # write in file，maximum 1MB，back up 5 files。
    handler = logging.handlers.RotatingFileHandler(
        f'log/linker{pid}.log', maxBytes=1e6, backupCount=10)
    logger.setLevel(logging.DEBUG)
    logger.addHandler(handler)
    total = 0
    count = 0

    ids = []
    codes = []
    for i in range(chunk[0], chunk[1]):
        ass = ass_list[i]
        id = id_list[i]
        
        try:
            eles = split_assignee(ass)
            total += 1
            res = []
            # print(eles)
            for ele in eles:
                tmp = choose(ele, des_dict.keys())
                if tmp != '':
                    cnt = str(datetime.datetime.now()) + ": " + id + " | " + ele + " <-> " + tmp + ": " + des_dict[tmp][0] + " (" + str(count) + " / " + str(total) + ")"
                    logger.info(cnt)
                    res.append(des_dict[tmp][0])
                    # print(key)
            if res:
                count += 1
                ids.append(id)
                codes.append(", ".join(res))
                # print(ids)
                # print(codes)
            
        except Exception as e:
            cnt = str(datetime.datetime.now()) + ": "  + "error occured:" + str(e) 
            logger.warning(cnt)
            logger.error(cnt)
    cnt = str(datetime.datetime.now()) + ": save chunks "  + str(chunk[0]) + " - " + str(chunk[1])
    logger.info(cnt)
    save(ids, codes, chunk)
    cnt = str(datetime.datetime.now()) + ": saved. "
    logger.info(cnt)
            

if __name__ == '__main__':
    try:
        os.mkdir('log/') 
    except Exception as e:
        print('errer',e) 

    print('set multirocessing...')

    pool = multiprocessing.Pool(processes=n_processes)
    manager = multiprocessing.Manager()

    client = Client(host=config.host22, port=config.port22, database=config.database22, user=config.user22, password=base64.b64decode(config.password22).decode('utf-8'))
    query_ass = "select patent_id, assignee from his_data_snaps.google_patent_data_common" #  where assignee like '%珠海格力电器%'  limit 2000
    df_ass = client.execute(query_ass)
    manager = multiprocessing.Manager()
    shared_id_list = manager.list([ass[0] for ass in df_ass])
    shared_ass_list = manager.list([ass[1] for ass in df_ass])
    num_data = len(shared_id_list)
    client.disconnect()

    conn = pymysql.connect(host=config.host70, user=config.user70, password=base64.b64decode(config.password70).decode('utf-8'), database=config.database70)
   
    query_des = "select S_INFO_WINDCODE, S_INFO_COMPNAME, S_INFO_COMPNAMEENG, S_INFO_LISTDATE from wind_db.asharedescription where (S_INFO_WINDCODE not like 'A%') and (S_INFO_LISTDATE is not null) and (S_INFO_NAME not like '%(IPO终止)') and (S_INFO_NAME not like '%(退市)')"
    df_des = pd.read_sql(query_des, con=conn)
    desc_dict = {}
    for i in range(len(df_des)):
        compname = df_des.iloc[i, 1]
        compname_eng = df_des.iloc[i, 2]
        windcode = df_des.iloc[i, 0]
        listdate = df_des.iloc[i, 3]
        if compname is not None:
            if compname not in desc_dict.keys() or desc_dict[compname_eng.lower().strip()][1] < listdate:
                desc_dict[compname] = (windcode, listdate)
        if compname_eng is not None:
            if compname_eng not in desc_dict.keys() or desc_dict[compname_eng.lower().strip()][1] < listdate:
                desc_dict[compname_eng.lower().strip()] = (windcode, listdate)
    conn.close()
    shared_des_dict = manager.dict(desc_dict)


    chunks = generate_chunks(num_data, n_processes)

    for i in range(n_processes):
        pool.apply_async(worker, (i, chunks[i], shared_id_list, shared_ass_list, shared_des_dict))

    pool.close()
    pool.join()