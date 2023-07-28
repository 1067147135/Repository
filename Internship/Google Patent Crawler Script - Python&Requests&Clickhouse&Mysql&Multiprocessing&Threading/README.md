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

7. `入库`：将匹配成功的链接关系（`专利号 - A 股公司代码`）入库至 `google_patent_symbol` 数据表。


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

下载链接生成的默认筛选参数是：
 - 按照优先权日期
 - 仅限专利
 - 结果以最新优先排序

该程序有两个可指定的参数要运行时传入：
 - `begin`、`end`：爬取日期范围，包含 begin 和 end 这两天。
```bash
python step1_download_requests.py <begin> <end>
# e.g. python step1_download_requests.py 20210101 20210106
```

该程序其他可指定的硬编码参数在 `config.py` 的 `# step1` 部分：
```python
# step1
download_path = 'xxx'
countrys = ["CN"] # , "US"
cookie = 'xxx'
user_agent = 'xxx'
```
 - `download_path`：csv 文件下载存储路径。（csv 文件名是自动生成的 `日期-国家.csv`。）
 - `countrys`：计划爬取的国家列表，可以添加其他国家的简称（e.g. US）以爬取更多的国家的专利数据。
 - `cookie`：请求头的Cookie。当程序因为 429 封禁终止时，需要更换一个新的有效的 Cookie 才可以继续爬取。新的 Cookie 可以手动通过浏览器访问下载地址时通过 F12 开发者页面抓请求头的包复制获得。如果手动访问也 429 的话，就找同事帮忙提供一下他们的 Cookie。一般情况下 1 个 Cookie 使用到 429 封禁之前可以爬取 1 个国家将近 1 年份的数据。
 - `user-agent`：请求头的浏览器客户端类型。该参数基本不需要改动。

---
### step2_analysis_web_only.py

该程序使用 requests 逐条访问 csv 文件中每个专利的详情页，用 BeautifulSoup 解析拉取到的 html 提取概要等额外信息，并将除了被引用信息以外的信息存入 `日期-国家-common.csv` 临时 csv 文件，将被引用信息存入 `日期-国家-cite.csv` 临时 csv 文件。

该程序使用并行技术，同时使用多进程和多线程。进程之间以通过 `multiprocessing.Manager().Queue()` 实现动态负载均衡，一个进程一次任务分析一整个原始 csv 文件。线程之间使用静态分配均分任务，一个线程一次任务分析一个原始 csv 文件的 `1/线程数` 。

该程序已通过 logging 实现轮转日志，日志存储于相对路径 `/log/` 文件夹下。每个线程对应一个日志文件，其后缀数字为 `10 * pid + tid` 。（后缀数字是指 `.log` 之前的数字， `.log` 之后的是轮转日志备份编号）

该程序可指定的参数在 `config.py` 的 `# step2` 部分：
```python
# step2
step2_input_folder_path = 'xxx'   # path to folder
step2_output_folder_path = 'xxx'
step2_n_threads = 1
step2_n_processes = 1
```
 - `step2_input_folder_path`: 原始 csv 文件的存储路径。程序将从这个文件夹中逐个读取 csv 文件进行爬取分析。
 - `step2_output_folder_path`：结果临时 csv 文件的存储路径。程序将把分析完后生成的临时 csv 文件 `日期-国家-common.csv` 和 `日期-国家-cite.csv` 存储到该路径下。
 - `step2_n_threads`：并行参数，每个进程的线程数。
 - `step2_n_processes`：并行参数，该程序的进程数。
  
---
### step2_analysis_db_web_fast.py

该程序是 `step2_analysis_web_only.py` 的需求特化加速版本，适用于重新爬取分析的场合，即：当完成一轮 step 1 - 3 之后，爬取需求从 `按申请日期` 变更为 `按优先权日期`。一般而言这需要重新跑一轮 step 1 - 3，但 step 2 是非常耗时的，分析三年的数据要数十个小时。考虑到两个需求中有大部分数据是重合的，且 step 2 最耗时的部分是访问网页，更改爬取详情页部分的逻辑为：优先比对数据库，若数据库已有该条数据，则从数据库拷贝，若数据库不存在该数据，再去访问详情页。

为了减少对数据库的访问次数，降低数据库负载：每分析一个原始 csv 文件，从数据库拉取一次当天的 common 数据存入字典；程序运行时，一次性拉取全部 cite 数据存入字典。后续比对使用字典。（截至撰写该文档时，common 总数据约 750w 条，cite 总数据约 69w 条）

**注意**：如果要使用该程序，一定要根据使用时的需求，个性化修改 `db2dict_common(date_str)` 和 `db2dict_cite()` 方法里的 sql 语句。

该程序的其他参数设置以及其他特点和 `step2_analysis_web_only.py` 完全一致，不再赘述。

---
### split.py

该程序是一个文件分类小工具，它可以把存在一个文件夹里的不同后缀的临时 csv 文件分类转移到两个文件夹里，以便于使用 `step3_warehouse.py`。当然如果文件不多，手动分类，就不需要用这个小工具。

