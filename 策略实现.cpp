#include<iostream>
#include<algorithm>
#include<cstring>
#include<map>

using namespace std;

//定义可转债结构体,bond[xxx]表序号为xxx 可转债的信息 
struct Bond
{
    string name;
    string code;
    double close;
    double premium_rate;
    double factor;
};

struct all_bond
{
    Bond bond[100];
};

//Day_Bond["n"] 表示第n 天所有可转债的信息,Day_Bond["n"].bond[xxx].info 表示在第n 天序号为xxx 的可转债的info 信息
map<string,all_bond> Day_Bond;


//重载小于号，按照factor 对结构体数组进行排序
bool cmp(Bond &a,Bond &b)
{
    return a.factor < b.factor;
}

//定义仓位
struct position()
{
    double total_fund;
    struct selected_bond[10];
}

//写入第day天的文件信息 
void write_infod(string day)
{
    int cnt = 0;
    string line;
    ifstream infile(day);
    while(getline(infile,line))
    {
        stringstream linestream(line);
        linestream >> bond[cnt].name >> bond[cnt].code >> bond[cnt].close >> bond[cnt].premium_rate;
        bond[cnt].factor = bond[cnt].close + bond[cnt].premiunr_reate;
        cnt++;
    }
    infile.close();
}

int main()
{
    write_info();
    sort(begin(bond),end(bond),cmp);

}




