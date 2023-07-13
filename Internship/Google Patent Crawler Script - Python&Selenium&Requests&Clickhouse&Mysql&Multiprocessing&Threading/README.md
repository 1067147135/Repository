# google_patent_crawler

## 项目的目的

从 `patents.google.com` 爬取专利数据以供量化研究员研究。

当前需求为爬取中国 2021 年至 2023 年的专利数据，程序经过少许参数修改后支持更大时间范围和国家范围的数据爬取。

## 项目的基本思路

1. `下载`：按日搜索，下载每天的专利数据 csv 文件。csv 文件包含如下信息：
    - `id`：专利号
    - `title`：专利名
    - `assignee`：受让人
    - `inventor/author`：发明人/作者
    - `priority date`：优先权日期
    - `filing/creation date`：申请/创造日期
    - `publication date`：公开日期
    - `grant date`：授权日期
    - `result link`：结果链接
    - `representative figure link`：代表性图链接
<br/>
<br/>
2. `分析`：分析 csv 文件内容，再访问各个专利详情页进一步提取补全信息。补全的信息如下：
    - `abstract`：概要
    - `claims_num`：专利的权利要求数量
    - `classifications`：专利的分类
    - `cite`：被引用信息
      - `Id`：被引用专利/原专利的专利号
      - `Publication number`：引用原专利的专利号
      - `Priority date`：引用专利的优先权日期
      - `Publication date`：引用专利的
      - `Assignee`：引用专利的受让人
      - `Title`：引用专利的标题
<br/>
<br/>
3. `入库`：将分析后的数据存入数据库。
    - `google_patent_data_common`：存储除了被引用信息以外的信息
    - `google_patent_data_cite`：存储被引用信息
<br/>
<br/>
4. `检查`：检查入库数据的引用关系是否完整。
    - `google_patent_data_cite` 表的 `patent_id` 是否在 `google_patent_data_common` 表都有。
    - `google_patent_data_cite` 表的 `cited_by_patent_id` 中以 CN 开头的部分是否在 `google_patent_data_common` 表都有。
<br/>
<br/>
5. `补全`：爬取补全检查时发现的缺漏数据。
<br/>
<br/>
6. `链接`：将 `google_patent_data_common` 里的 `assignee` 与 A 股公司代码匹配链接。
<br/>
<br/>
1. `入库`：将匹配成功的链接关系（`专利号 - A 股公司代码`）入库至 `google_patent_symbol` 数据表。


## 项目使用的技术栈

`python` 模块：
- `requests`（高效访问浏览器，绕过429封禁）
- `BeautifulSoup`、`json`（分析提取网页、请求返回结果信息）
- `datetime`（处理日期时间数据）
- `time`（控制访问浏览器的间隔，防止被封）
- `os`、`csv`、`codecs`、`sys`（读写 csv 文件）
- `random`（从请求头池中随机取请求头）
- `pandas`、`pymysql`（查询 MySQL 数据库）
- `clickhouse_driver` （查询 ClickHouse 数据库）
- `logging`（轮转日志，在记录运行信息的同时限制 log 的内存占用）
- `threading`、`multiprocessing`（多线程和多进程并行加速）

## 项目的数据库结果信息

项目爬取的所有数据结果已入库至 `xx.xx.x.xx` 的 ClickHouse 数据库。

三张结果数据表的建表语句如下：

