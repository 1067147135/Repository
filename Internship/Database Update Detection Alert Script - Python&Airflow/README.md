# 数据库更新检测告警脚本 Database Update Detection Alert Script 

## 项目的目的

检查数据库的行情数据是否及时更新。

## 项目的基本思路

在每个交易日定时检查数据库的最新数据是否为当天数据。若不是，则数据库未及时更新，通过企业微信 bot 发送告警信息。

## 项目使用的技术栈

python 模块：
- pandas （查询 MySQL 数据库）
- pymysql （查询 MySQL 数据库）
- clickhouse_driver （查询 ClickHouse 数据库）
- datetime （处理日期时间数据）
- requests （操作企业微信机器人）
- json （编辑告警信息）
- airflow （周期性执行检查脚本）

## 项目的检查范围

### wind_db_check_alert.py

#### xx.xx.x.70 数据库

- **daily_db**: 
  - xxx
  - xxx
- **tdb_min**: 
  - xxx
  - xxx
  - xxx
  - xxx
  - xxx
  - xxx
  - xxx
  - xxx
  - xxx


### his_bopu_db_check.py

#### xx.xx.x.72 数据库
- **his_bopu**: 
  - xxx
  - xxx
  - xxx
  - xxx
  - xxx
  - xxx
  - xxx
  - xxx
  - xxx
  - xxx_x


#### xx.xx.x.52 数据库
- **bdf_backfill**:
  - xxx
  - xxx

## 项目实现的核心函数
- **send_alert**(db_name_list, old_list, new_list):
  - 向企业微信机器人发送告警信息
  - db_name_list：所有未及时更新数据库的数据库名称构成的 list
  - old_list：对应的数据库内当前最新数据的最新交易日期的 list
  - new_list：对应的数据库应当录入数据的最新交易日期的 list
  - e.g. 若今天交易日 2023-6-9 检查发现数据表 his_bopu.stock_1min 只有最新到 2023-6-8 的数据，则 db_name_list 包括 his_bopu.stock_1min, old_list 包含 2023-6-8，new_list 包含 2023-6-9。
  
- **check_future_date**(day):
  - 访问 xxx.cfuturescalendar 交易日历数据表，检查 day 是否为期货交易日。
  
- **check_stock_date**(day):
  - 访问 xxx.asharecalendar 交易日历数据表，检查 day 是否为股票交易日。

- **last_future_trade_day**(today)：
  - 访问 xxx.cfuturescalendar 交易日历数据表，查询 today 前的上一个交易日。

- **check_db_updated_1730**()，**check_db_updated_2030**()，**check_db_updated_2200**()：
  - 查询数据库更新维护情况，17：30 批次、20：30 批次、22：00 批次

## 检查时间
- 数据库检查有三批，分别是每个交易日的 17：30 批次、20：30 批次、22：00 批次。每个批次由一个 DAG 负责。其中
  - 17：30 批次包含 tdb_min 数据库下列出的的数据表，对应 **tdb_min_check_1730**
  - 20：30 批次包含 his_bopu 数据库和 bdf_backfill 数据库下列出的数据表，对应**his_bopu_check_2030**
  - 22：00 批次包含 daily_db 数据库下列出的的数据表，对应 **daily_db_check_2200**
- xxx_x 数据表为半夜更新，统一在下一个交易日的 20：30 批次检查上一个交易日数据的录入情况。

