import os
import sys
my_module_path = os.path.abspath('/path/to/my_config.py')
sys.path.append(os.path.dirname(my_module_path))
import my_config
import base64

import pandas as pd
import pymysql
from clickhouse_driver import Client
import datetime
import requests
import json
from airflow import DAG
import pendulum
from airflow.operators.python import PythonOperator
from airflow.exceptions import AirflowException


tables_stock = ['xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx']
tables_future = ['xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx']
tables_bdf_backfill = ['xxx', 'xxx']


# 向企业微信机器人发送告警信息
def send_alert(db_name_list, old_list, new_list, retry):
    # 构造消息体
    cnt = "[{}][maintainer:swl]: The database is not updated today! (Retry: {}) Details are as follows\n\n".format(datetime.datetime.now(), retry)
    for i in range(len(db_name_list)):
        cnt += "Table: {} \nUpdated trade_date: {} \nCurrent trade_date: {} \n---\n".format(db_name_list[i], old_list[i], new_list[i])
    
    msg = {
        "msgtype": "markdown",
        "markdown": {
            "content": cnt
        }
    }

    # 发送请求
    webhook = f"https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key={my_config.webhook}"

    response = requests.post(
        url=webhook, 
        headers={
            "Content-Type": "application/json",
        },
        data=json.dumps(msg)
    )

    if response.status_code == 200:
        print("消息发送成功")
    else:
        print("消息发送失败")

# 检查今天是否为期货交易日
def check_future_date(day):
    conn = pymysql.connect(host=my_config.host70, user=my_config.user70, password=base64.b64decode(my_config.password70).decode('utf-8'))
    query = "SELECT TRADE_DAYS FROM xxx.cfuturescalendar WHERE TRADE_DAYS = {} limit 1;".format(day.strftime("%Y%m%d"))
    df = pd.read_sql(query, con=conn)
    conn.close() 
    return not df.empty

def check_stock_date(day):
    conn = pymysql.connect(host=my_config.host70, user=my_config.user70, password=base64.b64decode(my_config.password70).decode('utf-8'))
    query = "SELECT TRADE_DAYS FROM xxx.asharecalendar WHERE TRADE_DAYS = {} limit 1;".format(day.strftime("%Y%m%d"))
    df = pd.read_sql(query, con=conn)
    conn.close() 
    return not df.empty

def last_future_trade_day(today):
    oneday = datetime.timedelta(days=1)
    day = today - oneday
    while not check_future_date(day):
        day = day - oneday
    return day

# 查询数据库更新维护情况(20:30批次)
def check_db_updated_2030():
    db_name_list = [] 
    old_list = []
    new_list = []

    if check_future_date(datetime.date.today()):
        # 检查除了limao数据表的期货期权数据表
        client = Client(host=my_config.host72, port=my_config.port72, user=my_config.user72, password=base64.b64decode(my_config.password72).decode('utf-8'))
        target = datetime.date.today()
        for table in tables_future:
            query = "SELECT max(trade_date) from his_bopu.{};".format(table)
            df = client.execute(query)

            # 如果数据库没有更新，记录数据库名，和最新更新日
            if (df[0][0] != target):
                db_name_list.append('his_bopu.{}'.format(table))
                old_list.append(df[0][0])
                new_list.append(target)
        
        # 单独检查limao数据表
        target = last_future_trade_day(datetime.date.today())
        query = "SELECT max(trade_date) from his_bopu.xxx_x;"
        df = client.execute(query)

        # 如果数据库没有更新，记录数据库名，和最新更新日
        if (df[0][0] != target):
            db_name_list.append('his_bopu.xxx_x')
            old_list.append(df[0][0])
            new_list.append(target)
        client.disconnect()

    if check_stock_date(datetime.date.today()):
        # 检查股票数据表
        client = Client(host=my_config.host72, port=my_config.port72, user=my_config.user72, password=base64.b64decode(my_config.password72).decode('utf-8'))
        target = datetime.date.today()
        for table in tables_stock:
            query = "SELECT max(trade_date) from his_bopu.{};".format(table)
            df = client.execute(query)
            # 如果数据库没有更新，记录数据库名，和最新更新日
            if (df[0][0] != target):
                db_name_list.append('his_bopu.{}'.format(table))
                old_list.append(df[0][0])   
                new_list.append(target) 
        client.disconnect() 

        client = Client(host=my_config.host52, port=my_config.port52, user=my_config.user52, password=base64.b64decode(my_config.password52).decode('utf-8'))  
        target = datetime.date.today()
        for table in tables_bdf_backfill:
            query = "SELECT max(trade_date) from bdf_backfill.{};".format(table)
            df = client.execute(query)
            # 如果数据库没有更新，记录数据库名，最新更新日，和期望最新日
            if (df[0][0] != target):
                db_name_list.append('bdf_backfill.{}'.format(table))
                old_list.append(df[0][0])   
                new_list.append(target) 
        client.disconnect()
        
    # 统一发送告警
    if db_name_list:
        retry_count = round((datetime.datetime.now() - datetime.datetime.now().replace(hour=20, minute=30, second=0, microsecond=0)).total_seconds() / 3600)
        send_alert(db_name_list, old_list, new_list, f"{retry_count}/2")
        raise AirflowException(f"his_bopu has not updated yet. Retry: {retry_count}/2.")

default_args = {
    'retries': 2,
    'retry_delay': datetime.timedelta(hours=1),
}

# 定义DAG
dag2030 = DAG("his_bopu_check_2030",
    start_date=pendulum.datetime(2023, 6, 5, tz="Asia/Shanghai"),
    schedule_interval="30 20 * * 1-5", 
    catchup = False, 
    tags= ["swl", "IT", "db check"],
    default_args=default_args
)

def run_2030():
    check_db_updated_2030()

task2030 = PythonOperator(
    task_id='job2030',
    python_callable=run_2030,  # 指定要执行的函数
    dag=dag2030
)

