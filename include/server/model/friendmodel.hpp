#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include<vector>
using namespace std;
#include<user.hpp>
//维护好友信息的操作接口方法
class FriendModel
{
private:
    /* data */
public:
    //添加好友关系
    bool insert(int userid,int friendid);

    //返回用户好友列表
    vector<User> query(int userid);

    //判断两人是否为好友
    bool judgeFriend(int id,int fid);
};

#endif