```sql
CREATE TABLE xxx.google_patent_data_common
(
    patent_id String Codec (ZSTD),
    title String Codec (ZSTD),
    assignee String Codec (ZSTD),
    inventor_author String Codec (ZSTD),
    priority_date Nullable(Date) Codec (ZSTD),
    filing_creation_date Date Codec (ZSTD),
    publication_date Nullable(Date) Codec (ZSTD),
    grant_date Nullable(Date) Codec (ZSTD),
    result_link Nullable(String) Codec (ZSTD),
    representative_figure_link Nullable(String) Codec (ZSTD),
    abstract Nullable(String) Codec (ZSTD),
    claims_num Nullable(UInt32) Codec (ZSTD),
    classifications Nullable(String) Codec (ZSTD),
    bopu_update_time DateTime64(6, 'UTC') default toDateTime64(toString(now64(6)), 6, 'UTC') Codec (ZSTD)
)
ENGINE = MergeTree
PARTITION BY toYYYYMM(filing_creation_date)
ORDER BY (filing_creation_date, patent_id)
SETTINGS index_granularity = 8192;


CREATE TABLE xxx.google_patent_data_cite
(
    patent_id String Codec (ZSTD),
    cited_by_patent_id String Codec (ZSTD),
    priority_date Date Codec (ZSTD),
    publication_date Date Codec (ZSTD),
    assignee String Codec (ZSTD),
    title String Codec (ZSTD),
    bopu_update_time DateTime64(6, 'UTC') default toDateTime64(toString(now64(6)), 6, 'UTC') Codec (ZSTD)
)
ENGINE = MergeTree
PARTITION BY sipHash64(patent_id) % 16
ORDER BY (patent_id, cited_by_patent_id)
SETTINGS index_granularity = 8192;


CREATE TABLE xxx.google_patent_symbol
(
    patent_id String Codec (ZSTD),
    bopu_symbol LowCardinality(String) Codec (ZSTD),
    bopu_update_time DateTime64(6, 'UTC') default toDateTime64(toString(now64(6)), 6, 'UTC') Codec (ZSTD)
)
ENGINE = MergeTree
PARTITION BY sipHash64(patent_id) % 16
ORDER BY (patent_id)
SETTINGS index_granularity = 8192;
```

## 项目的使用方法

### step1_download_requests.py

该程序使用 requests 访问按要求生成的 patents.google.com 的 csv 下载地址获取 csv 数据文件，每 15 秒请求一次（请求太频繁会被 429 封禁）。

在 `if __name__ == '__main__':` 下的 `# Arguments` 的部分可以看到该程序可以指定的参数：
```python
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
```
 - `begin`、`end`：爬取日期范围，包含 begin 和 end 这两天。考虑到这对参数相比别的参数会被更频繁地更改，这对参数有两种设置方法。其他参数因为相对固定，均采用硬编码。
    - 运行时传入：
        ```bash
        python step1_download_requests.py <begin> <end>
        # e.g. python step1_download_requests.py 20210101 20210106
         ```
    - 硬编码：
        ```python
        begin = "<begin>"
        end = "<end>"
        # e.g. 
        # begin = "20210101"
        # end = "20210106"
        ```
 - `download_path`：csv 文件下载存储路径。（csv 文件名是自动生成的 `日期-国家.csv`。）
 - `countrys`：计划爬取的国家列表，可以添加其他国家的简称（e.g. US）以爬取更多的国家的专利数据。
 - `headers`：请求头信息。
   - `cookie`：Cookie。当程序因为 429 封禁终止时，需要更换一个新的有效的 Cookie 才可以继续爬取。新的 Cookie 可以手动通过浏览器访问下载地址时通过 F12 开发者页面抓请求头的包复制获得。如果手动访问也 429 的话，就找同事帮忙提供一下他们的 Cookie。一般情况下 1 个 Cookie 使用到 429 封禁之前可以爬取 1 个国家将近 1 年份的数据。
   - `user-agent`：浏览器客户端类型。该参数基本不需要改动。

---
### step1_download_selenium.py

step1_download_requests.py 的早期 selenium 实现版本，因无法绕过 429 封禁且无法在命令行服务器上调试被 step1_download_requests.py 替代而弃用停止维护。此文件仅作留档，以备个人未来参考。

---
### step2_analysis_web_only.py

