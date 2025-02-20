#include "friendmodel.hpp"
#include "db.hpp"
// 添加好友关系
bool FriendModel::insert(int userid, int friendid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values('%d','%d')", userid, friendid);
    MySQL mysql;
    if (mysql.connect())
        if (mysql.update(sql))
        {
            LOG_INFO << "friendmodel :insert success!";
            return true;
        }
        else
            LOG_INFO << "friendmodel :insert failed!";
    return false;
}
// 判断两人是否是好友
bool FriendModel::judgeFriend(int id, int fid)
{
    vector<User> vecuser = this->query(id);
    for (auto &user : vecuser)
    { // 遍历 vecuser 中的每个 User
        if (user.getId() == fid)
        {                // 检查 User 的 id 是否与 searchId 相等
            return true; // 找到匹配的 User
        }
    }
    return false;
}

// 返回用户好友列表
vector<User> FriendModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid = %d", userid);
    MySQL mysql;
    vector<User> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res)
        {
            // 把userid用户的所有离线消息放入vec中返回
            while (MYSQL_ROW row = mysql_fetch_row(res))
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            LOG_INFO << "FriendModel :query success!";
            return vec;
        }
        LOG_INFO << "FriendModel :query failed!";
    }
}