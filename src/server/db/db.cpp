#include "db.hpp"

MySQL::MySQL()
{
    _con = mysql_init(nullptr);
}
// 释放数据库连接资源
MySQL::~MySQL()
{
    if (_con != nullptr)
        mysql_close(_con);
}
// 连接数据库
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_con, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p!=nullptr)
    {
        mysql_query(_con, "set names gbk"); // 避免中文乱码
        LOG_INFO<<"connect MYSQL success!";
    }
    else{
        LOG_INFO<<"connect MYSQL failed!";
    }
    return p;
        
}
// 更新操作
bool MySQL::update(string sql)
{
    if (mysql_query(_con, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败!";
    }
    return true;
}
// 查询操作
MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(_con, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败!";
        return nullptr;
    }
    return mysql_use_result(_con);
}

MYSQL *MySQL::getcon(){
    return this->_con;
}