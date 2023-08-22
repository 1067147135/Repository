import os
import sys
my_module_path = os.path.abspath('$AIRFLOW_HOME/dags/path/to/personal_folder/my_config.py')
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


def send_alert(etfs, retry, logger):
    cnt = f"[{datetime.datetime.now()}][his_global.index_eod_prices][{os.path.dirname(my_module_path)}][maintainer:swl]: Failed to update his_global.index_eod_prices! (Retry: {retry}) \nThe following content has not been updated yet: "
    cnt += ", ".join(etfs)

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
        print("success")
    else:
        print("fail")

def index_eod_prices_update():
    # initialize logging
    logging.basicConfig()
    # initialize logger
    logger = logging.getLogger('logger')
    # write in file，maximum 1MB，back up 5 files。
    handler = logging.handlers.RotatingFileHandler(
        f'{os.path.dirname(my_module_path)}/log/index_eod_prices_update_0500.log', maxBytes=1e6, backupCount=5)
    logger.setLevel(logging.DEBUG)
    logger.addHandler(handler)

    p1 = int(datetime.datetime.now().replace(hour=8, minute=8, second=0, microsecond=0).timestamp()) - one_day
    p2 = int(datetime.datetime.now().replace(hour=8, minute=8, second=0, microsecond=0).timestamp())
    target = int((datetime.datetime.now() - datetime.timedelta(days=1)).replace(hour=4, minute=0, second=0, microsecond=0).timestamp())
    client = Client(host=my_config.host72, port=my_config.port72, user=my_config.user72, password=base64.b64decode(my_config.password72).decode('utf-8'))
    not_updated = []

    for etf_tag in etf: 
        
        url = f"https://query1.finance.yahoo.com/v8/finance/chart/{etf_tag}?interval=1d&period1={p1}&period2={p2}"
        logger.info(f"[{datetime.datetime.now()}] Try to get {url}")
        response = requests.get(url, headers=headers)
        if response.status_code != 200:
            not_updated.append(etf_tag)
            continue

        logger.info(f"[{datetime.datetime.now()}] Try to parse data...")
        parsed_data = json.loads(response.text)['chart']['result'][0]
        dates_timestamp = parsed_data['timestamp'][0]
        # print(dates_timestamp, "\t| ", target)
        if dates_timestamp < target:
            not_updated.append(etf_tag)
            continue
        
        symbol = etf_dict[etf_tag]
        trade_date = datetime.date.fromtimestamp(dates_timestamp - half_day)
        open = round(parsed_data['indicators']['quote'][0]['open'][0], 2) 
        high = round(parsed_data['indicators']['quote'][0]['high'][0], 2) 
        low = round(parsed_data['indicators']['quote'][0]['low'][0], 2) 
        close = round(parsed_data['indicators']['quote'][0]['close'][0], 2) 
        # adjclose = round(parsed_data['indicators']['adjclose'][0]['adjclose'][0], 2) 
        # volumn = round(parsed_data['indicators']['quote'][0]['volume'][0], 2) 
        logger.info(f"[{datetime.datetime.now()}] Data ready: '{symbol}', '{trade_date}', '{open}', '{high}', '{low}', '{close}'")

        check_query = f"SELECT * FROM his_global.index_eod_prices WHERE trade_date = '{trade_date}' and symbol = '{symbol}'"
        df = pd.DataFrame(client.execute(check_query))
        if (df.empty):
            update_query = 'INSERT INTO his_global.index_eod_prices (symbol, trade_date, open, high, low, close) VALUES'
            update_query += f"('{symbol}', '{trade_date}', '{open}', '{high}', '{low}', '{close}')"
            # print(update_query)
            logger.info(f"[{datetime.datetime.now()}] Insert data: {update_query}")
            try: 
                client.execute(update_query)
                logger.info(f"[{datetime.datetime.now()}] Success!")
            except Exception as e:
                not_updated.append(etf_tag)
                print(e)
                continue
        
    client.disconnect()
    if not_updated: 
        retry_count = round((datetime.datetime.now() - datetime.datetime.now().replace(hour=5, minute=0, second=0, microsecond=0)).total_seconds() / 3600)
        send_alert(not_updated, f"{retry_count}/3", logger)
        raise AirflowException(f"Fail to update his_global.index_eod_prices. Retry: {retry_count}/3.")

# index_eod_prices_update()
def run_0500():
    index_eod_prices_update()

default_args_0500 = {
    'retries': 3,
    'retry_delay': datetime.timedelta(hours=1),
}

dag0500 = DAG(
    dag_id="index_eod_prices_update_0500",
    start_date=pendulum.datetime(2023, 6, 5, tz="Asia/Shanghai"),
    schedule_interval='0 5 * * 2-6',
    catchup = False,
    tags= ["swl", "IT", "db update"],
    default_args=default_args_0500
)

task0500 = PythonOperator(
    task_id='job0500',
    python_callable=run_0500,  
    dag=dag0500
)