import requests
import datetime
import time
import sys

# generate time interval by day
def generate_time_by_day(begin, end):
    start_date = datetime.datetime.strptime(begin, '%Y%m%d')
    end_date = datetime.datetime.strptime(end, '%Y%m%d')
    result = []

    while start_date <= end_date:
        result.append(start_date.strftime('%Y%m%d'))
        start_date += datetime.timedelta(days=1)

    return result


if __name__ == '__main__':
    # Arguments
    if len(sys.argv) > 2:
        begin = sys.argv[1]
        end = sys.argv[2]
    else:
        begin = "20210101"
        end = "20210106"
    download_path = 'xxx'
    countrys = ["CN"] # , "US"
    headers = {'cookie': 'xxx', 
            'user-agent': 'xxx'
    }
    
    time_list = generate_time_by_day(begin, end)
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

