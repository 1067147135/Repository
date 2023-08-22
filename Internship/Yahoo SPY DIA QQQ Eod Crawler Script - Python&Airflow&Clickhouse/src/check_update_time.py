import requests
import datetime
import time
import json

start_timestamp = int(datetime.datetime(2023, 8, 14, 8, 0, 0).timestamp())
ten_min = int(datetime.timedelta(minutes=10).total_seconds())
half_hour = int(datetime.timedelta(minutes=30).total_seconds())
half_day = int(datetime.timedelta(hours=12).total_seconds())
one_day = int(datetime.timedelta(days=1).total_seconds())
headers = {'user-agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36 Edg/115.0.1901.203'}
etf = ['SPY', 'QQQ', 'DIA']

if __name__ == '__main__':
    print("program sleep...")
    time.sleep(int(datetime.timedelta(hours=7).total_seconds()))
    print("program start...")
    # The program was run in the morning of 2023-08-16 to observe the change of the interface returning EOD data
    # With the following timestamp, the interface should return the EOD data of August 15, 
    # but the EOD data of August 15 has not come out before 4:00 on August 16, 
    # and the interface will temporarily return the EOD data of August 14 or the real-time market on August 15.
    p1 = int(datetime.datetime(2023, 8, 15, 8, 0, 0).timestamp())
    p2 = int(datetime.datetime(2023, 8, 16, 8, 0, 0).timestamp())
    for i in range(200):
        
        for etf_tag in etf: 
            url = f"https://query1.finance.yahoo.com/v8/finance/chart/{etf_tag}?interval=1d&period1={p1}&period2={p2}"
            response = requests.get(url, headers=headers)

            if (response.status_code == 200):
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

                print(datetime.datetime.now(), "\t| ", etf_tag, "\t| ", date, "\t| ", date_time)
                print(opens[0], "\t| ", highs[0],  "\t| ", lows[0],  "\t| ", closes[0],  "\t| ", adjcloses[0], "\t| ",  volumns[0])
            else:
                print(f"[{datetime.datetime.now()}] fail with {response.status_code}")


        time.sleep(ten_min)