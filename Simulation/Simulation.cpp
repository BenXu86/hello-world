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
    int fps; //��GPU��ռʱ��fps
    int max_tolerate;//��������fps����
    int resource;//���ռ����Դ������ֵ�ϵ�������GPU��������
    float slope;// GPU-fps����б�ʣ��ɵ�(1,0)��ƫ��shift = -slope
    int serverID;
    Game(){
        this->fps = random(40,120);
        this->max_tolerate = fps-30;//��ϣ��fpsԽ��ʱ����������fps��ʧҲ��Խ��
        this->resource = random(fps/2,fps);//��ϣ��max_tolerateԽСʱ��������Խǿ
        this->slope = fps;//�����Դ-fps ���ߵ�б��
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
    // ���ص������resource��Դʱ��fps
    int GetFPS(int resource){
        if(resource >= this->resource)
            return fps;
        return (int)(slope * resource/this->resource );
    }
};
// ����Щ��Ϸ��һ̨����ΪCapacity�ķ�����ʱ����ȡ��Сfps
// ����games�Ѿ�����fps����
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
        // ��Щ��Ϸ���ܷ���һ��
        if((games[i].fps - temp)>games[i].max_tolerate)
            return -1;
        if(temp<min)
            min = temp;
    }
    return min;
}

// ��ͳ̰���㷨����
int Patch(vector<Game>games,int ServerNum,int Capacity)
{

    // ���ŵ���Ϸ��Ŀ
    int count = ServerNum;
    int minfps = games[0].fps;
    vector<Game>server[ServerNum];
    vector<vector<Game>>servers;
    for(int i=0;i<ServerNum;++i)
        servers.push_back(server[i]);
    // ��ǰServerNum����Ϸ�����������
    for(int i=0;i<ServerNum&&i<games.size();++i)
        servers[i].push_back(games[i]);
    // �Ե�count��game���ŷ�����
    while(count < games.size())
    {
        // ������Ϸ����ÿ�������������õ���ÿ̨����������Сfps
        int fps[ServerNum];
        // ��ÿ������������ȥ����һ�»��fps
        for(int i=0;i<ServerNum;++i)
        {
            servers[i].push_back(games[count]);
            fps[i] = GetMinFPS(servers[i],Capacity);
            servers[i].pop_back();
        }
        // Ѱ��ʹfps�����Ǹ�server
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
// �Ե�������������̬�滮����games,ʹ����������
int Dynamic_Patch(vector<Game>games,int ServerResource)
{
    int minFps[games.size()][ServerResource];
    for(int i=0;i<games.size();++i)
        for(int j=0;j<ServerResource;++j)
            minFps[i][j] = 0;
    //ǰi����Ϸ���ռ�Ϊj
    for(int i=0;i<games.size();++i)
    {
        for(int j=0;j<ServerResource;++j)
        {
            //�ܿռ��޷����µ�i����Ϸ
            if(j<games[i].resource)
                minFps[i][j] = minFps[i-1][j];
            if(i == 0)//�Ե�һ����Ϸ��������ĵ�һ�У�ֱ�Ӹ�ֵ
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
    // �Ȱ���fps��������
    for(int i=0;i<games.size();++i)
        for(int j=i+1;j<games.size();++j)
            if(games[i].fps>games[j].fps)
                swap(games[i],games[j]);
    //cout<<games.size();
    int ServerNum = 7;
    // ÿ̨����������Դ��
    // ����Դ����ʱ����������Ϸ����Դ�������Է���
    int ServerResource = 120;
    int minfps=Patch(games,ServerNum,ServerResource);
    for(int i=0;i<games.size();++i)
        cout<<"NO."<<i<<": fps="<<games[i].fps<<" max_tolerate="<<games[i].max_tolerate<<endl;
    cout<<minfps;


    system("pause");
}
