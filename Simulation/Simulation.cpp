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
    Game(){
        this->fps = random(40,120);
        this->max_tolerate = fps-30;//我希望fps越大时，最大的容忍fps丢失也能越大
        this->resource = random(fps/2,fps);//我希望max_tolerate越小时抢夺能力越强
        this->slope = fps;//获得资源-fps 曲线的斜率
    }
    Game(Game *g){
        this->fps = g->fps;
        this->resource = g->resource;
        this->max_tolerate = g->max_tolerate;
        this->slope = g->slope;
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

int Patch(vector<Game>games,int ServerNum,int Capacity)
{
    // 先按照fps升序排序
    for(int i=0;i<games.size();++i)
        for(int j=i+1;j<games.size();++j)
            if(games[i].fps>games[j].fps)
                swap(games[i],games[j]);
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



int main()
{
    srand(unsigned(time(0)));
    vector<Game>games;
    for(int i=0;i<20;++i)
        games.push_back(new Game());
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
