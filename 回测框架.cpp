#include<iostream>
#include<algorithm>
#include<ctime>
#include<set>
#include<fstream>
#include<sstream>

using namespace std;

const int N = 365 * 5;

string backtest_scope[N]; //存储回测所用的时间段
int total_days = 0; //回测所用总天数

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


//初始化backtest_scope[N],存储格式:backtest_scope[k] = 2022-01-01.csv
//参数: 回测开始及结束的时间

void backtest_scope_init(int sty,int stm,int std,int edy,int edm,int edd)
{
    //定义开始,结束时间的时间戳
    time_t st = time_stamp(sty,stm,std);
    time_t ed = time_stamp(edy,edm,edd);

    //从回测起点开始循环读入日期到backtest_scope[N]中
    for(time_t init = st;init <= ed;init += 24 * 60 * 60)
    {
        tm* local_time = localtime(&init);
        char data[30];
        strftime(data,sizeof(data),"%Y-%m-%d",local_time);
        backtest_scope[total_days++] = data;
    }
    for(int i = 0;i < total_days;i++)
    {
        backtest_scope[i] += ".csv";
    }
}


//定义可转债结构体,bond[xxx]表索引为xxx 可转债的信息

struct Bond
{
    string name;
    string code;
    double close;
    double premium_rate;
    double factor;

    //重载小于号，按照factor 对结构体数组进行排序
    bool operator <(const Bond & x)const
    {
        return factor < x.factor;
    }
};


//定义仓位

struct position
{
    double total_fund;
    double val_cash;
    double val_bond;
    Bond holding[10];  //持仓的可转债
};


//将资金每日的变化写入curve_data.csv 文件

void write_into(string day,position capital)
{
    ofstream outfile("curve_data.csv",ios_base::app); // 打开文件,构造输出流对象outfile 并绑定文件
    string nday = day;
    nday.erase(nday.length() - 4);
    outfile << nday << ' ' << capital.total_fund << ' ' << capital.val_cash << ' ' << capital.val_bond << endl;
    outfile.close();
}


//更新仓位

void update(string day,position capital)
{
    int cnt = 0; // 记录当天可转债的数量
    Bond bond[300]; // 存储当天可转债的数据
    string line;
    ifstream infile(day);
    if (!infile) cout << "Failed to open file!" << endl;

    //将当日的可转债记录在bond[] 中
    while(getline(infile,line))
    {
        stringstream linestream(line);
        string name;
        string code;
        double close;
        double premium_rate;
        linestream >> name >> code >> close >> premium_rate;
        double factor = close + premium_rate;
        bond[cnt++] = {name,code,close,premium_rate,factor};
    }
    infile.close();

    sort(bond,bond + cnt); // 将当天所有可转债按照factor 从小到大排序

    //预处理已持有和当天的可转债
    string poi_bond[10];  // 已持有的可转债名称
    string new_bond[10];  // 当天可转债的名称
    for(int i = 0;i < 10;i++) poi_bond[i] = capital.holding[i].name;
    for(int i = 0;i < 10;i++) new_bond[i] = bond[i].name;

    //更新仓位,若仓位中的可转债不在前十则将其卖出
    for(int i = 0;i < 10;i++)
    {
        bool ck = 0;
        string temp = poi_bond[i];
        for(int j = 0;j < 10;j++) if(temp == new_bond[j]) ck = 1;
        if(!ck)
        {
            capital.val_bond -= capital.holding[i].close;
            for(int z = 0;z < 300;z++)
            {
                if(temp == bond[z].name) capital.val_cash += bond[z].close;
            }
            capital.holding[i] = {"NULL","NULL",0,0,0};
        }
    }

    //更新仓位,将新晋前十的可转债买入
    for(int i = 0;i < 10;i++)
    {
        bool ck = 0;
        string temp = new_bond[i];
        for(int j = 0;j < 10;j++) if(temp == poi_bond[j]) ck = 1;
        if(!ck)
        {

            capital.val_cash -= bond[i].close;
            capital.val_bond += bond[i].close;
            for(int z = 0;z < 10;z++) if(capital.holding[z].name == "NULL") capital.holding[z] = {bond[i].name,bond[i].code,bond[i].close,bond[i].premium_rate,bond[i].factor};
        }
    }

    capital.total_fund = capital.val_cash + capital.val_bond;
    write_into(day,capital);
}

//初始化仓位
void init_poi(position capital,double f,double c,double b)
{
    capital.total_fund = f;
    capital.val_cash = c;
    capital.val_bond = b;
}

//清空curve_data.csv 文件
void clc()
{
    ofstream myfile("curve_data.csv", ios::trunc); //打开文件并清空它
    myfile.close(); //关闭文件
}

int main()
{
    //初始化回测时间段
    backtest_scope_init(2022,1,1,2022,1,30);

    //初始化仓位
    position capital;
    init_poi(capital,1e6,1e6,0);

    clc();

    for(int i = 0;i < total_days;i++) update(backtest_scope[i],capital);
    return 0;
}

