#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
#include <muduo/base/Logging.h>

using namespace std;
static string server = "192.168.1.198";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

class MySQL
{
public:
    // 初始化数据库
    MySQL();

    // 释放数据库连接资源
    ~MySQL();

    // 连接数据库
    bool connect();

    // 更新操作
    bool update(string sql);

    // 查询操作
    MYSQL_RES *query(string sql);

    //返回连接
    MYSQL *getcon();

private:
    MYSQL *_con;
};

#endif