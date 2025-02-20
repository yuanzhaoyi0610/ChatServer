# ChatServer
可以工作在nginxTcp负载均衡环境中的集群聊天服务器和客户端源码，基于muduo库实现 +redis +mysql

编译方式
cd build
rm -rf *
cmake ..
make

需要安装nginx服务启动  mysql redis 开发环境配置


/usr/local/nginx/conf/nginx.conf中

在even{} 和http{}之间 添加：
#nginx tcp loadbalance config
stream{
    upstream MyServer{
        server 192.168.1.198:6000 weight=1 max_fails=3 fail_timeout=30s;
        server 192.168.1.198:6002 weight=1 max_fails=3 fail_timeout=30s;
    }
    server{
        proxy_connect_timeout 1s;
        #proxy_timeout 3s;
        listen 8000;
        proxy_pass MyServer;
        tcp_nodelay on;
    }
}
