#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include<functional>
#include<iostream>
#include<string>
#include "json.hpp"
using namespace muduo;
using namespace muduo::net;
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
class ChatServer
{
public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg);
    // 启动服务
    void start();

private:
    TcpServer _server;
    EventLoop *_loop;
    
    //连接事件回调函数
    void onConnection(const TcpConnectionPtr &con);
    //信息事件回调函数
    void OnMessage(const TcpConnectionPtr &con, Buffer *buff, Timestamp time);

    
};

#endif