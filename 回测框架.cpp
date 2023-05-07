#include<iostream>
#include<algorithm>
#include<ctime>
#include<set>
#include<fstream>
#include<sstream>
#include<map>
#include<string>

using namespace std;

const int N = 365 * 10 + 10;

string backtest_scope[N]; //存储回测所用的时间段
int total_days = 0; //回测所用总天数
double Capital[N]; //存储每天仓位的价值
int Capcnt; //capital 的下标索引
map<string,pair<double,double>> mp_amount;
map<string,pair<string,string>> mp_day;
map<string,pair<double,double>> mp_price;

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
        strftime(data,sizeof(data),"%Y%m%d",local_time);
        backtest_scope[total_days++] = data;
    }
    for(int i = 0;i < total_days;i++) backtest_scope[i] += ".csv";
}

//定义可转债结构体,bond[xxx]表索引为xxx 可转债的信息
struct Bond
{
    string name;
    double open;
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
    double holding_number;
    Bond holding[20];  //持仓的可转债
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

//Capital.csv
void write_Capital(string day,double fund)
{
    ofstream outfile("Capital.csv",ios_base::app); // 打开文件,构造输出流对象outfile 并绑定文件
    outfile << day << '\t' << fund << endl;
    outfile.close();
}

//更新仓位
void update(string day,position &capital)
{
    capital.val_bond = 0;
    int lot = 10;
    int lots = 5 * 10;
    double fee = 0.003;
    double slipp = 0.003;
    int cnt = 0; // 记录当天可转债的数量
    Bond bond[1000]; // 存储当天可转债的数据
    string line;
    ifstream infile("/Users/charles/Convertible_bond_strategy/数据源/" + day);
    if (!infile) cout << "Failed to open file!" << endl;

    getline(infile,line);
    //将当日的可转债记录在bond[] 中
    while(getline(infile,line))
    {
        int x;
        stringstream linestream(line);
        string name;
        double open;
        double high;
        double low;
        double close;
        double premium_rate;
        linestream >> x >> name >> open >> high >> low >> close >> premium_rate;
        double factor = open + premium_rate;
        bond[cnt++] = {name,open,premium_rate,factor};
    }
    infile.close();

    sort(bond,bond + cnt); // 将当天所有可转债按照factor 从小到大排序

    //更新仓位,若仓位中的可转债不在前十则将其卖出
    for(int i = 0;i < 20;i++)
    {
        if(capital.holding[i].name == "null") continue;
        //不卖
        int ck = 0;
        string temp = capital.holding[i].name;
        for(int j = 0;j < 20;j++)
        {
            if(temp == bond[j].name)
            {
                capital.val_bond += bond[j].open * lots;
                ck = 1;
                break;
            }
        }
        if(ck) continue;

        //卖
        double sell_price = 0;
        int note = 0;
        mp_day[temp].second = day.substr(0,day.length() - 4);
        for(int z = 20;z < cnt;z++)
        {
            //未退市
            if(temp == bond[z].name)
            {
                note = 1;
                sell_price = bond[z].open * (1 - slipp) * (1 - fee);
                break;
            }
        }
        //已退市
        if(note == 0) sell_price = capital.holding[i].open;

        capital.val_cash += sell_price * lots;
        mp_price[temp].second = sell_price;
        mp_amount[temp].second = sell_price * lots;

        ofstream file("/Users/charles/Convertible_bond_strategy/入场出场价.csv",ios_base :: app);
        file << temp << '\t' << mp_day[temp].first << '\t' << mp_day[temp].second << '\t' << mp_price[temp].first << '\t' << mp_price[temp].second << '\t' << lots << '\t' << mp_amount[temp].first << '\t' << mp_amount[temp].second << '\t' << mp_amount[temp].second - mp_amount[temp].first << endl;
        file.close();

        mp_price[temp].first = mp_price[temp].second = 0;
        mp_day[temp].first = mp_day[temp].second = "null";
        mp_amount[temp].first = mp_amount[temp].second = 0;
        capital.holding[i] = {"null",0,0,0};
    }

    //更新仓位,将新晋前十的可转债买入
    for(int i = 0;i < 20;i++)
    {
        //不买
        int ck = 0;
        string temp = bond[i].name;
        for(int j = 0;j < 20;j++)
        {
            if(temp == capital.holding[j].name)
            {
                ck = 1;
                break;
            }
        }
        if(ck == 1) continue;

        //买
        double buy_price = 0;
        buy_price = bond[i].open * (1 + fee) * (1 + slipp);
        if(capital.val_cash >= buy_price * lots && bond[i].open != 0)
        {
            capital.val_cash -= buy_price * lots;
            capital.val_bond += bond[i].open * lots;
            for(int z = 0;z < 20;z++)
            {
                if(capital.holding[z].name == "null")
                {
                    capital.holding[z] = bond[i];
                    capital.holding[z].open = buy_price;
                    break;
                }
            }
            mp_price[temp].first = buy_price;
            mp_day[temp].first = day.substr(0,day.length() - 4);
            mp_amount[temp].first = buy_price * lots;
        }

    }
    capital.total_fund = capital.val_cash + capital.val_bond;
    Capital[Capcnt++] = capital.total_fund;
    write_Capital(day.substr(0,day.length() - 4),capital.total_fund);
    write_into(day,capital);
}

//初始化仓位
void init_poi(position &capital,double f,double c,double b)
{
    capital.total_fund = f;
    capital.val_cash = c;
    capital.val_bond = b;
    capital.holding_number = 0;
    for(int i = 0;i < 20;i++) capital.holding[i] = {"null",0,0,0};
}

//清空文件
void clc(string path)
{
    ofstream myfile(path, ios::trunc); //打开文件并清空它
    myfile.close(); //关闭文件
}

int main()
{
    //初始化回测时间段
    backtest_scope_init(2018,5,30,2022,4,30);

    //初始化仓位
    position capital;
    double inicash = 1e5;
    init_poi(capital,inicash,inicash,0);

    clc("/Users/charles/Convertible_bond_strategy/入场出场价.csv");
    clc("/Users/charles/Convertible_bond_strategy/Capital.csv");

    ofstream file("/Users/charles/Convertible_bond_strategy/入场出场价.csv",ios_base :: app);
    file << "名称" << '\t' << "进场时间" << '\t' << "出场时间" << '\t' << "买入价" << '\t' << "卖出价" << '\t' << "份额" << '\t' << "买入金额" << '\t' << "卖出金额" << '\t' << "利润" << endl;
    file.close();

    for(int i = 0;i < total_days;i++)
    {
        int cnt = 0;
        string s;
        ifstream file1("/Users/charles/Convertible_bond_strategy/数据源/" + backtest_scope[i]);
        while(getline(file1,s))
        {
            cnt++;
            if(cnt > 3)
            {
                update(backtest_scope[i],capital);
                break;
            }
        }
        file1.close();
    }
    return 0;
}
