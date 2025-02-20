#include "chatserver.hpp"
#include "chatservice.hpp"
ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg) : _server(loop, listenAddr, nameArg), _loop(loop)
{ // 事件循环，IP+Port,服务器名字
  // 给服务器注册用户连接的创建和断开回调
  _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
  // 给服务器注册用户读写事件回调
  _server.setMessageCallback(std::bind(&ChatServer::OnMessage, this, _1, _2, _3));
  // 设置服务器端的线程数量
  _server.setThreadNum(4); // 一个IO线程，三个worker线程  muduo库会自己分配
}

// 启动
void ChatServer::start()
{
  _server.start();
}

// 连接监听
void ChatServer::onConnection(const TcpConnectionPtr &con)
{
  if (!con->connected())
  {
    ChatService::instance()->clientCloseException(con);
    con->shutdown();
  }
}

// 读写事件监听
void ChatServer::OnMessage(const TcpConnectionPtr &con, Buffer *buff, Timestamp time)
{
  string buf = buff->retrieveAllAsString();
  // 数据的反序列化
  json js = json::parse(buf);

  // 通过js["msgid"] 获取一个业务处理器handler
  // 目的就是解耦网络模块的代码和业务模块的代码
  auto msghandler = ChatService::instance()->GetHandler(js["msgid"].get<int>());

  // 回调消息绑定好的事件处理器
  msghandler(con, js, time);
}

