import config
from utils import generate_time

import requests
import datetime
import time
import sys

if __name__ == '__main__':
    # Arguments
    try: 
        begin = sys.argv[1]
        end = sys.argv[2]

        download_path = config.download_path
        countrys = config.countrys
        headers = {'cookie': config.cookie, 
                'user-agent': config.user_agent
        }
        
        print('Start working!')
        time_list = generate_time.generate_time_by_day(begin, end)
        for ctr in countrys:
            for t in time_list:
                url = "https://patents.google.com/xhr/query?url=country%3D{}%26before%3Dpriority%3A{}%26after%3Dpriority%3A{}%26type%3DPATENT%26sort%3Dnew&exp=&download=true".format(ctr, t, t)
                response = requests.get(url, headers=headers)
                print(datetime.datetime.now())
                print(url)
                print(response.status_code)
                if response.status_code == 429:
                    break
                f = open(f'{download_path}{t}-{ctr}.csv','w', encoding="utf-8")
                f.write(response.content.decode("utf-8"))
                f.close()
                time.sleep(15)
        print('Finished!')        
    except:
        print('Invalid arguments!')
        print('The correct command should be: python step1_download_requests.py <begin> <end>')
        print('e.g. python step1_download_requests.py 20210101 20211231')
        print('The program terminated.')
    

