#ifndef TINYRPC_NET_THREAD_POOL_H
#define TINYRPC_NET_THREAD_POOL_H

#include <vector>
#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/io_thread.h"

namespace tinyrpc{

class IOThreadPool{
public:
    IOThreadPool(int num);
    ~IOThreadPool();

    void Start();

    void Join();

    IOThread * get_iothread();

private:
    int m_thread_num_ {0};
    std::vector<IOThread *> m_io_threads_pool_;

    int m_index_ {0}; //Round-Robin
};

}

#endif