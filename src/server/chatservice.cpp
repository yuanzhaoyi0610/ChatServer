#include "chatservice.hpp"
#include "public.hpp"
#include "user.hpp"
#include <iostream>

using namespace std;

ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的回调操作
ChatService::ChatService()
{
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    // 群组业务管理相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    if (_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 登录业务
void ChatService::login(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int id = js["id"];
    string pwd = js["password"];
    // 使用id去返回user的信息
    User user = _usermodel.query(id);
    // 登录成功
    if (user.getId() == id and user.getPassword() == pwd)
    {

        if (user.getState() == "online")
        {
            // 用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2; // 非0表示有问题
            response["errmsg"] = "this account is using,input other!";
            con->send(response.dump());
        }
        else // 成功登录
        {
            // 更新用户状态信息 为在线
            user.setState("online");
            _usermodel.update(user);

            // 订阅channel
            _redis.subscribe(id);

            // 响应信息
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0; // 表示没有问题
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 智能指针，构造的时候上锁，析构的时候解锁
            {
                lock_guard<mutex> lock(_conMutex);
                // 记录用户连接信息
                _userconMap.insert({id, con}); // 会在多线程里面被调用，要考虑线程安全问题
            }

            // 查询该用户好友列表
            vector<User> uservec = _friendmodel.query(id);
            if (!uservec.empty())
            {
                // 间接转化一下信息
                vector<string> vec2;
                for (User &user : uservec)
                {
                    json js;
                    js["name"] = user.getName();
                    js["id"] = user.getId();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupmodel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            // 查询用户是否有离线消息
            vector<string> vec = _offlinemodel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 删除离线消息
                _offlinemodel.remove(id);
            }
            // 发送响应
            con->send(response.dump());
        }
    }
    else
    {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1; // 表示有问题
        response["errmsg"] = "id or password is invalid!";
        con->send(response.dump());
    }
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_conMutex);
        auto it = _userconMap.find(userid);
        if (it != _userconMap.end())
        {
            _userconMap.erase(it);
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _usermodel.update(user);
}

// 注册业务
void ChatService::reg(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPassword(pwd);
    if (_usermodel.insert(user))
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0; // 表示没有问题
        response["id"] = user.getId();
        con->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1; // 有问题
        con->send(response.dump());
    }
}

MsgHandler ChatService::GetHandler(int msgid)
{
    // 记录错误日志,msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    // 返回一个空的默认处理器，空操作
    if (it == _msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &con, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << "can not find handler!";
        };
    }
    else
        return _msgHandlerMap[msgid];
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &con)
{
    User user;
    {
        lock_guard<mutex> lock(_conMutex);
        for (auto it = _userconMap.begin(); it != _userconMap.end(); it++)
        {
            if (it->second == con)
            {
                // 从map表删除这个连接信息
                user.setId(it->first);
                _userconMap.erase(it);
                break;
            }
        }
    }
    // 注销订阅
    _redis.unsubscribe(user.getId());

    // 更新用户的状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        _usermodel.update(user);
    }
}

// 单对单聊天
void ChatService::oneChat(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    // 发送方id
    int id = js["id"].get<int>();
    // 如果是好友就转发
    if (_friendmodel.judgeFriend(id, toid))
    {
        {
            lock_guard<mutex> lock(_conMutex);
            auto it = _userconMap.find(toid);
            if (it != _userconMap.end())
            {
                // toid在线，转发消息   服务器主动推送消息给toid用户
                it->second->send(js.dump());
                return;
            }
            else // 如果在连接map中没有找到，则在数据库中查询是否在线，因为有可能在其他服务器中存在此连接
            {
                // 查询toid是否在线
                User user = _usermodel.query(toid);
                if (user.getState() == "online")
                {
                    _redis.publish(toid, js.dump());
                    return;
                }
                // toid不在线，存储离线消息
                _offlinemodel.insert(toid, js.dump());
            }
        }
    }

    else
    {
        // 否则就返回一个对方不是您好友
        json res_err;
        res_err["msgid"] = ONE_CHAT_ERR; // 表示对方不是您好友
        res_err["toid"] = toid;
        con->send(res_err.dump());
    }
}

// 服务器异常后重置user状态
void ChatService::reset()
{
    // 把所有online用户状态都设置为offline
    _usermodel.resetstate();
}

void ChatService::addFriend(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int userid = js["id"];
    int friendid = js["friendid"];

    // 存储好友信息
    _friendmodel.insert(userid, friendid);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int userid = js["id"];
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group gro(-1, name, desc);
    if (_groupmodel.createGroup(gro))
    {
        _groupmodel.addGroup(userid, gro.getId(), "creator");
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int userid = js["id"];
    int groupid = js["groupid"];
    _groupmodel.addGroup(userid, groupid, "normal");
}

// 群聊
void ChatService::groupChat(const TcpConnectionPtr &con, json &js, Timestamp time)
{
    int userid = js["id"];
    int groupid = js["groupid"];
    vector<int> useridvec = _groupmodel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_conMutex);
    for (int id : useridvec)
    {
        auto it = _userconMap.find(id);
        if (it != _userconMap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            // 查询toid是否在线
            User user = _usermodel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                // 存储离线群消息
                _offlinemodel.insert(id, js.dump());
            }
        }
    }
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_conMutex);
    auto it = _userconMap.find(userid);
    if (it != _userconMap.end())
    {
        it->second->send(msg);
        return;
    }
    // 有可能从消息队列拉消息的时候，对方客户下线了
    //  存储该用户的离线消息
    _offlinemodel.insert(userid, msg);
}