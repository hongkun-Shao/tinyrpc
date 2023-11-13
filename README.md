# tinyrpc
## 1.Tiny RPC Overview
Tinyrpc is a multithreaded asynchronous RPC framework developed based on C++11, designed to be efficient and concise while maintaining extremely high performance.

Tinyrpc is also based on the master-slave actor architecture, with epoll used for IO multiplexing at the bottom. The application layer is based on protobuf custom rpc communication protocol, and will also support simple HTTP protocol.

## 2.Run Environment
1. CentOS 8.2
2. g++(support C++11)
3. protobuf 3.19.4a
4. tinyxml
### 2.1 Build Environment
1. protobuf
    protobuf Recommended use of 3.19.4 and above:

    Installation process:
```
    wget  https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protobuf-cpp-3.19.4.tar.gz

    tar -xzvf protobuf-cpp-3.19.4.tar.gz

    cd protobuf-cpp-3.19.4

    ./configure -prefix=/usr

    make -j4 

    sudo make install
```
After the installation is completed, you can find that the header files will be located under/usr/include/google, and the library files will be located under/usr/lib (or/usr/lib64)
  

2. tinyxml
The project used a configuration module and used XML as the configuration file. Therefore, it is necessary to install libtinyxml to parse the XML file.

```
    wget https://udomain.dl.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.zip

    unzip tinyxml_2_6_2.zip

    To generate the libtinyxml.a static library, simply modify the makefile as follows:

    # 84 line
    OUTPUT := libtinyxml.a 

    # 194, 105 line
    ${OUTPUT}: ${OBJS}
        ${AR} $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}
    
    cd tinyxml
    make -j4

    # Copy library files to the system library file search path
    cp libtinyxml.a /usr/lib/

    # Copy header files to the system header file search path
    mkdir /usr/include/tinyxml
    cp *.h /usr/include/tinyxml
```

## 3.How to use tinyrpc

1. Clone the project locally, assuming the absolute path of the project is /root/tinyrpc
2. `cd /root/tinyrpc`
3. View the makefile file and change `LIBS += /usr/lib64/libprotobuf.a  /usr/lib/libtinyxml.a` to the address where you downloaded it yourself.
5. `make` to generate lib, bin, obj file
6. `make install` install tinyrpc.a tinyrpc/*.h to local adress。
7. create proto file，use generator to generate your rpc service。Reference Article:[模块讲解七：Generator(脚手架)模块](http://showmycodes.com/2023/11/13/模块讲解七：generator脚手架模块/)

## 4. Development Sequence
```
1.Environment install and Project development
1.1 Build Environment and Install library
1.2 Log Module development
1.3 Config Module development

2. EventLoop Module package
2.1 EventLoop Core Class Construction
2.2 FdEvent package & test
2.3 Timer
2.4 Master-slave Reactor 
2.5 EventLoop Module overall test

3. Tcp Module package
3.1 TcpBuffer
3.2 TcpConnection 
3.3 TcpServer 
3.4 TcpClient
3.5 Tcp Module test

4. RPC Protocol package
4.1 TinyPB Protocol encode
4.2 TinyPB Protocol decode
4.3 TinyPB Protocol Module test

5. RPC Module package 
5.1 RpcController and RcpClosure basic class
5.2 RpcDispatcher 
5.3 RpcChannel
5.4 RpcAsyncChannel
5.5 Rpc Module overall test

6. RPC Generator package
6.1 code generator 
6.2 build & test project

```
## 5.Code Explanation Address
personal blog：http://showmycodes.com/category/pesonal-demo/tinyrpc/

## 6. TODO
1. Register Module 
2. Coroutine

## 7.Thank to 
1. https://github.com/attackoncs/rpc (tiny basic synchronous RPC framework)
2. https://github.com/Gooddbird/tinyrpc (tiny multi threaded asynchronous RPC framework)
3. https://github.com/grpc/grpc (a modern, open source, high-performance remote procedure call (RPC) framework)
4. https://www.bilibili.com/video/BV1cg4y1j7Wr/?spm_id_from=333.788&vd_source=ad408864adbf0c5272deb4934fdf08dc (Video learning website)