该程序使用 requests 逐条访问 csv 文件中每个专利的详情页，用 BeautifulSoup 解析拉取到的 html 提取概要等额外信息，并将除了被引用信息以外的信息存入 `日期-国家-common.csv` 临时 csv 文件，将被引用信息存入 `日期-国家-cite.csv` 临时 csv 文件。

该程序使用并行技术，同时使用多进程和多线程。进程之间以通过 `multiprocessing.Manager().Queue()` 实现动态负载均衡，一个进程一次任务分析一整个原始 csv 文件。线程之间使用静态分配均分任务，一个线程一次任务分析一个原始 csv 文件的 `1/线程数` 。

该程序已通过 logging 实现轮转日志，日志存储于相对路径 `/log/` 文件夹下。每个线程对应一个日志文件，其后缀数字为 `10 * pid + tid` 。（后缀数字是指 `.log` 之前的数字， `.log` 之后的是轮转日志备份编号）

在文件开头的 `# Arguments` 的部分可以看到该程序可以指定的参数：
```python
input_folder_path = 'xxx'   # path to folder
output_folder_path = 'xxx' 
n_thread = 4
n_processes = 8
```
 - `input_folder_path`: 原始 csv 文件的存储路径。程序将从这个文件夹中逐个读取 csv 文件进行爬取分析。
 - `output_folder_path`：结果临时 csv 文件的存储路径。程序将把分析完后生成的临时 csv 文件 `日期-国家-common.csv` 和 `日期-国家-cite.csv` 存储到该路径下。
 - `n_thread`：并行参数，每个进程的线程数。
 - `n_processes`：并行参数，该程序的进程数。

---
### step2_analysis_db_web_fast.py

该程序是 `step2_analysis_web_only.py` 的需求特化加速版本，适用于重新爬取分析的场合，即：当完成一轮 step 1 - 3 之后，爬取需求从 `按申请日期` 变更为 `按优先权日期`。一般而言这需要重新跑一轮 step 1 - 3，但 step 2 是非常耗时的，分析三年的数据要数十个小时。考虑到两个需求中有大部分数据是重合的，且 step 2 最耗时的部分是访问网页，更改爬取详情页部分的逻辑为：优先比对数据库，若数据库已有该条数据，则从数据库拷贝，若数据库不存在该数据，再去访问详情页。

为了减少对数据库的访问次数，降低数据库负载：每分析一个原始 csv 文件，从数据库拉取一次当天的 common 数据存入字典；程序运行时，一次性拉取全部 cite 数据存入字典。后续比对使用字典。（common 总数据约 750w 条，cite 总数据约 69w 条）

**注意**：如果要使用该程序，一定要根据使用时的需求，个性化修改 `db2dict_common(date_str)` 和 `db2dict_cite()` 方法里的 sql 语句。

该程序的其他参数设置和 `step2_analysis_web_only.py` 完全一致，不再赘述。

---
### split.py

该程序是一个文件分类小工具，它可以把存在一个文件夹里的不同后缀的临时 csv 文件分类转移到两个文件夹里，以便于使用 `step3_warehouse.py`。

在文件开头的 `# Arguments` 的部分可以看到该程序可以指定的参数：
```python
src_dir = "xxx"
dst_dir_a = "xxx"
dst_dir_b = "xxx"
```
 - `src_dir`：源文件夹路径。
 - `dst_dir_a`：用于保存以 common 结尾的文件的目标文件夹路径。
 - `dst_dir_b`：用于保存以 cite 结尾的文件的目标文件夹路径。

---
### step3_warehouse.py

该程序将指定的多个文件夹里的所有临时 csv 文件 （common 和 cite，分别在不同文件夹里
）的数据跳过空行后录入 `xx.xx.x.xx` 数据库。

**注意**：数据表没有设置唯一字段的限制，程序也不是先查询再插入的逻辑，如果不作任何修改直接**多次**运行该程序，**会往数据库录入重复数据**。该程序适用于一次性录入大量数据。

