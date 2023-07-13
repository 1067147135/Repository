import codecs
from clickhouse_driver import Client
import sys

if __name__ == '__main__':
    # 将输出重定向到文件
    sys.stdout = open('output.txt', 'w')

    print("get data")
    sys.stdout.flush()  # 刷新输出
    client = Client(host='xx.xx.x.xx', port='xxx', database='xxx', user='xxx', password='xxx')

    # 从数据库获取数据并存入set
    query_cite_id = 'select distinct patent_id from xxx.google_patent_data_cite'
    df_cite_id = client.execute(query_cite_id)
    cite_id_set = {id[0] for id in df_cite_id}

    query_cited_by_id = "select distinct cited_by_patent_id from xxx.google_patent_data_cite where cited_by_patent_id like 'CN%'"
    df_cited_by_id = client.execute(query_cited_by_id)
    cited_by_id_set = {id[0] for id in df_cited_by_id}

    query_common_id = 'select distinct patent_id from xxx.google_patent_data_common'
    df_common_id = client.execute(query_common_id)
    common_id_set = {id[0] for id in df_common_id}

    print("start check")
    sys.stdout.flush()  # 刷新输出
    count = 0
    # check 1: cite表的 patent_id 是否在 common 表都有
    for id in cite_id_set:
        count += 1
        if id not in common_id_set:
            print('check 1 error:', id)
        if count % 1000 == 0:
            print("check 1 checked: ", count)
            sys.stdout.flush()  # 刷新输出

    # check 2: cite表的cited_by_patent_id中以CN开头的部分是否在common表都有
    for id in cited_by_id_set:
        
        if id not in common_id_set:
            count += 1
            print('check 2 error:', id)
        # if count % 1000 == 0:
        #     print("check 2 checked: ", count)
        #     sys.stdout.flush()  # 刷新输出



    print("done:", count)
    sys.stdout.flush()  # 刷新输出

    # 关闭文件
    sys.stdout.close()