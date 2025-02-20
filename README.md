# ChatServer
可以工作在nginxTcp负载均衡环境中的集群聊天服务器和客户端源码，基于muduo库实现 +redis +mysql

编译方式
cd build
rm -rf *
cmake ..
make

需要安装nginx服务启动  mysql redis 开发环境配置
