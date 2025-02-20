#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"
// 对User表的操作类
class UserModel
{

public:
    // user的插入
    bool insert(User &user);

    //根据id查询用户信息
    User query(int id);

    //更新用户的状态信息
    bool update(User user);

    //重置用户的状态信息
    void resetstate();
};

#endif