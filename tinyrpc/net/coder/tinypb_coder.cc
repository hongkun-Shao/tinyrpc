#include <vector>
#include <string.h>
#include <arpa/inet.h>
#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/util.h"
#include "tinyrpc/net/coder/tinypb_coder.h"


namespace tinyrpc{

void TinyPBCoder::Encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) {
    for(auto &i : messages){
        std::shared_ptr<TinyPBProtocol> msg = std::dynamic_pointer_cast<TinyPBProtocol>(i);
        int len = 0;
        const char * buf = EncodeTinyPB(msg, len);
        if(buf != nullptr && len != 0){
            out_buffer->WriteToBuffer(buf, len);
        }
        if(buf){
            free((void*)buf);
            buf = nullptr;
        }
    }
}    

void TinyPBCoder::Decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) {
    while(1){
        //travers buffer, and find PB_START, decode get the position of end signal
        //check is PB_END
        std::vector<char> temp = buffer->m_buffer_;
        int start_index = buffer->get_read_index();
        int end_index = -1;

        int pkg_len = 0;
        bool parse_success = false;
        int i = 0;
        for(int i = start_index; i < buffer->get_write_index(); ++i){
            if(temp[i] == TinyPBProtocol::PB_START){
                //read 4 bytes(net char)
                if(i + 1 < buffer->get_write_index()){
                    pkg_len = GetInt32FromNetByte(&temp[i + 1]);
                    DEBUGLOG("get pkg len = %d", pkg_len);

                    //the index of end signal
                    int j = i + pkg_len - 1;
                    if(j >= buffer->get_write_index()){
                        continue;
                    }
                    if(temp[j] == TinyPBProtocol::PB_END){
                        start_index = i;
                        end_index = j;
                        parse_success = true;
                        break;
                    }
                }
            }
        }

        if(i >= buffer->get_write_index()){
            DEBUGLOG("decode end, read all buffer data");
            return;
        }

        if(parse_success){
            buffer->MoveReadIndex(end_index - start_index + 1);
            std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
            message->m_pkg_len_ = pkg_len;

            int req_id_len_index = start_index + sizeof(char) + sizeof(message->m_pkg_len_);
            if(req_id_len_index >= end_index){
                message->m_parse_success_ = false;
                ERRORLOG("parse error, req_id_len_index[%d] >= end_index[%d]", req_id_len_index, end_index);
                continue;
            }
            message->m_req_id_len_ = GetInt32FromNetByte(&temp[req_id_len_index]);
            DEBUGLOG("parse req id len = %d", message->m_req_id_len_);

            int req_id_index = req_id_len_index + sizeof(message->m_req_id_len_);

            char req_id[100] = {0};
            memcpy(&req_id[0], &temp[req_id_index], message->m_req_id_len_);
            message->m_req_id_ = std::string(req_id);
            DEBUGLOG("parse req id = %s", message->m_req_id_.c_str());
            
            int method_name_len_index = req_id_index + message->m_req_id_len_;
            if (method_name_len_index >= end_index){
                message->m_parse_success_ = false;
                ERRORLOG("parse error, method_name_len_index[%d] >= end_index[%d]", method_name_len_index, end_index);
                continue;
            }
            message->m_method_name_len_ = GetInt32FromNetByte(&temp[method_name_len_index]);

            int method_name_index = method_name_len_index + sizeof(message->m_method_name_len_);
            char method_name[512] = {0};
            memcpy(&method_name[0], &temp[method_name_index], message->m_method_name_len_);
            message->m_method_name_ = std::string(method_name);
            DEBUGLOG("parse method name = %s", message->m_method_name_.c_str());

            int err_code_index = method_name_index + message->m_method_name_len_;
            if (err_code_index >= end_index) {
                message->m_parse_success_ = false;
                ERRORLOG("parse error, err_code_index[%d] >= end_index[%d]", err_code_index, end_index);
                continue;
            }
            message->m_err_code_ = GetInt32FromNetByte(&temp[err_code_index]);


            int error_info_len_index = err_code_index + sizeof(message->m_err_code_);
            if (error_info_len_index >= end_index) {
                message->m_parse_success_ = false;
                ERRORLOG("parse error, error_info_len_index[%d] >= end_index[%d]", error_info_len_index, end_index);
                continue;
            }
            message->m_err_info_len_ = GetInt32FromNetByte(&temp[error_info_len_index]);

            int err_info_index = error_info_len_index + sizeof(message->m_err_info_len_);
            char error_info[512] = {0};
            memcpy(&error_info[0], &temp[err_info_index], message->m_err_info_len_);
            message->m_err_info_ = std::string(error_info);
            DEBUGLOG("parse error_info = %s", message->m_err_info_.c_str());

            int pb_data_len = message->m_pkg_len_ - message->m_method_name_len_ - message->m_req_id_len_ - message->m_err_info_len_ - 2 - 24;

            int pd_data_index = err_info_index + message->m_err_info_len_;
            message->m_pb_data_ = std::string(&temp[pd_data_index], pb_data_len);

            // 这里校验和去解析
            message->m_parse_success_ = true;

            out_messages.push_back(message);
        }
    }   
}

