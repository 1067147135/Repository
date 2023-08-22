import os
import sys
my_module_path = os.path.abspath('$AIRFLOW_HOME/dags/path/to/personal_folder/my_config.py')
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


tables_daily_db = ["xxx", "xxx"]
tables_tdb_min = ['xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx', 'xxx']

#获取今天的日期
today = datetime.date.today()

target = today

# 向企业微信机器人发送告警信息
def send_alert(db_name_list, old_list, new, retry):
    # 构造消息体
    cnt = "[{}][maintainer:swl]: The database is not updated today! (Retry: {}) Details are as follows\n\n".format(datetime.datetime.now(), retry)
    for i in range(len(db_name_list)):
        cnt += "Table: {} \nUpdated trade_date: {} \nCurrent trade_date: {} \n---\n".format(db_name_list[i], old_list[i], new)
    
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

# 查询数据库更新维护情况(22:00批次)
def check_db_updated_2200():
    conn = pymysql.connect(host=my_config.host70, user=my_config.user70, password=base64.b64decode(my_config.password70).decode('utf-8'))
    db_name_list = [] 
    old_list = []

    # 检查daily_db下的一系列数据表
    for table in tables_daily_db:
        query = "SELECT max(trade_date) from daily_db.{};".format(table)
        df = pd.read_sql(query, con=conn)
        # 如果数据库没有更新，记录数据库名，和最新更新日
        if (df.iloc[0, 0] != target):
            db_name_list.append('daily_db.{}'.format(table))
            old_list.append(df.iloc[0, 0])
    conn.close() 

    # 统一发送告警
    if db_name_list:
        retry_count = round((datetime.datetime.now() - datetime.datetime.now().replace(hour=22, minute=00, second=0, microsecond=0)).total_seconds() / 3600)
        send_alert(db_name_list, old_list, target, f"{retry_count}/1")
        raise AirflowException(f"daily_db has not updated yet. Retry: {retry_count}/1.")

# 查询数据库更新维护情况(17:30批次)
def check_db_updated_1730():
    conn = pymysql.connect(host=my_config.host70, user=my_config.user70, password=base64.b64decode(my_config.password70).decode('utf-8'))
    db_name_list = [] 
    old_list = []

    # 检查tdb_min下的一系列数据表
    for table in tables_tdb_min:
        query = "SELECT max(trade_date) from tdb_min.{};".format(table)
        df = pd.read_sql(query, con=conn)
        # 如果数据库没有更新，记录数据库名，和最新更新日
        if (df.iloc[0, 0] != target):
            db_name_list.append('tdb_min.{}'.format(table))
            old_list.append(df.iloc[0, 0])
    conn.close() 

    # 统一发送告警
    if db_name_list:
        retry_count = round((datetime.datetime.now() - datetime.datetime.now().replace(hour=17, minute=30, second=0, microsecond=0)).total_seconds() / 3600)
        send_alert(db_name_list, old_list, target, f"{retry_count}/3")
        raise AirflowException(f"tdb_min has not updated yet. Retry: {retry_count}/3.")

# 检查今天是否为期货交易日
def check_date(day):
    conn = pymysql.connect(host=my_config.host70, user=my_config.user70, password=base64.b64decode(my_config.password70).decode('utf-8'))
    query = "SELECT TRADE_DAYS FROM xxx.cfuturescalendar WHERE TRADE_DAYS = {} limit 1;".format(day.strftime("%Y%m%d"))
    df = pd.read_sql(query, con=conn)
    conn.close() 
    return not df.empty

def run_1730():
    # 判断当天是不是交易日
    if check_date(today):
        check_db_updated_1730()

def run_2200():
    if check_date(today):
        check_db_updated_2200()


# 定义DAG

default_args_1730 = {
    'retries': 3,
    'retry_delay': datetime.timedelta(hours=1),
}

default_args_2200 = {
    'retries': 1,
    'retry_delay': datetime.timedelta(hours=1),
}
dag1730 = DAG(
    dag_id="tdb_min_check_1730",
    start_date=pendulum.datetime(2023, 6, 5, tz="Asia/Shanghai"),
    schedule_interval='30 17 * * 1-5',
    catchup = False,
    tags= ["swl", "IT", "db check"],
    default_args=default_args_1730
)

dag2200 = DAG(
    dag_id="daily_db_check_2200",
    start_date=pendulum.datetime(2023, 6, 5, tz="Asia/Shanghai"),
    schedule_interval="0 22 * * 1-5",
    catchup = False,
    tags= ["swl", "IT", "db check"],
    default_args=default_args_2200
)

task1730 = PythonOperator(
    task_id='job1730',
    python_callable=run_1730,  # 指定要执行的函数
    dag=dag1730
)

task2200 = PythonOperator(
    task_id='job2200',
    python_callable=run_2200,  # 指定要执行的函数
    dag=dag2200
)
