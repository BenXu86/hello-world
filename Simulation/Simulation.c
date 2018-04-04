#include<iostream>
#include<string>
using namespace std;
class Game
{
    string name;
    int fps;//单独运行在一个服务器时的fps
    int ability;//抢占fps的能力
    int max_tolerate; //能容忍的最大的fps降低

};
