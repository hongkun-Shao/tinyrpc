#pragma once
#ifndef TINYRPC_NET_CODER_TINYPB_PROTOCOL_H
#define TINYRPC_NET_CODER_TINYRB_PROTOCOL_H

#include <string>
#include "tinyrpc/net/coder/abstract_protocol.h"

namespace tinyrpc{

class TinyPBProtocol : public AbstractProtocol{
public:
    TinyPBProtocol(){}
    ~TinyPBProtocol() {}

public:
    static char PB_START;
    static char PB_END;

public:

    int32_t m_pkg_len_ {0};
    int32_t m_msg_id_len_ {0};

    int32_t m_method_name_len_ {0};
    std::string m_method_name_;

    int32_t m_err_code_ {0};
    int32_t m_err_info_len_ {0};
    std::string m_err_info_;
    
    std::string m_pb_data_;
    int32_t m_check_sum_ {0};

    bool m_parse_success_ {false};
    
};

}


#endif