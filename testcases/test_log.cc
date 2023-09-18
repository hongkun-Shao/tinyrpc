//#include <pthread.h>
#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/config.h"


void * fun(void *){
    int i = 1;
    while (i--) {
        DEBUGLOG("debug this is thread in %s", "fun");
        INFOLOG("info this is thread in %s", "fun");
    }

  return NULL;
}

int main() {

    tinyrpc::Config::SetGlobalConfig("../conf/tinyrpc.xml");

    tinyrpc::Logger::InitGlobalLogger();


    pthread_t thread;
    pthread_create(&thread, NULL, &fun, NULL);

    int i = 1;
    while (i--) {
        DEBUGLOG("test debug log %s", "11");
        INFOLOG("test info log %s", "11");
    }

    pthread_join(thread, NULL);
    return 0;
}