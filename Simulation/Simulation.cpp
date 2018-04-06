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
class Server
{
public:
    int Capacity;//服务器资源数
    int rest;//服务器剩余空闲资源
    vector<Game>games;//服务器上跑的游戏
    int minfps;//当前服务器运行的最小fps
    int index;//最小fps对应的游戏在games里的索引
    int total;//目前所有游戏的占用资源
    int Cal_minfps()
    {
        int r = 0;
        int minfps0 = 200;

        for(int i=0;i<games.size();++i)
        {
            r += games[i].resource;
            if(minfps0>games[i].fps)
                minfps0 = games[i].fps;
        }
        total = r;
        if(r<=Capacity)
        {
            this->rest = Capacity - r;
            this->minfps = minfps0;
            return minfps0;
        }
        else
        {
            this->rest = 0;
            minfps0 = games[0].GetFPS(games[0].resource * 1.0 / r * this->Capacity);
            for(int i=0;i<games.size();++i)
            {
                int t = games[i].GetFPS(games[i].resource * 1.0/ r * this->Capacity);
                if(t<minfps0)
                    minfps0 = t;
            }
            this->minfps = minfps0;
            return minfps0;
        }
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
int Patch(vector<Game>  games,int ServerNum,int Capacity)
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
    // 为第count个game安排服务器
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
// Dynamic optimal patch single
int Dynamic_PS(vector<Game>& games,Server & server,int ServerID)
{
    int minFps[games.size()][server.Capacity+1];
    // 记录最优解的路径
    int x[games.size()][server.Capacity+1];
    int y[games.size()][server.Capacity+1];
    // 记录第i个游戏是否被分配
    bool has[games.size()][server.Capacity+1];
    for(int i=0;i<games.size();++i)
        for(int j=0;j<=server.Capacity;++j)
            {
                minFps[i][j] = 1000;
                has[i][j] = true;
            }
    //前i个游戏，空间为j
    for(int i=0;i<games.size();++i)
    {
        for(int j=0;j<=server.Capacity;++j)
        {
            if(i == 0)//对第一款游戏，即矩阵的第一行，直接赋值
            {
                if(j>=games[0].resource)
                {
                    minFps[i][j] = games[i].fps;
                    has[i][j] = true;
                }
                else
                    has[i][j] =false;
                if(j != 0)
                {
                    x[i][j] = 0;
                    y[i][j] = 0;
                }
                else
                {
                    x[0][0] = -1;
                    y[0][0] = -1;
                }
                continue;
            }
            if(j<games[i].resource)
            {
                x[i][j] = i-1;
                y[i][j] = j;
                minFps[i][j] = minFps[i-1][j];
                has[i][j] = false;
                continue;
            }
            int put;

            put = min(minFps[i-1][j-games[i].resource],games[i].fps);
            x[i][j] = i-1;
            y[i][j] = j-games[i].resource;

            //如果放进去更好
            if(put >=minFps[i-1][j] || minFps[i-1][j] == 1000)
            {
                minFps[i][j] = put;
                has[i][j] = true;
            }
            else
            {
                minFps[i][j] = minFps[i-1][j];
                has[i][j] = false;
                x[i][j] = i-1;
                y[i][j] = j;
            }
        }
    }

    //下一步，将ServerID分配给各个被选中的游戏
    int i = games.size()-1,j = server.Capacity;

    while(!(i == -1 && j == -1))
    {
        int a = i, b = j;
        if(has[i][j] == true )
        {
            //cout<<"put!";
            games[i].serverID = ServerID;
            server.games.push_back(games[i]);
            //cout<<server.games[server.games.size()-1].fps<<endl;
        }
        i = x[a][b];
        j = y[a][b];
        //cout<<i<<" "<<j<<endl;
    }
    for(int i=0;i<server.games.size();++i)
        if(server.games[i].fps == server.minfps)
        {
            server.index = i;
            break;
        }
    return server.minfps;
}
// dynamic optimal patch total
int Dynamic_PT(vector<Game> &games,Server * servers,int ServerNum)
{
    for(int i = 0;i<ServerNum;++i)
    {
        if(games.size()!=0)
            Dynamic_PS(games,servers[i],i);
        else
            break;
        for(int i=0;i<games.size();)
            if(games[i].serverID != -1)
                games.erase(games.begin()+i);
            else
                ++i;
    }
    //cout<<"the rest games size is "<<games.size()<<endl;
    for(int i=0;i<ServerNum;++i)
        servers[i].Cal_minfps();
    while(games.size()!=0)
    {
        //之后用贪心算法分配,将games按照fps/resource降序排序
        for(int i=0;i<games.size();++i)
            for(int j= i+1;j<games.size();++j)
            {
                float g1 = games[i].fps/games[i].resource;
                float g2 = games[j].fps/games[j].resource;
                if(g1<g2)swap(games[i],games[j]);
            }
        //将服务器按照minfps*rest升序排序
        for(int i=0;i<ServerNum;++i)
            for(int j=i+1;j<ServerNum;++j)
            {
                //当游戏数量少时，突出rest的重要性
                //当游戏数目多时，突出fps的重要性
                double f1 = (servers[i].minfps);//+0.01*(servers[i].rest + 1)-(servers[i].total);
                double f2 = (servers[j].minfps);//+0.01*(servers[j].rest + 1)-(servers[j].total) ;
                if(f2<f1)
                    swap(servers[i],servers[j]);
            }
        servers[ServerNum-1].games.push_back(games[games.size()-1]);
        //cout<<servers[ServerNum-1].games[servers[ServerNum-1].games.size()-1].fps<<endl;
        servers[ServerNum-1].Cal_minfps();
        games.pop_back();
    }
    //记录最小fps的服务器
    int index = 0;
    int minst = servers[0].minfps;
    //cout<<minst<<endl;
    for(int i=0;i<ServerNum;++i)
    {
        if(minst> servers[i].minfps)
        {
            minst=servers[i].minfps;
            index = i;
        }
    }
    //cout<<"index:"<<index<<endl;
    return minst;
}

int main()
{
    srand(unsigned(time(0)));
    int count = 0;
    int equal = 0;
    int testNum=1000;
    int ServerNum = 7;
    int ServerResource = 120;
    int can_run2 = 0;
    int can_run1 = 0;
    int is_clear = 0;
    for(int tt=0;tt<testNum;++tt)
    {
        vector<Game>games;
        for(int i=0;i<15;++i)
            games.push_back(new Game());
        // 先按照fps升序排序
        for(int i=0;i<games.size();++i)
            for(int j=i+1;j<games.size();++j)
                if(games[i].fps>games[j].fps)
                    swap(games[i],games[j]);
        //cout<<games.size();

        // 每台服务器的资源量
        // 当资源不足时，将根据游戏的资源请求线性分配

        int minfps=Patch(games,ServerNum,ServerResource);
    //    for(int i=0;i<games.size();++i)
    //        cout<<"NO."<<i<<": fps="<<games[i].fps<<" max_tolerate="<<games[i].max_tolerate<<endl;
        //cout<<minfps<<endl;

        Server servers[ServerNum];
        for(int i=0;i<ServerNum;++i)
            servers[i].Capacity = ServerResource;
        int minfps2 = Dynamic_PT(games,servers,ServerNum);
        if(games.size()==0)is_clear++;
        if(minfps2 > minfps)count++;
        if(minfps2 == minfps)equal++;
        if(minfps2 >= 30)can_run2++;
        if(minfps >= 30)can_run1++;
    }

    cout<<count<<endl;
    cout<<equal<<endl;
    cout<<testNum-count-equal<<endl;
    cout<<can_run2<<endl;
    cout<<can_run1<<endl;
    cout<<endl<<is_clear<<endl;

    //cout<<"when using dynamic optimization,the min fps is:"<<minfps<<endl;
    //system("pause");
//    for(int i=0;i<ServerNum;++i)
//    {
//        int re_count = 0;
//        cout<<"the "<<i<<"st server loads games:"<<endl;
//        for(int j=0;j<servers[i].games.size();++j)
//        {
//            cout<<servers[i].games[j].fps<<"\t"<<servers[i].games[j].resource<<endl;
//            re_count += servers[i].games[j].resource;
//        }
//        cout<<re_count;
//        cout<<endl;
//    }
    system("pause");
}


/*
注：当动态规划算法可以放下所有游戏时（即服务器资源足够所有游戏完美跑时），
动态规划算法比贪心更优。现在的问题是，如何找到一种合理的在动态规划算法结束后更好地分配的方法
*/
