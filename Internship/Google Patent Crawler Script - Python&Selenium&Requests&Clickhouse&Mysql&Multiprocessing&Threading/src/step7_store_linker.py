import config
from utils.db_helper import fix

import os
import csv
import codecs
import base64
from clickhouse_driver import Client

# Arguments
folders_linker = config.folders_linker

def trans_single(string):
    trans_dict = {'SZ': 'SZSE', 'SH': 'SSE', 'BJ': 'BSE'}
    part = string.split(".")
    res = part[0] + '|ST|' + trans_dict[part[1]]
    return res
    
def trans_group(string):
    res = string.split(", ")
    for i in range(len(res)):
        res[i] = trans_single(res[i])
    return ", ".join(res)

if __name__ == '__main__':
    client = Client(host=config.host22, port=config.port22, database=config.database22, user=config.user22, password=base64.b64decode(config.password22).decode('utf-8'))

    # insert linker
    for folder in folders_linker:
        file_names = os.listdir(folder)   # read all file names
        for file in file_names:
            file_path = os.path.join(folder, file)   # construct the full path
            with codecs.open(file_path, "r", "utf-8") as f:
                reader = csv.reader(f)
                data = list(reader)[1:]
                query = 'INSERT INTO his_data_snaps.google_patent_symbol (* EXCEPT(bopu_update_time)) VALUES'
                query += ','.join([f"('{fix(row[0])}', '{fix(trans_group(row[1]))}')" for row in data])
                client.execute(query)
                print("finish", file_path)