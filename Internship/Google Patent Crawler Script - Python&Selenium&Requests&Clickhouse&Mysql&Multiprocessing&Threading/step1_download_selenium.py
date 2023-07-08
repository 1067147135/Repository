from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
import time
import sys
import datetime

# set the time interval
# begin = "20220101"
# end = "20221231"
# set arguments for the browser driver
browser_path = 'E:\driver\chromedriver.exe'
download_path = 'E:\BOPU\download'
chrome_options = Options()
# chrome_options.add_argument('--headless') # use headless browser
# chrome_options.add_argument('--disable-gpu')
chrome_options.add_argument('--ignore-certificate-errors')
# chrome_options.add_argument('--proxy-server=http://27.150.87.152:8089')
prefs = {"download.default_directory": download_path}  # set download path
chrome_options.add_experimental_option("prefs", prefs)
browser = webdriver.Chrome(options=chrome_options) #executable_path=browser_path, 

# generate time interval by month
# e.g. begin = '20230107', end = '20230307'
def generate_time_by_month(begin, end):
    start_date = datetime.datetime.strptime(begin, '%Y%m%d')
    end_date = datetime.datetime.strptime(end, '%Y%m%d')
    result = []

    while start_date <= end_date:
        # get the last day of the current month
        year = start_date.year
        month = start_date.month
        if month == 12:
            end_of_month = datetime.datetime(year=year, month=month, day=31)
        else:
            end_of_month = datetime.datetime(year=year, month=month+1, day=1) - datetime.timedelta(days=1)

        # add the start date and end date of the current month
        result.append((start_date.strftime('%Y%m%d'), min(end_date, end_of_month).strftime('%Y%m%d')))

        # update the date to the next month
        start_date = end_of_month + datetime.timedelta(days=1)

    return result

# generate time interval by month
def generate_time_by_day(begin, end):
    start_date = datetime.datetime.strptime(begin, '%Y%m%d')
    end_date = datetime.datetime.strptime(end, '%Y%m%d')
    result = []

    while start_date <= end_date:
        result.append(start_date.strftime('%Y%m%d'))
        start_date += datetime.timedelta(days=1)

    return result

if __name__ == '__main__':
    time_list = generate_time_by_day(sys.argv[1], sys.argv[2])

    countrys = ["US"] # , "CN"

    for ctr in countrys:
        for t in time_list:
            url = "https://patents.google.com/?country={}&before=filing:{}&after=filing:{}&type=PATENT&sort=new".format(ctr, t, t)
            browser.get(url)
            time.sleep(5)
            try: 
                download_button = browser.find_element(By.XPATH, '//*[@id="count"]/div[1]/span[2]/a')
                download_button.click()
                time.sleep(10)
            except:
                print("No info between {} - {} in {}".format(t, t, ctr))

    browser.quit()