该程序可指定的参数在 `config.py` 的 `# split` 部分：
```python
# split
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

该程序可指定的参数在 `config.py` 的 `# step3` 部分：
```python
# step3
folders_common = ['xxx']
folders_cite = ['xxx']
```
 - `folders_common`：存有 common 数据的临时 csv 文件所在的文件夹列表。
 - `folders_cite`：存有 cite 数据的临时 csv 文件所在的文件夹列表。

---
### step4_check.py

该程序对入库的 common 和 cite 数据进行引用关系完整性检查。
 - `google_patent_data_cite` 表的 `patent_id` 是否在 `google_patent_data_common` 表都有。
 - `google_patent_data_cite` 表的 `cited_by_patent_id` 中以 CN 开头的部分是否在 `google_patent_data_common` 表都有。

**注意**：如果爬取国家发生变化，此处检查的条件也应当相应变更，注意修改主程序里的 `query_cited_by_id` 以满足需求。

该程序无需参数直接运行，输出结果在当前文件夹的 `output.txt` 。


---
### step5_replenish.py

该程序会检查不满足第二个完整性条件时缺少的专利号，并上网爬取补全。爬取的到的数据中，除了被引用信息以外的信息将被存入 `replenish-common.csv` 临时 csv 文件，被引用信息将被存入 `replenish-cite.csv` 临时 csv 文件。

将 `replenish-common.csv` 和 `replenish-cite.csv` 补充入库需要将它们分开放到特定文件夹并使用 `step3_warehouse.py` 入库。

该程序使用了轮转日志和多线程加速，每个线程对应一个日志文件，日志文件在相对路径 `/log/` 文件夹下，后缀数字为 `tid`。

因为数据是通过搜索引擎收集的，而搜索引擎因为性能限制和受众特点只保证返回 “大量” 符合条件的相关数据以供来浏览，不保证返回 “全部” 符合条件的数据。甚至不同的时间用相同的 url 拉取到的数据都不一样。故 step 1 下载的不可能是完整的数据，进而 step 4 的第二个完整性检查往往无法通过，于是 step 5 由此而生。`数据补全的过程就是重复 step 3 - 5，直到 step 4 检查通过（无法再往数据库中增补数据）`。好消息是实验结果是这个循环是可以收敛的。

该程序可指定的参数在 `config.py` 的 `# step5` 部分：
```python
# step 5
step5_output_folder_path = 'xxx'
step5_n_threads = 1
```
 - `step5_output_folder_path`：输出临时 csv 文件存储路径。
 - `step5_n_threads`：并行参数，线程数。

---
### step6_linker.py

该程序将 `google_patent_data_common` 的 `assignee` 字段初步过滤后直接匹配A股上市公司，将匹配结果 `专利号 - A股上市公司代码` 存入 `线程负责范围.csv` 临时 csv 文件。

该程序使用了轮转日志和多进程加速，每个进程对应一个日志文件，日志文件在相对路径 `/log/` 文件夹下，后缀数字为 `pid`。

该程序可指定的参数在 `config.py` 的 `# step6` 部分：
```python
# step 6
step6_output_folder_path = 'xxx'
step6_n_threads = 1
```
 - `step6_output_folder_path`：输出临时 csv 文件存储路径。
 - `step6_n_threads`：并行参数，进程程数。

---
### step7_store_linker.py

该程序将 `线程负责范围.csv` 临时 csv 文件里的 `A股上市公司代码` 转化成 `bopu_symbol`，并把 `patent_id - bopu_symbol` 入库至 `google_patent_symbol` 数据表。

该程序可指定的参数在 `config.py` 的 `# step7` 部分：
```python
# step 7
folders_linker = ['xxx']
```
 - `folders_linker`：存储 step 6 输出临时 csv 文件的文件夹。

## Assignee 与 A 股上市公司的匹配逻辑

1. 匹配目标预处理 - 仅匹配 wind_db.asharedescription 中已上市公司的公司交易代码，忽略英文公司名大小写：  
（1）忽略交易代码 S_INFO_WINDCODE 以 “A” 开头的公司。  
（2）忽略公司简称 S_INFO_NAME 包含“(IPO终止)”或“(退市)”的公司。  
（3）忽略上市日期 S_INFO_LISTDATE 为空的公司。  
（4）英文公司名转小写。  
（5）若同一个公司有多个交易代码，以上市日期最新的交易代码为准。  
1. 匹配时逻辑：  
（1）格式化并切分专利的 assignee：中文公司名删除“（Cn）”，中文格式括号转为英文格式括号，然后按“, ”切分；英文公司名转小写，删除带“, ”的公司后缀（如“ co., ltd”）防止干扰切分，切分后再删除其他公司后缀（如“ corp”）防止因格式不一致而失配。  
（2）对每个切分出来的 assignee 逐个匹配：若 assignee 包含“university”或者“大学”，要求按（1）格式化后结果完全一致才算匹配成功；若 assignee 为中文且字数小于 5，判定为人名不进行匹配；其他情况用 in 运算符匹配，若匹配到多个公司，以公司名最短（即最接近的）为最终结果。
1. 匹配后补正：  
（1）分别核查程序匹配结果和通联数据库怕匹配结果的交集和差集，筛选特殊的难以规避的匹配错误情况，拎出来最后统一删除。
 
## 有其他相关问题请联系
1067147135@qq.com（石雯岚）