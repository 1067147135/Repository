import config
from utils.db_helper import fix

import os
import csv
import codecs
import base64
from clickhouse_driver import Client

# Arguments
folders_common = config.folders_cite
folders_cite = config.folders_common

if __name__ == '__main__':
    client = Client(host=config.host22, port=config.port22, database=config.database22, user=config.user22, password=base64.b64decode(config.password22).decode('utf-8'))

    # insert common
    for folder in folders_common:
        file_names = os.listdir(folder)   # read all file names
        for file in file_names:
            file_path = os.path.join(folder, file)   # construct the full path
            with codecs.open(file_path, "r", "utf-8") as f:
                reader = csv.reader(f)
                data = list(reader)[1:]
                query = 'INSERT INTO his_data_snaps.google_patent_data_common (* EXCEPT(bopu_update_time)) VALUES'
                query += ','.join([f"('{fix(row[0])}', '{fix(row[1])}', '{fix(row[2])}', '{fix(row[3])}', '{fix(row[4])}', '{fix(row[5])}', '{fix(row[6])}', '{fix(row[7])}', '{fix(row[8])}', '{fix(row[9])}', '{fix(row[10])}', '{fix(row[11])}', '{fix(row[12])}')" for row in data if row[1] != '' and row[2] != '' and row[3] != '' and row[5] != ''])
                client.execute(query)
                print("finish", file_path)

    # insert cite
    for folder in folders_cite:
        file_names = os.listdir(folder)   # read all file names
        for file in file_names:
            file_path = os.path.join(folder, file)   # construct the full path
            with codecs.open(file_path, "r", "utf-8") as f:
                reader = csv.reader(f)
                data = list(reader)[1:]
                if len(data) > 0:
                    query = 'INSERT INTO his_data_snaps.google_patent_data_cite (patent_id, cited_by_patent_id, priority_date, publication_date, assignee, title) VALUES'
                    query += ','.join([f"('{fix(row[0])}', '{fix(row[1])}', '{fix(row[2])}', '{fix(row[3])}', '{fix(row[4])}', '{fix(row[5])}')" for row in data])
                    client.execute(query)
                print("finish", file_path)
