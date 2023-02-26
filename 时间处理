#include<iostream>
#include<algorithm>
#include<ctime>

using namespace std;

//关于tm，tm 结构体年份从1900 开始,月份从0 开始计数
//time_t 数据类型为时间戳,为1970 年1 月1 日到某个时间点的秒数
//mktime() 将tm 结构体转化为time_t 时间戳


//实现将日期转换为时间戳
time_t time_stamp(int year,int month,int day)
{
    tm t = {};
    int y = year - 1900;
    t.tm_year = y;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    time_t ans = mktime(&t);
    return ans;
}

int main()
{
    //定义开始,结束时间的时间戳
    time_t st = time_stamp(2022,1,1);
    time_t ed = time_stamp(2022,12,12);
    for(time_t init = st;init <= ed;init += 24 * 60 * 60)
    {
        tm* local_time = localtime(&init);
        char data[11];
        strftime(data,sizeof(data),"%Y-%m-%d",local_time);
        cout << data << endl;
    }


}
