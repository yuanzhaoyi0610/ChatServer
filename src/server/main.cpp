#include "chatserver.hpp"
#include <signal.h>
#include <chatservice.hpp>
// 处理服务器ctrl+c结束后，重置user状态
void resetHadler(int)
{
  ChatService::instance()->reset();
  exit(0);
}

int main(int argc,char**argv)
{
  signal(SIGINT, resetHadler);
  EventLoop loop;
  
  char *ip = argv[1];
  uint16_t port = atoi(argv[2]); 
  InetAddress addr(ip, port);
  ChatServer server(&loop, addr, "Chatserver");
  server.start();
  loop.loop(); // 阻塞方式等待用户的操作
  return 0;
}