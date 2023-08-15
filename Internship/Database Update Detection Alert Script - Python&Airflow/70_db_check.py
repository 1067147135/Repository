import os
import sys
my_module_path = os.path.abspath('/path/to/my_config.py')
sys.path.append(os.path.dirname(my_module_path))
import my_config
import base64

import pandas as pd
import pymysql
import datetime
import requests
import json
from airflow import DAG
import pendulum
from airflow.operators.python import PythonOperator
from airflow.exceptions import AirflowException

# pair = (db.table name, time coloumn, stock/future)
pair_0840 = [("xxx.xxx", "update_date", "s"), 
             ("xxx.xxx", "update_date", "s"), 
             ("xxx", "trade_date", "f")]

pair_1730 = [("xxx.xxx", "modify_time", "f"), 
             ("xxx.xxx", "op_date", "f"), 
             ("xxx.xxx", "trade_date", "s")]

pair_2030 = [("xxx.xxx", "bopu_update_time", "s"), 
             ("xxx.xxx", "trade_date", "f")]
            # ("xxx.xxx", "trade_date", "s") checkes next trading date, others check today

today = datetime.date.today()

def send_alert(db_name_list, old_list, new_list, retry):
    
    cnt = "[{}][maintainer:swl]: The database is not updated today! (Retry: {}) Details are as follows\n\n".format(datetime.datetime.now(), retry)
    for i in range(len(db_name_list)):
        cnt += "Table: {} \nLatest date: {} \nTarget date: {} \n---\n".format(db_name_list[i], old_list[i], new_list[i])

    msg = {
        "msgtype": "markdown",
        "markdown": {
            "content": cnt
        }
    }

    webhook = f"https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key={my_config.webhook}"

    response = requests.post(
        url=webhook, 
        headers={
            "Content-Type": "application/json",
        },
        data=json.dumps(msg)
    )

    if response.status_code == 200:
        print("success")
    else:
        print("fail")

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

def next_stock_trade_day(today):
    oneday = datetime.timedelta(days=1)
    day = today + oneday
    while not check_stock_date(day):
        day = day + oneday
    return day

def check_70_db_updated_0840():
    conn = pymysql.connect(host=my_config.host70, user=my_config.user70, password=base64.b64decode(my_config.password70).decode('utf-8'))
    db_name_list = []
    old_list = []
    new_list = []
    flag_f = check_future_date(today)
    flag_s = check_stock_date(today)
    
    for table, col, flag in pair_0840:
        if flag == "f" and not flag_f: 
            continue
        if flag == "s" and not flag_s:
            continue
        query = f"SELECT max(date({col})) from {table};"
        df = pd.read_sql(query, con=conn)
        if (df.iloc[0, 0] != today):
            db_name_list.append(table)
            old_list.append(df.iloc[0, 0])
            new_list.append(today)
    
    conn.close() 

    if db_name_list:
        retry_count = round((datetime.datetime.now() - datetime.datetime.now().replace(hour=8, minute=40, second=0, microsecond=0)).total_seconds() / 1800)
        send_alert(db_name_list, old_list, new_list, f"{retry_count}/2")
        raise AirflowException(f"70_0840 has not updated yet. Retry: {retry_count}/2.")

def check_70_db_updated_1730():
    conn = pymysql.connect(host=my_config.host70, user=my_config.user70, password=base64.b64decode(my_config.password70).decode('utf-8'))
    db_name_list = []
    old_list = []
    new_list = []
    flag_f = check_future_date(today)
    flag_s = check_stock_date(today)

    for table, col, flag in pair_1730:
        if flag == "f" and not flag_f: 
            continue
        if flag == "s" and not flag_s:
            continue
        query = f"SELECT max(date({col})) from {table};"
        df = pd.read_sql(query, con=conn)
        if (df.iloc[0, 0] != today):
            db_name_list.append(table)
            old_list.append(df.iloc[0, 0])
            new_list.append(today)

    conn.close() 

    if db_name_list:
        retry_count = round((datetime.datetime.now() - datetime.datetime.now().replace(hour=17, minute=30, second=0, microsecond=0)).total_seconds() / 1800)
        send_alert(db_name_list, old_list, new_list, f"{retry_count}/2")
        raise AirflowException(f"70_1730 has not updated yet. Retry: {retry_count}/2.")

def check_70_db_updated_2030():
    conn = pymysql.connect(host=my_config.host70, user=my_config.user70, password=base64.b64decode(my_config.password70).decode('utf-8'))
    db_name_list = []
    old_list = []
    new_list = []
    flag_f = check_future_date(today)
    flag_s = check_stock_date(today)

    for table, col, flag in pair_2030:
        if flag == "f" and not flag_f: 
            continue
        if flag == "s" and not flag_s:
            continue
        query = f"SELECT max(date({col})) from {table};"
        df = pd.read_sql(query, con=conn)
        if (df.iloc[0, 0] != today):
            db_name_list.append(table)
            old_list.append(df.iloc[0, 0])
            new_list.append(today)

    if flag_s: # special logic for xxx.xxx: checkes next trading date (others check today)
        query = f"SELECT max(date(trade_date)) from xxx.xxx;"
        df = pd.read_sql(query, con=conn)
        if (df.iloc[0, 0] != next_stock_trade_day(today)):
            db_name_list.append('xxx.xxx')
            old_list.append(df.iloc[0, 0])
            new_list.append(next_stock_trade_day(today))

    conn.close() 

    if db_name_list:
        retry_count = round((datetime.datetime.now() - datetime.datetime.now().replace(hour=20, minute=30, second=0, microsecond=0)).total_seconds() / 1800)
        send_alert(db_name_list, old_list, new_list, f"{retry_count}/2")
        raise AirflowException(f"70_2030 has not updated yet. Retry: {retry_count}/2.")
    
def run_70_0840():
    check_70_db_updated_0840()

def run_70_1730():
    check_70_db_updated_1730()

def run_70_2030():
    check_70_db_updated_2030()

default_args = {
    'retries': 2,
    'retry_delay': datetime.timedelta(minutes=30),
}

dag_70_0840 = DAG(
    dag_id="db_70_check_0840",
    start_date=pendulum.datetime(2023, 6, 5, tz="Asia/Shanghai"),
    schedule_interval='40 08 * * 1-5',
    catchup = False,
    tags= ["swl", "IT", "db check"],
    default_args=default_args
)

dag_70_1730 = DAG(
    dag_id="db_70_check_1730",
    start_date=pendulum.datetime(2023, 6, 5, tz="Asia/Shanghai"),
    schedule_interval='30 17 * * 1-5',
    catchup = False,
    tags= ["swl", "IT", "db check"],
    default_args=default_args
)

dag_70_2030 = DAG(
    dag_id="db_70_check_2030",
    start_date=pendulum.datetime(2023, 6, 5, tz="Asia/Shanghai"),
    schedule_interval='30 20 * * 1-5',
    catchup = False,
    tags= ["swl", "IT", "db check"],
    default_args=default_args
)

task_70_0840 = PythonOperator(
    task_id='job_70_0840',
    python_callable=run_70_0840,  
    dag=dag_70_0840
)

task_70_1730 = PythonOperator(
    task_id='job_70_1730',
    python_callable=run_70_1730,
    dag=dag_70_1730
)

task_70_2030 = PythonOperator(
    task_id='job_70_2030',
    python_callable=run_70_2030,  
    dag=dag_70_2030
)