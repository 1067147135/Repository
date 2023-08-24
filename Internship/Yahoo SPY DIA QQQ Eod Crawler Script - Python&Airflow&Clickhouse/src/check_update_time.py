import requests
import datetime
import time
import json
import os
import schedule
import yfinance as yf
import pandas as pd

import logging
import logging.handlers

start_timestamp = int(datetime.datetime(2023, 8, 14, 8, 0, 0).timestamp())
ten_min = int(datetime.timedelta(minutes=10).total_seconds())
half_hour = int(datetime.timedelta(minutes=30).total_seconds())
half_day = int(datetime.timedelta(hours=12).total_seconds())
one_day = int(datetime.timedelta(days=1).total_seconds())
headers = {'user-agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36 Edg/115.0.1901.203'}
etf = ['SPY', 'QQQ', 'DIA']

# initialize logging
logging.basicConfig()
# initialize logger
logger = logging.getLogger('logger')
# write in file，maximum 1MB，back up 5 files。
handler = logging.handlers.RotatingFileHandler(
    f'{os.getcwd()}/log/monitor.log', maxBytes=1e6, backupCount=5)
logger.setLevel(logging.DEBUG)
logger.addHandler(handler)

# The program was run every 10 minutes from 0:00 a.m. to monitor data updates on Yahoo!
def job_monitor():

    p1 = int(datetime.datetime.now().replace(hour=8, minute=8, second=0, microsecond=0).timestamp()) - one_day
    p2 = int(datetime.datetime.now().replace(hour=8, minute=8, second=0, microsecond=0).timestamp())

    for etf_tag in etf: 
        # data from my crawler
        url = f"https://query1.finance.yahoo.com/v8/finance/chart/{etf_tag}?interval=1d&period1={p1}&period2={p2}"
        response = requests.get(url, headers=headers)

        try:
            parsed_data = json.loads(response.text)['chart']['result'][0]
            dates_timestamp = parsed_data['timestamp']
            date_time = datetime.datetime.fromtimestamp(dates_timestamp[0])
            date = datetime.date.fromtimestamp(dates_timestamp[0] - half_day)

            opens = parsed_data['indicators']['quote'][0]['open']
            highs = parsed_data['indicators']['quote'][0]['high']
            lows = parsed_data['indicators']['quote'][0]['low']
            closes = parsed_data['indicators']['quote'][0]['close']
            adjcloses = parsed_data['indicators']['adjclose'][0]['adjclose']
            volumns = parsed_data['indicators']['quote'][0]['volume']

            logger.info(f"[{datetime.datetime.now()}] source: my crawler \t| etf_tag: {etf_tag} \t| beijing_time: {date_time}")
            logger.info(f"date: {date} \t\t\t\t| open: {opens[0]} \t| high: {highs[0]} \t| low: {lows[0]} \t| close: {closes[0]} \t| adjcloses: {adjcloses[0]} \t| volumn: {volumns[0]}")

        except Exception as e:
            logger.info(f"[{datetime.datetime.now()}] my crawler fail with {response.status_code}, error: {e}")


        # data from yfinance lib
        single_etf = yf.Ticker(etf_tag)
        df = single_etf.history(period='1d')
        try: 
            logger.info(f"[{datetime.datetime.now()}] source: yfinance \t\t| etf_tag: {etf_tag}")
            logger.info(f"time: {pd.to_datetime(df.index).strftime('%Y-%m-%d %H:%M:%S %Z').to_list()[0]} \t| open: {df.iloc[0, 0]} \t| high: {df.iloc[0, 1]} \t| low: {df.iloc[0, 2]} \t| close: {df.iloc[0, 3]} \t| volumn: {df.iloc[0, 4]} \t| dividends: {df.iloc[0, 5]} \t| stock splits: {df.iloc[0, 6]}")
        except Exception as e:
            logger.info(f"[{datetime.datetime.now()}] yfinance fail with error: {e}")


        
if __name__ == '__main__':
    

    # set job, run every 10 minutes
    schedule.every(10).minutes.do(job_monitor)

    # compute the seconds to next midnight (beijing timezone)
    now = time.time()
    next_midnight = (int(now) // 86400 + 1) * 86400 - 28800
    seconds_to_midnight = next_midnight - now

    # wait until next midnight
    logger.info(f"The program will run after {seconds_to_midnight} seconds")
    time.sleep(seconds_to_midnight)

    # start doing job
    while True:
        schedule.run_pending()
        time.sleep(1)
