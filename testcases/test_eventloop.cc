#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/config.h"
#include "tinyrpc/net/fd_event.h"
#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/timer_event.h"

int main (){
    tinyrpc::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    tinyrpc::Logger::InitGlobalLogger();
    tinyrpc::EventLoop * eventloop = new tinyrpc::EventLoop();

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        ERRORLOG("listenfd = -1");
        exit(0);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_port = htons(1731);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr.sin_addr);

    int res = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if(res != 0){
        ERRORLOG("bind error");
        exit(0);
    }

    res = listen(listenfd, 100);
    if(res != 0){
        ERRORLOG("listen error");
        exit(0);
    }

    tinyrpc::FdEvent event(listenfd);
    event.Listen(tinyrpc::FdEvent::IN_EVENT, [listenfd](){
        sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, sizeof(peer_addr));
        int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);

        DEBUGLOG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
    });
    eventloop->AddEpollEvent(&event);

    int i = 0;
    tinyrpc::TimerEvent::s_ptr timer_event = std::make_shared<tinyrpc::TimerEvent>(
        1000, true, [&i](){
            INFOLOG("trigger timer event, count = %d", i ++);
        }
    );
    eventloop->AddTimerEvent(timer_event);
    eventloop->Loop();

    return 0;
}