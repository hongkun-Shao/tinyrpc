# tinyrpc

## 1.Tiny RPC 概述

tinyrpc 是基于 C++11 开发的一款多线程的异步 RPC 框架，它旨在高效、简洁的同时，又保持至极高的性能。

tinyrpc 同样是基于主从 Reactor 架构，底层采用 epoll 实现 IO 多路复用。应用层则基于 protobuf 自定义 rpc 通信协议，同时也将支持简单的 HTTP 协议。

## 2.运行环境

1. CentOS 8.2
2. g++(support C++11)
3. protobuf 3.19.4a
4. tinyxml

### 2.1环境搭建

1. protobuf
   protobuf 推荐使用 3.19.4 及其以上：

   安装过程：

```
    wget  https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protobuf-cpp-3.19.4.tar.gz

    tar -xzvf protobuf-cpp-3.19.4.tar.gz

    cd protobuf-cpp-3.19.4

    ./configure -prefix=/usr

    make -j4 

    sudo make install
```

安装完成后，你可以找到头文件将位于 /usr/include/google 下，库文件将位于/usr/lib(或者/usr/lib64) 下。   

2. tinyxml
   项目中使用到了配置模块，采用了 xml 作为配置文件。因此需要安装 libtinyxml 解析 xml 文件。

```
    wget https://udomain.dl.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.zip

    unzip tinyxml_2_6_2.zip

    要生成 libtinyxml.a 静态库，需要简单修改 makefile 如下:

    # 84 行修改为如下
    OUTPUT := libtinyxml.a 

    # 194, 105 行修改如下
    ${OUTPUT}: ${OBJS}
        ${AR} $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}
    
    cd tinyxml
    make -j4

    # copy 库文件到系统库文件搜索路径下
    cp libtinyxml.a /usr/lib/

    # copy 头文件到系统头文件搜索路径下 
    mkdir /usr/include/tinyxml
    cp *.h /usr/include/tinyxml
```

## 3.使用tinyrpc

1. 将项目clone到本地， 假设项目绝对路径为/root/tinyrpc
2. `cd /root/tinyrpc`
3. 查看makefile文件, 将makefile文件中的 `LIBS += /usr/lib64/libprotobuf.a  /usr/lib/libtinyxml.a`
   改成自己下载的地址。
5. `make` 生成lib, bin, obj文件
6. `make install` 将tinyrpc.a tinyrpc/*.h安装到本地环境中。
7. 创建proto文件， 使用generator生成自己的rpc服务。参考文章: [模块讲解七：Generator(脚手架)模块](http://showmycodes.com/2023/11/13/模块讲解七：generator脚手架模块/)


## 4. 开发顺序

````
# tinyrpc
## 1.Tiny RPC 概述
tinyrpc 是基于 C++11 开发的一款多线程的异步 RPC 框架，它旨在高效、简洁的同时，又保持至极高的性能。

tinyrpc 同样是基于主从 Reactor 架构，底层采用 epoll 实现 IO 多路复用。应用层则基于 protobuf 自定义 rpc 通信协议，同时也将支持简单的 HTTP 协议。

## 2.运行环境
1. CentOS 8.2
2. g++(support C++11)
3. protobuf 3.19.4a
4. tinyxml
### 2.1环境搭建
1. protobuf
    protobuf 推荐使用 3.19.4 及其以上：

    安装过程：
```
    wget  https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protobuf-cpp-3.19.4.tar.gz

    tar -xzvf protobuf-cpp-3.19.4.tar.gz

    cd protobuf-cpp-3.19.4

    ./configure -prefix=/usr

    make -j4 

    sudo make install
```

安装完成后，你可以找到头文件将位于 /usr/include/google 下，库文件将位于/usr/lib(或者/usr/lib64) 下。   

2. tinyxml
项目中使用到了配置模块，采用了 xml 作为配置文件。因此需要安装 libtinyxml 解析 xml 文件。
```
    wget https://udomain.dl.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.zip

    unzip tinyxml_2_6_2.zip

    要生成 libtinyxml.a 静态库，需要简单修改 makefile 如下:

    # 84 行修改为如下
    OUTPUT := libtinyxml.a 

    # 194, 105 行修改如下
    ${OUTPUT}: ${OBJS}
        ${AR} $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}
    
    cd tinyxml
    make -j4

    # copy 库文件到系统库文件搜索路径下
    cp libtinyxml.a /usr/lib/

    # copy 头文件到系统头文件搜索路径下 
    mkdir /usr/include/tinyxml
    cp *.h /usr/include/tinyxml
```
## 3.使用tinyrpc
1. 将项目clone到本地， 假设项目绝对路径为/root/tinyrpc
2. `cd /root/tinyrpc`
3. 查看makefile文件, 将makefile文件中的
`LIBS += /usr/lib64/libprotobuf.a /usr/lib/libtinyxml.a`
改成自己下载的地址。
4. `make`


## 4. 开发顺序
```
1. 环境安装与项目开发
1.1 环境搭建和依赖库安装
1.2 日志模块开发
1.3 配置模块开发

2. EventLoop 模块封装
2.1 EventLoop 核心类构建
2.2 FdEvent 封装以及测试
2.3 定时器 Timer
2.4 主从 Reactor 
2.5 EventLoop 模块整体测试

3. Tcp 模块封装
3.1 TcpBuffer
3.2 TcpConnection 
3.3 TcpServer 
3.4 TcpClient
3.5 Tcp 模块测试

4. RPC 协议封装
4.1 TinyPB 协议编码
4.2 TinyPB 协议解码
4.3 编解码模块测试

5. RPC 通信模块封装
5.1 RpcController 以及 RcpClosure 等基础类
5.2 RpcDispatcher 分发器
5.3 RpcChannel
5.4 RpcAsyncChannel
5.5 Rpc 模块集成测试

6. RPC 脚手架封装
6.1 代码生成器开发
6.2 项目的构建与测试

```
## 5.代码讲解地址
个人博客：http://showmycodes.com/category/pesonal-demo/tinyrpc/

## 6.待优化点
1. 服务注册中心
2. 协程优化

## 7.致谢
1. https://github.com/attackoncs/rpc (轻型同步RPC框架)
2. https://github.com/Gooddbird/tinyrpc (轻型多线程异步RPC框架)
3. https://github.com/grpc/grpc (一个现代的、开源的、高性能的远程过程调用（RPC）框架)
4. https://www.bilibili.com/video/BV1cg4y1j7Wr/?spm_id_from=333.788&vd_source=ad408864adbf0c5272deb4934fdf08dc (视频学习地址)
````

## 5.代码讲解地址

个人博客：http://showmycodes.com/category/pesonal-demo/tinyrpc/

## 6.待优化点

1. 服务注册中心
2. 协程优化

## 7.致谢

1. https://github.com/attackoncs/rpc (轻型同步RPC框架)
2. https://github.com/Gooddbird/tinyrpc (轻型多线程异步RPC框架)
3. https://github.com/grpc/grpc (一个现代的、开源的、高性能的远程过程调用（RPC）框架)
4. https://www.bilibili.com/video/BV1cg4y1j7Wr/?spm_id_from=333.788&vd_source=ad408864adbf0c5272deb4934fdf08dc (视频学习地址)