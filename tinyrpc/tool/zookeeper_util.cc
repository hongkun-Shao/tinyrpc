#include <iostream>
#include <semaphore.h>
#include "tinyrpc/tool/log.h"
#include "zookeeper_util.h"
// #include "mprpcapplication.h"

namespace tinyrpc{

// 全局的watcher观察器   zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx){
    // 回调的消息类型是和会话相关的消息类型
    if (type == ZOO_SESSION_EVENT){  
        // zkclient和zkserver连接成功
		if (state == ZOO_CONNECTED_STATE){
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
		}
	}
}

ZkClient::ZkClient() : m_zhandle(nullptr) { }

ZkClient::~ZkClient(){
    if (m_zhandle != nullptr){
        zookeeper_close(m_zhandle); // 关闭句柄，释放资源  MySQL_Conn
    }
}

// 连接zkserver
void ZkClient::Start(){
	//默认地址，TODO:改成通过config来配置connstr
    std::string host = "127.0.0.1";
    std::string port = "2181";
    std::string connstr = host + ":" + port;
    
	/*
	zookeeper_mt：多线程版本
	zookeeper的API客户端程序提供了三个线程
	API调用线程 
	网络I/O线程  pthread_create  poll
	watcher回调线程 pthread_create
	*/
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle){
		ERRORLOG("zookeeper_init error!");
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem);//阻塞到这里，直到信号量不为0
	INFOLOG("zookeeper_init success!");
    
}

void ZkClient::Create(const char *path, const char *data, int datalen, int state){
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
	// 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
	flag = zoo_exists(m_zhandle, path, 0, nullptr);
	// 表示path的znode节点不存在
	if(ZNONODE == flag){
		// 创建指定path的znode节点了
		flag = zoo_create(m_zhandle, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
		if (flag == ZOK){
			INFOLOG("znode create success... path: %s", path);
		}else{
			ERRORLOG("znode create error... path: %s, return value: %d", path, flag);
			exit(EXIT_FAILURE);
		}
	}
}

// 根据指定的path，获取znode节点的值
std::string ZkClient::GetData(const char *path){
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
	if (flag != ZOK){
		ERRORLOG("get znode error... path: %s", path);
		return "";
	}else{
		return buffer;
	}
}

}