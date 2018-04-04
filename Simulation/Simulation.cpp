#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <vector>
using namespace std;
int random(int start,int end){
    return start+(end-start)*rand()/(RAND_MAX + 1);
}
class Game
{
public:
    //string name;
    int fps; //无GPU抢占时的fps
    int max_tolerate;//允许的最大fps削弱
    int resource;//最大占用资源数（数值上等于抢夺GPU的能力）
    float slope;// GPU-fps曲线斜率，由点(1,0)得偏移shift = -slope
    int serverID;
    Game(){
        this->fps = random(40,120);
        this->max_tolerate = fps-30;//我希望fps越大时，最大的容忍fps丢失也能越大
        this->resource = random(fps/2,fps);//我希望max_tolerate越小时抢夺能力越强
        this->slope = fps;//获得资源-fps 曲线的斜率
        serverID = -1;
    }
    Game(Game *g){
        this->fps = g->fps;
        this->resource = g->resource;
        this->max_tolerate = g->max_tolerate;
        this->slope = g->slope;
        serverID = -1;
        //this->name = g->name;
    }
    // 返回当分配给resource资源时的fps
    int GetFPS(int resource){
        if(resource >= this->resource)
            return fps;
        return (int)(slope * resource/this->resource );
    }
};
// 当这些游戏放一台容量为Capacity的服务器时，获取最小fps
// 其中games已经根据fps排序
int GetMinFPS(vector<Game> games,int Capacity)
{
    int total = 0;
    for(int i=0;i<games.size();++i)
        total += games[i].resource;
    if(total <= Capacity)
        return games[0].fps;
    int min = games[0].GetFPS(games[0].resource * Capacity/total);
    for(int i=1;i<games.size();++i)
    {
        int temp = games[i].GetFPS(games[i].resource * Capacity/total);
        // 这些游戏不能放在一起
        if((games[i].fps - temp)>games[i].max_tolerate)
            return -1;
        if(temp<min)
            min = temp;
    }
    return min;
}

// 传统贪心算法分配
int Patch(vector<Game>games,int ServerNum,int Capacity)
{

    // 安排的游戏数目
    int count = ServerNum;
    int minfps = games[0].fps;
    vector<Game>server[ServerNum];
    vector<vector<Game>>servers;
    for(int i=0;i<ServerNum;++i)
        servers.push_back(server[i]);
    // 将前ServerNum个游戏放入服务器中
    for(int i=0;i<ServerNum&&i<games.size();++i)
        servers[i].push_back(games[i]);
    // 对第count个game安排服务器
    while(count < games.size())
    {
        // 若将游戏放入每个服务器，所得到的每台服务器的最小fps
        int fps[ServerNum];
        // 对每个服务器，都去尝试一下获得fps
        for(int i=0;i<ServerNum;++i)
        {
            servers[i].push_back(games[count]);
            fps[i] = GetMinFPS(servers[i],Capacity);
            servers[i].pop_back();
        }
        // 寻找使fps最大的那个server
        int id= 0;
        int max = fps[0];
        for(int i=0;i<ServerNum;++i)
            if(max < fps[i])
            {
                id = i;
                max = fps[i];
            }
        servers[id].push_back(games[count]);

        if(max < minfps)
            minfps = max;
        count++;
    }
    return minfps;
}
// 对单个服务器，动态规划分配games,使其完美运行
int Dynamic_Patch(vector<Game>games,int ServerResource)
{
    int minFps[games.size()][ServerResource];
    for(int i=0;i<games.size();++i)
        for(int j=0;j<ServerResource;++j)
            minFps[i][j] = 0;
    //前i个游戏，空间为j
    for(int i=0;i<games.size();++i)
    {
        for(int j=0;j<ServerResource;++j)
        {
            //总空间无法放下第i个游戏
            if(j<games[i].resource)
                minFps[i][j] = minFps[i-1][j];
            if(i == 0)//对第一款游戏，即矩阵的第一行，直接赋值
            {
                minFps[i][j] = games[i].fps;
                continue;
            }
            int put;
            if(minFps[i-1][j-games[i].resource]!=0)
                put = min(minFps[i-1][j-games[i].resource],games[i].fps);
            else
                put = games[i].fps;
            if(put >=minFps[i-1][j])
                minFps[i][j] = put;
            else
                minFps[i][j] = minFps[i-1][j];
        }
    }
    return minFps[games.size()-1][ServerResource-1];
}


int main()
{
    srand(unsigned(time(0)));
    vector<Game>games;
    for(int i=0;i<20;++i)
        games.push_back(new Game());
    // 先按照fps升序排序
    for(int i=0;i<games.size();++i)
        for(int j=i+1;j<games.size();++j)
            if(games[i].fps>games[j].fps)
                swap(games[i],games[j]);
    //cout<<games.size();
    int ServerNum = 7;
    // 每台服务器的资源量
    // 当资源不足时，将根据游戏的资源请求线性分配
    int ServerResource = 120;
    int minfps=Patch(games,ServerNum,ServerResource);
    for(int i=0;i<games.size();++i)
        cout<<"NO."<<i<<": fps="<<games[i].fps<<" max_tolerate="<<games[i].max_tolerate<<endl;
    cout<<minfps;


    system("pause");
}