const char* TinyPBCoder::EncodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int& len) {
    if (message->m_req_id_.empty()) {
        message->m_req_id_ = "123456789";
    }
    DEBUGLOG("req id = %s", message->m_req_id_.c_str());
    int pkg_len = 2 + 24 + message->m_req_id_.length() + message->m_method_name_.length() + message->m_err_info_.length() + message->m_pb_data_.length();
    DEBUGLOG("pkg len = %d", pkg_len);

    char* buf = reinterpret_cast<char*>(malloc(pkg_len));
    char* temp = buf;

    *temp = TinyPBProtocol::PB_START;
    temp++;

    int32_t pkg_len_net = htonl(pkg_len);
    memcpy(temp, &pkg_len_net, sizeof(pkg_len_net));
    temp += sizeof(pkg_len_net);

    int req_id_len = message->m_req_id_.length();
    int32_t req_id_len_net = htonl(req_id_len);
    memcpy(temp, &req_id_len_net, sizeof(req_id_len_net));
    temp += sizeof(req_id_len_net);

    if (!message->m_req_id_.empty()) {
        memcpy(temp, &(message->m_req_id_[0]), req_id_len);
        temp += req_id_len;
    }

    int method_name_len = message->m_method_name_.length();
    int32_t method_name_len_net = htonl(method_name_len);
    memcpy(temp, &method_name_len_net, sizeof(method_name_len_net));
    temp += sizeof(method_name_len_net);

    if (!message->m_method_name_.empty()) {
        memcpy(temp, &(message->m_method_name_[0]), method_name_len);
        temp += method_name_len;
    }

    int32_t err_code_net = htonl(message->m_err_code_);
    memcpy(temp, &err_code_net, sizeof(err_code_net));
    temp += sizeof(err_code_net);

    int err_info_len = message->m_err_info_.length();
    int32_t err_info_len_net = htonl(err_info_len);
    memcpy(temp, &err_info_len_net, sizeof(err_info_len_net));
    temp += sizeof(err_info_len_net);

    if (!message->m_err_info_.empty()) {
        memcpy(temp, &(message->m_err_info_[0]), err_info_len);
        temp += err_info_len;
    }

    if (!message->m_pb_data_.empty()) {
        memcpy(temp, &(message->m_pb_data_[0]), message->m_pb_data_.length());
        temp += message->m_pb_data_.length();
    }

    int32_t check_sum_net = htonl(1);
    memcpy(temp, &check_sum_net, sizeof(check_sum_net));
    temp += sizeof(check_sum_net);

    *temp = TinyPBProtocol::PB_END;

    message->m_pkg_len_ = pkg_len;
    message->m_req_id_len_ = req_id_len;
    message->m_method_name_len_ = method_name_len;
    message->m_err_info_len_ = err_info_len;
    message->m_parse_success_ = true;
    len = pkg_len;

    DEBUGLOG("encode message[%s] success", message->m_req_id_.c_str());

    return buf;
}
}