import os
import csv
import codecs
from clickhouse_driver import Client

# Arguments
folders_linker = ['xxx']

def fix(string):
    special_characters = ["\\", "'", "\"", "\0", "\b", "\n", "\r", "\t", "\Z", "\%", "\_"]
    # 需要转义的特殊字符列表

    for character in special_characters:
        if character in string:
            string = string.replace(character, "\\" + character)  # 加上转义符进行替换

    return string

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
    client = Client(host='xx.xx.x.xx', port='xxx', database='xxx', user='xxx', password='xxx')
    # insert linker
    for folder in folders_linker:
        file_names = os.listdir(folder)   # read all file names
        for file in file_names:
            file_path = os.path.join(folder, file)   # construct the full path
            with codecs.open(file_path, "r", "utf-8") as f:
                reader = csv.reader(f)
                data = list(reader)[1:]
                query = 'INSERT INTO xxx.google_patent_symbol (* EXCEPT(bopu_update_time)) VALUES'
                query += ','.join([f"('{fix(row[0])}', '{fix(trans_group(row[1]))}')" for row in data])
                client.execute(query)
                print("finish", file_path)