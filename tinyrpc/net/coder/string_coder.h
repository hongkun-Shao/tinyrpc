#ifndef TINYRPC_NET_CODER_STRING_CODER_H
#define TINYRPC_NET_CODER_STRING_CODER_H

#include "tinyrpc/net/coder/abstract_coder.h"
#include "tinyrpc/net/coder/abstract_protocol.h"

namespace tinyrpc{

class StringProtocol : public AbstractProtocol{
public:
    std::string info;
};


class StringCoder : public AbstractCoder{

    //transfer message object to byte stream, write into buffer
    virtual void Encode(std::vector<AbstractProtocol::s_ptr> & message, TcpBuffer::s_ptr out_buffer){
        for(size_t i = 0; i < message.size(); ++ i){
            std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(message[i]);
            out_buffer->WriteToBuffer(msg->info.c_str(), msg->info.length());
        }
    }

    //transfer the bytes stream in buffer to message onject
    virtual void Decode(std::vector<AbstractProtocol::s_ptr> & out_message, TcpBuffer::s_ptr buffer){
        std::vector<char> res;
        buffer->ReadFromBuffer(res, buffer->GetReadSize());
        std::string info;
        for(size_t i = 0; i < res.size(); ++ i){
            info += res[i];
        }

        std::shared_ptr<StringProtocol> msg = std::make_shared<StringProtocol>();
        msg->info = info;
        msg->m_msg_id_ = "123456";
        out_message.push_back(msg);
    }
};

}

#endif