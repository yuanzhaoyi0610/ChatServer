// 业务模块
#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Logging.h>
#include "json.hpp"
#include <string>
#include <mutex>
#include "usermodel.hpp"
#include "offlinemodel.hpp"
#include "friendmodel.hpp"
#include"groupmodel.hpp"
#include"redis.hpp"
using json = nlohmann::json;
using namespace std;
using namespace muduo::net;
using namespace muduo;
// 处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &con, json &js, Timestamp time)>;

// 单例模式 聊天服务器业务类
class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService *instance();
    // 登录和注册业务
    void login(const TcpConnectionPtr &con, json &js, Timestamp time);
    void reg(const TcpConnectionPtr &con, json &js, Timestamp time);
    // 获取消息对应的 处理器
    MsgHandler GetHandler(int msgid);

    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &con);

    // 一对一聊天服务
    void oneChat(const TcpConnectionPtr &con, json &js, Timestamp time);

    // 添加好友
    void addFriend(const TcpConnectionPtr &con, json &js, Timestamp time);

    //创建群组业务
    void createGroup(const TcpConnectionPtr &con, json &js, Timestamp time);

    //加入群组业务
    void addGroup(const TcpConnectionPtr &con, json &js, Timestamp time);

    //群聊天业务
    void groupChat(const TcpConnectionPtr &con, json &js, Timestamp time);

    //注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleRedisSubscribeMessage(int userid, string msg);

    // 服务器异常后退出
    void reset();

private:
    ChatService();
    // 存储消息id和其对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    // 数据操作类对象
    UserModel _usermodel;
    OfflineModel _offlinemodel;
    FriendModel _friendmodel;
    GroupModel _groupmodel;
    

    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userconMap;

    // 定义互斥锁，线程安全
    mutex _conMutex;

    Redis _redis;

     
};

#endif