#include <string>
#include <memory>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/config.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/net/coder/string_coder.h"
#include "tinyrpc/net/coder/tinypb_coder.h"
#include "tinyrpc/net/coder/abstract_protocol.h"

// void test_connect(){
//     // 调用 conenct 连接 server
//     // wirte 一个字符串
//     // 等待 read 返回结果

//     int fd = socket(AF_INET, SOCK_STREAM, 0);
//     if(fd < 0){
//         ERRORLOG("invaild fd %d", fd);
//         exit(0);
//     }
//     sockaddr_in server_addr;
//     memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(1731);
//     inet_aton("127.0.0.1", &server_addr.sin_addr);

//     int res = connect(fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr));

//     DEBUGLOG("connect success");

//     std::string msg = "hello, tinyrpc";
//     res = write(fd, msg.c_str(), msg.length());

//     DEBUGLOG("success write %d bytes, [%s]", res, msg.c_str());

//     char buf[100];
//     res = read(fd, buf, 100);
//     DEBUGLOG("success read %d bytes, [%s]", res, std::string(buf).c_str());
// }

void test_tcp_client(){
    tinyrpc::IPNetAddr::s_ptr addr = std::make_shared<tinyrpc::IPNetAddr>("127.0.0.1", 1731);
    tinyrpc::TcpClient client(addr);
    client.Connect([addr, &client](){
        DEBUGLOG("connect to [%s] success", addr->ToString().c_str());
        std::shared_ptr<tinyrpc::TinyPBProtocol> message = std::make_shared<tinyrpc::TinyPBProtocol>();
        message->m_msg_id_ = "123456";
        message->m_pb_data_ = "test pb data";
        client.WriteMessage(message, [](tinyrpc::AbstractProtocol::s_ptr msg_ptr){
            DEBUGLOG("send message success");
        });
        client.ReadMessage("123456", [](tinyrpc::AbstractProtocol::s_ptr msg_ptr){
            std::shared_ptr<tinyrpc::TinyPBProtocol> message = std::dynamic_pointer_cast<tinyrpc::TinyPBProtocol>(msg_ptr);
            DEBUGLOG("msg_id[%s], get response %s", message->m_msg_id_.c_str(), message->m_pb_data_.c_str());
        });
    });
}

int main (){
    tinyrpc::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    tinyrpc::Logger::InitGlobalLogger();
    //text_connect();
    test_tcp_client();
    return 0;
}