## 如何拓展
- 先对照下表，如果有信息完全吻合的，可以在对应 list 添加数据表的名称：

    list名称 | 检查批次 | 数据库IP | 数据库类型 | 商品类型 | 重试次数（间隔1小时）
    --- | --- | --- | --- | --- | ---
    tables_daily_db | 22：00 | xx.xx.x.70 | MySQL | 期货 | 1
    tables_tdb_min | 17：30 | xx.xx.x.70 | MySQL | 期货 | 3
    tables_stock | 20：30 | xx.xx.x.72 | ClickHouse | 股票 | 2
    tables_future | 20：30 | xx.xx.x.72 | ClickHouse | 期货 | 2
    tables_bdf_backfill | 20：30 | xx.xx.x.52 | ClickHouse | 股票 | 2
    - e.g. 如果想添加对数据表 bdf_backfill.xxx_xx 每个交易日 20：30 的数据更新检查，可以直接在 tables_bdf_backfill 里添加 xxx_xx。  
    添加前：
    `tables_bdf_backfill = ['xxx', 'xxx']`  
    添加后：
    `tables_bdf_backfill = ['xxx', 'xxx', 'xxx_xx']`
- 如果都不符合，建议参考：
    ```python
    # 要检查的数据库和数据表
    tables_xxx = ["xxx", "xxx"]

    # 查询数据库更新维护情况(xxxx批次)
    def check_db_updated_xxxx():
        db_name_list = [] 
        old_list = []
        new_list = []

        # 判断是否为交易日
        # 若希望一次检查多个数据库统一发送告警，可以复制这块模板用于不同的数据库
        # 参考check_db_updated_2030()
        if check_xxx_date(datetime.date.today()):
            # 期望的数据最新交易日
            target = datetime.date.today()
            # 若是访问 ClickHouse 数据库
            client = Client(host='xxx', port='9000', user='xxx', database='xxx', password='xxx')
            # 若是访问 MySQL 数据库
            conn = pymysql.connect(host='xxx', user='xxx', password='xxx', database='xxx')

            for table in tables_xxx:
                query = "SELECT max(trade_date) from xxx.{};".format(table)
                # 如果数据库没有更新，记录数据库名，最新更新日，和期望最新日
                # 若是访问 ClickHouse 数据库
                df = client.execute(query)
                if (df[0][0] != target):
                    db_name_list.append('xxx.{}'.format(table))
                    old_list.append(df[0][0])
                    new_list.append(target)
                # 若是访问 MySQL 数据库
                df = pd.read_sql(query, con=conn)
                if (df.iloc[0, 0] != target):
                    db_name_list.append('xxx.{}'.format(table))
                    old_list.append(df.iloc[0, 0])
                    new_list.append(target)

            # 若是访问 ClickHouse 数据库
            client.disconnect()
            # 若是访问 MySQL 数据库
            conn.close() 
        
        # 统一发送告警
        if db_name_list:
            retry_count = round((datetime.datetime.now() - datetime.datetime.now().replace(hour=xx, minute=xx, second=0, microsecond=0)).total_seconds() / 3600)
            send_alert(db_name_list, old_list, new_list, f"{retry_count}/x")
            # 触发 Airflow 重试
            raise AirflowException(f"xxx has not updated yet. Retry: {retry_count}/x.")


    # 把所有在这个批次运行的函数放这里
    def run_xxxx():
        check_db_updated_xxxx()

    # 设置重试次数和重试间隔
    default_args = {
        'retries': x,
        'retry_delay': datetime.timedelta(hours=1),
    }
    
    # 定义DAG
    dagxxxx = DAG(
        dag_id="xxx_check_xxxx",
        start_date=datetime.datetime(2023, 6, 5),
        schedule_interval='xx xx * * 1-5',
        catchup = False,
        tags= ["IT", "db check"], # tag 加上自己的名字
        default_args=default_args
    )

    taskxxxx = PythonOperator(
        task_id='jobxxxx',
        python_callable=run_xxxx,  # 指定要执行的函数
        dag=dagxxxx
    )

    ```
- 其他函数继承使用。(注意 send_alert() 里的具体告警信息应当个性化定制) 
- 将文件传到 root@xx.xx.x.53/home/airflow/airflow/dags/IT_group 下自己的文件夹下。  
- 到 http://xx.xx.x.53:8080/home 确认结果。

## 有其他相关问题请联系
1067147135@qq.com（石雯岚）