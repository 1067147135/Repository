import datetime

def generate_time_by_month(begin: str, end: str):
    """
    generate time interval by month
    - begin: e.g. '20230107'
    - end: e.g. '20230307'
    """
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

def generate_time_by_day(begin: str, end: str):
    """
    generate date by day
    - begin: e.g. '20230107'
    - end: e.g. '20230307'
    """
    start_date = datetime.datetime.strptime(begin, '%Y%m%d')
    end_date = datetime.datetime.strptime(end, '%Y%m%d')
    result = []

    while start_date <= end_date:
        result.append(start_date.strftime('%Y%m%d'))
        start_date += datetime.timedelta(days=1)

    return result