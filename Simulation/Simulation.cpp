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
class Server
{
public:
    int Capacity;//��������Դ��
    int rest;//������ʣ�������Դ
    vector<Game>games;//���������ܵ���Ϸ
    int minfps;//��ǰ���������е���Сfps
    int index;//��Сfps��Ӧ����Ϸ��games�������
    int total;//Ŀǰ������Ϸ��ռ����Դ
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
int Patch(vector<Game>  games,int ServerNum,int Capacity)
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
    // Ϊ��count��game���ŷ�����
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
// Dynamic optimal patch single
int Dynamic_PS(vector<Game>& games,Server & server,int ServerID)
{
    int minFps[games.size()][server.Capacity+1];
    // ��¼���Ž��·��
    int x[games.size()][server.Capacity+1];
    int y[games.size()][server.Capacity+1];
    // ��¼��i����Ϸ�Ƿ񱻷���
    bool has[games.size()][server.Capacity+1];
    for(int i=0;i<games.size();++i)
        for(int j=0;j<=server.Capacity;++j)
            {
                minFps[i][j] = 1000;
                has[i][j] = true;
            }
    //ǰi����Ϸ���ռ�Ϊj
    for(int i=0;i<games.size();++i)
    {
        for(int j=0;j<=server.Capacity;++j)
        {
            if(i == 0)//�Ե�һ����Ϸ��������ĵ�һ�У�ֱ�Ӹ�ֵ
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

            //����Ž�ȥ����
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

    //��һ������ServerID�����������ѡ�е���Ϸ
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
        //֮����̰���㷨����,��games����fps/resource��������
        for(int i=0;i<games.size();++i)
            for(int j= i+1;j<games.size();++j)
            {
                float g1 = games[i].fps/games[i].resource;
                float g2 = games[j].fps/games[j].resource;
                if(g1<g2)swap(games[i],games[j]);
            }
        //������������minfps*rest��������
        for(int i=0;i<ServerNum;++i)
            for(int j=i+1;j<ServerNum;++j)
            {
                //����Ϸ������ʱ��ͻ��rest����Ҫ��
                //����Ϸ��Ŀ��ʱ��ͻ��fps����Ҫ��
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
    //��¼��Сfps�ķ�����
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
        // �Ȱ���fps��������
        for(int i=0;i<games.size();++i)
            for(int j=i+1;j<games.size();++j)
                if(games[i].fps>games[j].fps)
                    swap(games[i],games[j]);
        //cout<<games.size();

        // ÿ̨����������Դ��
        // ����Դ����ʱ����������Ϸ����Դ�������Է���

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
ע������̬�滮�㷨���Է���������Ϸʱ������������Դ�㹻������Ϸ������ʱ����
��̬�滮�㷨��̰�ĸ��š����ڵ������ǣ�����ҵ�һ�ֺ�����ڶ�̬�滮�㷨��������õط���ķ���
*/
