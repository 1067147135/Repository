# yahoo_finance_data

## 项目的目的

从 [Yahoo finance](https://finance.yahoo.com/) 抓取 SPY，DIA，QQQ这3个美股ETF的日线数据。

## 项目的基本思路
- `check_update_time.py` 每周二到周六 4 点到 10 点期间，每隔十分钟访问接口查询日线数据。用于前期监控 [Yahoo finance](https://finance.yahoo.com/) 的数据更新时间，以确定将爬虫任务定时在什么时候执行。（程序输出参考 `src/log/monitor.log`）
- `index_eod_prices_update.py` 每周二至周六早上 6 点从 [Yahoo finance](https://finance.yahoo.com/) 抓取 SPY，DIA，QQQ这3个美股ETF的日线数据并入库至 72 Clickhouse 数据库的 his_global.index_eod_prices 数据表。没有抓到数据或者数据入库失败都将触发通过企业微信 bot 发送的告警，并在 1 小时后重试，最多重试 2 次。（程序日志参考 `src/log/index_eod_prices_update_0600.log`）
- `index_eod_prices_check.py` 每周二至周六早上 6 点半检查数据库是否成功录入今天的数据，录入的数据是否一致（最高价最大，最低价最小），未录入数据或者录入数据不一致都会触发通过企业微信 bot 发送的告警。
- 项目使用的参数全部提取至 `my_config.py` 统一管理。

## 项目使用的技术栈

python 模块：
- clickhouse_driver （查询 ClickHouse 数据库）
- datetime （处理日期时间数据）
- requests （操作企业微信机器人，爬取 Yahoo finance 上的数据）
- json （编辑告警信息，解析 Yahoo finance 接口的返回数据结果）
- airflow （周期性执行更新脚本）
- logging （轮转日志，在记录运行信息的同时限制 log 的内存占用）
- base64 （加密数据库密码等敏感信息）
- sched  (设定定时执行任务)


## 项目实现的核心函数
- **job_monitor()**：
  - 通过 sched 控制，每周二到周六 4 点到 10 点期间，每 10 分钟执行一次。
  - 通过两个途径（网页端使用的 query1 接口，和 yfinance 封装使用的 query2 接口）获取三支 etf 的日线数据，并将数据存到 `/src/log/monitor.log` 以备分析。
- **index_eod_prices_update()**: 
  - 从 [Yahoo finance](https://finance.yahoo.com/) 抓取 SPY，DIA，QQQ这3个美股ETF的日线数据并入库至 72 Clickhouse 数据库的 his_global.index_eod_prices 数据表。
  - 数据库入库前先做查重。
  - 如果抓不到数据，或者入库失败，则告警。
- **index_eod_prices_check()**:
  - 从 72 Clickhouse 数据库读取前一个交易日的日线数据，检查数据一致性（“开高收低”四个价格应该“高”最大，“低”最小）。
  - 如果读取不到数据，或者数据不一致，则告警。

## 涉及数据库的结构 

- 字段信息：
  
    | 字段中文名 | 字段名 | 字段类型 | 注释 |
    | ------ | ------ |------ |------ |
    | 合约代码 | symbol | String | |
    | 交易日 | trade_date | Date | |
    | 开盘价 | open | Float64 | |
    | 最高价 | high | Float64 | |
    | 最低价 | low | Float64 | |
    | 收盘价 | close |Float64 | |
    | 博普更新时间 | bopu_update_time  | DateTime64 | DEFAULT toDateTime64(toString(now64(6)), 6, 'UTC') |

- 数据库引擎：ReplicatedReplacingMergeTree ORDER BY (trade_date, symbol)
- 数据分区：PARTITION BY toYYYYMM(trade_date)

## 如何拓展
1. 该项目的相关环境部署路径为：  

    环境 | IP | 项目路径 | UI界面地址
    --- | --- | --- | --- 
    测试环境 | xx.xx.x.xx | $AIRFLOW_HOME/dags/path/to/personal_folder/ | http://xx.xx.x.xx:8080/home
    正式环境 | xx.xx.x.xx | $AIRFLOW_HOME/dags/path/to/personal_folder/ | http://xx.xx.x.xx:8080/home

    数据库地址账号以及webhook地址在 `my_config.py`，数据库密码使用 base64 加密。

2. `index_eod_prices_update.py` 里有两个 list ，`etf` 存储的是用于构成访问的 Yahoo 的网络接口的美股简称，`etf_dict` 存储的是 `etf` 里的美股简称和 symbol 列的映射关系，比如 `SPY` 对应的 symbol 是 `SPY US Equity`。如果需要拓展访问的日线数据可以用以下格式的接口获取到，则可以通过将简称和映射关系同时存入 `etf` 和 `etf_dict` 完成拓展：
   ```python
   # 获取前一天日线数据的接口格式
   # etf 内存储的内容能填到 etf_tag 处即可使用此拓展方法，实测 etf_tag 的位置也可以填 Yahoo 支持的期货代码，如 CLG28.NYM，可以获取到期货的日线数据
   # p1 是起始时间，程序默认填北京时间前一天上午 8 点整的 timestamp
   # p2 是结束时间，程序默认填北京时间当天上午 8 点整的 timestamp
   url = f"https://query1.finance.yahoo.com/v8/finance/chart/{etf_tag}?interval=1d&period1={p1}&period2={p2}"

   # 拓展前
   etf = ['SPY', 'QQQ']
   etf_dict = {'SPY': 'SPY US Equity', 'QQQ': 'QQQ US Equity'}

   # 拓展后
   etf = ['SPY', 'QQQ', 'DIA']
   etf_dict = {'SPY': 'SPY US Equity', 'QQQ': 'QQQ US Equity', 'DIA': 'DIA US Equity'}

   ```


## 避坑指南
- 正式环境里的 DAG 文件不能包含任何中文，包括注释（特别要注意中文符号，如逗号、句号、括号，这种一眼看不出来），否则 UI 界面的 DAG 会热更新失败。具体的编译 log 可以去 `$AIRFLOW_HOME/logs/scheduler` 当日目录对应 dag 文件夹下面看。
- Airflow 执行 Python 文件时的依赖路径和直接执行 Python 文件时的依赖路径不一样，调用自己写的包（如 `my_config.py`）会出现 `ModuleNotFoundError`，可以参照如下代码在 Python 文件里添加依赖路径。
  ```python
  import os
  import sys
  my_module_path = os.path.abspath('$AIRFLOW_HOME/dags/path/to/my_config.py')
  sys.path.append(os.path.dirname(my_module_path))
  import my_config
  ```
- ReplicatedReplacingMergeTree 会在插入数据的时候进行查重，在索引 (trade_date, symbol) 出现重复的时候用新数据覆盖旧数据，但这个查重操作是异步的，如果在非常短的时间内插入两条索引 (trade_date, symbol) 一样的数据，那这两条数据都会成功入库，产生冗余。
- 72 数据库环境的 Clickhouse 版本为 20.4.5.36，`insert data in all the columns, except 'b'` 这个功能在 20.11.1.5109 版本才上线（根据Clickhouse github 文档更新版本）所以入库时只能采用指定字段名的方式。
- log 文件新建出来的时候默认其他用户是只有读取权限没有写入权限的，也就是说 airflow 执行的时候可能会因为 permission denied 无法 a 模式打开 log 文件而执行失败。可以使用 `ls -l` 查看当前文件夹下文件的权限，再使用 `chmod 666 <file_name.log>` 赋予所有用户对这个文件的读写权限。

## 有其他相关问题请联系
1067147135@qq.com（石雯岚）