如果录入重复数据的悲剧发生了，可以按照 `bopu_update_time` 批量删除数据。

在文件开头的 `# Arguments` 的部分可以看到该程序可以指定的参数：
```python
folders_common = ['xxx', 'xxx', 'xxx']
folders_cite = ['xxx', 'xxx', 'xxx']
```
 - `folders_common`：存有 common 数据的临时 csv 文件所在的文件夹列表。
 - `folders_cite`：存有 cite 数据的临时 csv 文件所在的文件夹列表。

---
### step4_check.py

该程序对入库的 common 和 cite 数据进行引用关系完整性检查。
 - `google_patent_data_cite` 表的 `patent_id` 是否在 `google_patent_data_common` 表都有。
 - `google_patent_data_cite` 表的 `cited_by_patent_id` 中以 CN 开头的部分是否在 `google_patent_data_common` 表都有。

**注意**：如果爬取国家发生变化，此处检查的条件也应当相应变更，注意修改主程序里的 `query_cited_by_id` 以满足需求。

该程序无需参数，输出结果在当前文件夹的 `output.txt` 。

---
### step5_replenish.py

该程序会检查不满足第二个完整性条件时缺少的专利号，并上网爬取补全。爬取的到的数据中，除了被引用信息以外的信息将被存入 `replenish-common.csv` 临时 csv 文件，被引用信息将被存入 `replenish-cite.csv` 临时 csv 文件。

将 `replenish-common.csv` 和 `replenish-cite.csv` 补充入库需要将它们分开放到特定文件夹并使用 `step3_warehouse.py` 入库。

该程序使用了轮转日志和多线程加速，每个线程对应一个日志文件，日志文件在相对路径 `/log/` 文件夹下，后缀数字为 `tid`。

因为数据是通过搜索引擎收集的，而搜索引擎因为性能限制和受众特点只保证返回 “大量” 符合条件的相关数据以供来浏览，不保证返回 “全部” 符合条件的数据。甚至不同的时间用相同的 url 拉取到的数据都不一样。故 step 1 下载的不可能是完整的数据，进而 step 4 的第二个完整性检查往往无法通过，于是 step 5 由此而生。`数据补全的过程就是重复 step 3 - 5，直到 step 4 检查通过（无法再往数据库中增补数据）`。好消息是实验结果是这个循环是可以收敛的。

在文件开头的 `# Arguments` 的部分可以看到该程序可以指定的参数：
```python
output_folder_path = 'xxx'
n_threads = 8
```
 - `output_folder_path`：输出临时 csv 文件存储路径。
 - `n_threads`：并行参数，线程数。

---
### step6_linker.py

该程序将 `google_patent_data_common` 的 `assignee` 字段初步过滤后直接匹配A股上市公司，将匹配结果 `专利号 - A股上市公司代码` 存入 `线程负责范围.csv` 临时 csv 文件。

该程序使用了轮转日志和多进程加速，每个进程对应一个日志文件，日志文件在相对路径 `/log/` 文件夹下，后缀数字为 `pid`。

在文件开头的 `# Arguments` 的部分可以看到该程序可以指定的参数：
```python
output_path = 'xxx'
n_processes = 8
```
 - `output_path`：输出临时 csv 文件存储路径。
 - `n_processes`：并行参数，进程程数。

---
### step7_store_linker.py

该程序将 `线程负责范围.csv` 临时 csv 文件里的 `A股上市公司代码` 转化成 `bopu_symbol`，并把 `patent_id - bopu_symbol` 入库至 `google_patent_symbol` 数据表。

在文件开头的 `# Arguments` 的部分可以看到该程序可以指定的参数：
```python
folders_linker = ['/home/swl/bopu/result_l']
```
 - `folders_linker`：存储 step 6 输出临时 csv 文件的文件夹。


## 有其他相关问题请联系
1067147135@qq.com（石雯岚）