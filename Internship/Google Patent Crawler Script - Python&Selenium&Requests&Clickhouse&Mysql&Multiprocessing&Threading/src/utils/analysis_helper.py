from bs4 import BeautifulSoup
import os
import re
import config
import time
import random
import requests
import datetime
import pandas as pd

class data:
    def __init__(self, n, k):
        self.abstr_list = [None]*n
        self.claims_list = [None]*n
        self.clas_list = [None]*n

        self.id_list = [[] for _ in range(k)]
        self.pub_number_list = [[] for _ in range(k)]
        self.pri_date_list = [[] for _ in range(k)]
        self.pub_date_list = [[] for _ in range(k)]
        self.assignee_list = [[] for _ in range(k)]
        self.title_list = [[] for _ in range(k)]

def get_title(soup):
    # 查找第一个符合条件的<meta>元素
    meta_element = soup.find('meta', {'name': 'DC.title'})
    if meta_element:
        # 获取content属性的值
        return meta_element.get('content').strip()
    else:
        return ""

def get_assignee(soup):
    meta_elements = soup.find_all('meta', {'scheme': 'assignee'})
    if meta_elements:
        # 获取content属性的值
        ass_list = []
        for ele in meta_elements:
            ass_list.append(ele.get('content').strip())
        return ", ".join(ass_list)
    else:
        return ""

def get_inventor(soup):
    meta_elements = soup.find_all('meta', {'scheme': 'inventor'})
    if meta_elements:
        # 获取content属性的值
        inv_list = []
        for ele in meta_elements:
            inv_list.append(ele.get('content').strip())
        return ", ".join(inv_list)
    else:
        return ""

def get_pri_date(soup):
    priority_date_element = soup.find(string = re.compile(r"(\s\w+)?riority to(\s\w+)?"))
    if priority_date_element:
        priority_date = priority_date_element.find_previous('time').text.strip()
        return priority_date
    else:
        return ""
    
def get_fil_date(soup):
    filing_date_element = soup.find(string = re.compile(r"(\s\w+)?filed by(\s\w+)?"))
    if filing_date_element:
        filing_date = filing_date_element.find_previous('time').text.strip()
        return filing_date
    else:
        return ""

def get_pub_date(soup, id):
    publication_date_element = soup.find(string = re.compile(r"(\s\w+)?ublication of {}(\s\w+)?".format(id.replace('-', ''))))
    if publication_date_element:
        publication_date = publication_date_element.find_previous('time').text.strip()
        return publication_date
    else:
        return ""

def get_gra_date(soup):
    grant_date_element = soup.find(string = re.compile(r"(\s\w+)?granted(\s\w+)?"))
    if grant_date_element:
        grant_date = grant_date_element.find_previous('time').text.strip()
        return grant_date
    else:
        return ""

def get_abstract(soup):
    abstract = soup.find('div', {'class': 'abstract'})
    if abstract:
        return abstract.text.strip()
    else:
        return ""

def get_claims_num(soup):
    claims = soup.find('section', {'itemprop': 'claims'})
    if claims:
        num = claims.find('span', {'itemprop': "count"})
        return num.text.strip()
    else:
        return ""

def get_classifications(soup):
    uls = soup.find_all('ul', {'itemprop': 'cpcs'})
    if uls:
        classification_list = []
        for ul in uls:
            lis = ul.find_all('li')
            last_li = lis[-1].text
            classification_list.append(last_li.split('—')[0].strip())
        return ", ".join(classification_list)
    else:
        return ""

def filter(pub_number):
    """
    translate patent id
    e.g. CN213780373U -> CN-213780373-U
    """
    if '-' in pub_number:
        return pub_number
    cty =  pub_number[:2]
    idx = 2
    while pub_number[idx].isdigit():
        idx += 1
    num = pub_number[2:idx]
    ed = pub_number[idx:]
    return cty + '-' + num + '-' + ed

def get_date_ctr(input_file):
    """
    return: e.g. 20220101-CN
    """
    f = open(input_file,'r', encoding="utf-8")
    url = f.readline().split(',')[1]
    cuts = url.split('=priority:')
    ctr = url.split('country=')[1][:2]
    f.close()
    return cuts[1][:8] + '-' + ctr

def get_date(input_file):
    """
    return: e.g. 2022-01-01
    """
    f = open(input_file,'r', encoding="utf-8")
    url = f.readline().split(',')[1]
    cuts = url.split('=priority:')
    return cuts[2][:4] + '-' + cuts[2][4:6] + '-' + cuts[2][6:8]

def save_common(abstr_list, claims_list, clas_list, input_file, output_folder_path):
    file_name = get_date_ctr(input_file)
    output_file = generate_path_name(output_folder_path, file_name, '-common')
    f = pd.read_csv(input_file, skiprows = 1, encoding="utf-8")
    f['abstract'] = abstr_list
    f['claims_num'] = claims_list
    f['classifications'] = clas_list
    f.to_csv(output_file, index=False)

def save_cite(id_list, pub_number_list, pri_date_list, pub_date_list, assignee_list, title_list, input_file, output_folder_path):
    file_name = get_date_ctr(input_file)
    output_file = generate_path_name(output_folder_path, file_name, '-cite')
    df = pd.DataFrame({
        'Id': id_list,
        'Publication number': pub_number_list,
        'Priority date': pri_date_list,
        'Publication date': pub_date_list,
        'Assignee': assignee_list,
        'Title': title_list
    })
    df.to_csv(output_file, index=False, encoding='utf-8')

def generate_path_name(path, name, middle = ''):
    return os.path.join(path, name + middle + '.csv')

def get_soup(url, logger):
    code = 0
    count = 1
    while count < 100:  # 循环去请求网站
        time.sleep(1)
        try:
            header = random.choice(config.headers_list)
            response = requests.get(url, headers=header, timeout=10)  
            code = response.status_code
            response.encoding = 'utf-8'
            soup = BeautifulSoup(response.text, 'html.parser')
            if (code == 200) and (get_abstract(soup) != "" or get_claims_num(soup) != "" or get_classifications(soup) != ""):
                break
        except:
            # print("Error:", code)
            cnt = str(datetime.datetime.now()) + ": Error in get {}, retry... ({})".format(url, count)
            logger.warning(cnt)
        count += 1
    if count >= 100:
        cnt = str(datetime.datetime.now()) + "Error in get {}, give up.".format(url)
        logger.warning(cnt)
        return -1
    return soup