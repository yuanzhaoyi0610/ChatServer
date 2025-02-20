#include "offlinemodel.hpp"

// 存储用户的离线消息
void OfflineModel::insert(int userid, string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values('%d','%s')", userid, msg.c_str());
    MySQL mysql;
    if (mysql.connect())
        if (mysql.update(sql))
        {
            LOG_INFO << "offlineModel :update success!";
        }
        else
            LOG_INFO << "offlineModel :update failed!";
}

// 删除用户的离线消息
void OfflineModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d", userid);
    MySQL mysql;
    if (mysql.connect())
        if (mysql.update(sql))
        {
            LOG_INFO << "offlineModel :remove success!";
        }
        else
            LOG_INFO << "offlineModel :remove failed!";
}

// 查询用户的离线消息
vector<string> OfflineModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userid);
    MySQL mysql;
    vector<string> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res)
        {
            // 把userid用户的所有离线消息放入vec中返回
            while (MYSQL_ROW row = mysql_fetch_row(res))
            {
                vec.push_back(row[0]); // 第一列的值就是row[0]，这里查询的message只有一列
                LOG_INFO << "offlineModel :query success!";
            }
            mysql_free_result(res);
            return vec;
        }
        LOG_INFO << "offlineModel :query failed!";
    }
}