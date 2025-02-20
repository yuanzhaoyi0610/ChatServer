#include "usermodel.hpp"
#include <db.hpp>
#include <iostream>
using namespace std;

bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name,password,state) values('%s','%s','%s')", user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            user.setId(mysql_insert_id(mysql.getcon()));
            LOG_INFO << "UserModel:insert success!";
            return true;
        }
    }
    LOG_INFO << "UserModel:insert failed!";
    return false;
}

User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select *from user where id= %d", id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res); // 释放资源
                LOG_INFO << "UserModel:query success!";
                return user;
            }
        }
    }
    LOG_INFO << "UserModel:query failed!";
    return User(); // 返回一个默认user  到时候通过验证id=-1 则查询失败了
}

bool UserModel::update(User user)
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state='%s' where id =%d ", user.getState().c_str(), user.getId());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            LOG_INFO << "UserModel:update success!";
            return true;
        }
    }
    LOG_INFO << "UserModel:update failed!";
    return false;
}

void UserModel::resetstate()
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state='offline' where state = 'online'");
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            LOG_INFO << "UserModel:resetstate success!";
        }
    }
    LOG_INFO << "UserModel:resetstate failed!";
}
