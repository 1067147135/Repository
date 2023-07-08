import os
import csv
import codecs
from clickhouse_driver import Client

folders_common = ['/home/swl/bopu/result_p/CN-2021-common', '/home/swl/bopu/result_p/CN-2022-common', '/home/swl/bopu/result_p/CN-2023-common']
folders_cite = ['/home/swl/bopu/result_p/CN-2021-cite', '/home/swl/bopu/result_p/CN-2022-cite', '/home/swl/bopu/result_p/CN-2023-cite']

client = Client(host='xx.xx.x.22', port='9000', database='xxx', user='xxx', password='xxx')

def fix(string):
    special_characters = ["\\", "'", "\"", "\0", "\b", "\n", "\r", "\t", "\Z", "\%", "\_"]
    # 需要转义的特殊字符列表

    for character in special_characters:
        if character in string:
            string = string.replace(character, "\\" + character)  # 加上转义符进行替换

    return string

# insert common
for folder in folders_common:
    file_names = os.listdir(folder)   # read all file names
    for file in file_names:
        file_path = os.path.join(folder, file)   # construct the full path
        with codecs.open(file_path, "r", "utf-8") as f:
            reader = csv.reader(f)
            data = list(reader)[1:]
            query = 'INSERT INTO xxx.google_patent_data_common (* EXCEPT(bopu_update_time)) VALUES'
            query += ','.join([f"('{fix(row[0])}', '{fix(row[1])}', '{fix(row[2])}', '{fix(row[3])}', '{fix(row[4])}', '{fix(row[5])}', '{fix(row[6])}', '{fix(row[7])}', '{fix(row[8])}', '{fix(row[9])}', '{fix(row[10])}', '{fix(row[11])}', '{fix(row[12])}')" for row in data])
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
                query = 'INSERT INTO xxx.google_patent_data_cite (patent_id, cited_by_patent_id, priority_date, publication_date, assignee, title) VALUES'
                query += ','.join([f"('{fix(row[0])}', '{fix(row[1])}', '{fix(row[2])}', '{fix(row[3])}', '{fix(row[4])}', '{fix(row[5])}')" for row in data])
                client.execute(query)
            print("finish", file_path)
