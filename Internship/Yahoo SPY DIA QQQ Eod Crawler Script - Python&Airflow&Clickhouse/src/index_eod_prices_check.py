import os
import sys
my_module_path = os.path.abspath('C:\Users\珞绮locky\Desktop\博普实习\雅虎日线数据\my_config.py')
sys.path.append(os.path.dirname(my_module_path))
import my_config

import requests
import datetime
import json
import base64
import pandas as pd
from clickhouse_driver import Client
from airflow import DAG
import pendulum
from airflow.operators.python import PythonOperator
from airflow.exceptions import AirflowException

import logging
import logging.handlers

one_day = int(datetime.timedelta(days=1).total_seconds())
half_day = int(datetime.timedelta(hours=12).total_seconds())
headers = {'user-agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36 Edg/115.0.1901.203'}
etf = ['SPY', 'QQQ', 'DIA']
etf_dict = {'SPY': 'SPY US Equity', 'QQQ': 'QQQ US Equity', 'DIA': 'DIA US Equity'}


def send_alert(etfs, infos, logger):
    cnt = f"[{datetime.datetime.now()}][his_global.index_eod_prices][{os.path.dirname(my_module_path)}][maintainer:swl]: Errors in updating his_global.index_eod_prices! \nErrors occur in the following content: "
    for i in range(etfs):
        cnt = cnt + etfs[i] + ": " + infos[i] + "\n"

    logger.info(f"[{datetime.datetime.now()}] Send alert: {cnt}")

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
        logger.info("success")
    else:
        logger.info(f"fail with {response.status_code}")

def index_eod_prices_check():
    # initialize logging
    logging.basicConfig()
    # initialize logger
    logger = logging.getLogger('logger')
    # write in file, maximum 1M, back up 5 files
    handler = logging.handlers.RotatingFileHandler(
        f'{os.path.dirname(my_module_path)}/log/index_eod_prices_check_0630.log', maxBytes=1e6, backupCount=5)
    logger.setLevel(logging.DEBUG)
    logger.addHandler(handler)

    client = Client(host=my_config.host72, port=my_config.port72, user=my_config.user72, password=base64.b64decode(my_config.password72).decode('utf-8'))
    error_etfs = []
    error_infos = []

    for etf_tag in etf: 
        check_query = f"SELECT open, high, low, close FROM his_global.index_eod_prices WHERE trade_date = '{datetime.date.today() - datetime.timedelta(days=1)}' and symbol = '{etf_dict[etf_tag]}'"
        df = pd.DataFrame(client.execute(check_query))
        if (df.empty):
            error_etfs.append(etf_tag)
            error_infos.append("no data in database.")
        else:    
            if max(df.iloc[0, 0], df.iloc[0, 1], df.iloc[0, 2], df.iloc[0, 3]) != df.iloc[0, 1] or min(df.iloc[0, 0], df.iloc[0, 1], df.iloc[0, 2], df.iloc[0, 3]) != df.iloc[0, 2]:
                error_etfs.append(etf_tag)
                error_infos.append(f"inconsistent data: open = {df.iloc[0, 0]}, high = {df.iloc[0, 1]}, low = {df.iloc[0, 2]}, close = {df.iloc[0, 3]}.")
        
    client.disconnect()
    if error_etfs: 
        send_alert(error_etfs, error_infos, logger)
        raise AirflowException(f"Errors in updating his_global.index_eod_prices.")

# index_eod_prices_check()
def run_0630():
    index_eod_prices_check()

dag0630 = DAG(
    dag_id="index_eod_prices_check_0630",
    start_date=pendulum.datetime(2023, 6, 5, tz="Asia/Shanghai"),
    schedule_interval='30 6 * * 2-6',
    catchup = False,
    tags= ["swl", "IT", "db check"],
)

task0630 = PythonOperator(
    task_id='job0630',
    python_callable=run_0630,  
    dag=dag0630
)