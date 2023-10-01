#########################ROCKET#########
# makefile
# ikerli
# 2022-05-23
##################################

PATH_BIN = bin
PATH_LIB = lib
PATH_OBJ = obj

PATH_TINYRPC = tinyrpc
PATH_TOOL = $(PATH_TINYRPC)/common
PATH_NET = $(PATH_TINYRPC)/net
PATH_TCP = $(PATH_TINYRPC)/net/tcp

PATH_TESTCASES = testcases

# will install lib to /usr/lib/libsocket.a
PATH_INSTALL_LIB_ROOT = /usr/lib

# will install all header file to /usr/include/socket
PATH_INSTALL_INC_ROOT = /usr/include

PATH_INSTALL_INC_TOOL = $(PATH_INSTALL_INC_ROOT)/$(PATH_TOOL)
PATH_INSTALL_INC_NET = $(PATH_INSTALL_INC_ROOT)/$(PATH_NET)
PATH_INSTALL_INC_TCP = $(PATH_INSTALL_INC_ROOT)/$(PATH_TCP)


# PATH_PROTOBUF = /usr/include/google
# PATH_TINYXML = /usr/include/tinyxml

CXX := g++

CXXFLAGS += -g -O0 -std=c++11 -Wall -Wno-deprecated -Wno-unused-but-set-variable

CXXFLAGS += -I./ -I$(PATH_TINYRPC)	-I$(PATH_TOOL) -I$(PATH_NET) -I$(PATH_TCP)

LIBS += /usr/lib64/libprotobuf.a	/usr/lib/libtinyxml.a


TOOL_OBJ := $(patsubst $(PATH_TOOL)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_TOOL)/*.cc))
NET_OBJ := $(patsubst $(PATH_NET)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_NET)/*.cc))
TCP_OBJ := $(patsubst $(PATH_TCP)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_TCP)/*.cc))

ALL_TESTS : $(PATH_BIN)/test_log $(PATH_BIN)/test_eventloop $(PATH_BIN)/test_tcp $(PATH_BIN)/test_client

TEST_CASE_OUT := $(PATH_BIN)/test_log $(PATH_BIN)/test_eventloop $(PATH_BIN)/test_tcp $(PATH_BIN)/test_client

LIB_OUT := $(PATH_LIB)/libtinyrpc.a

$(PATH_BIN)/test_log: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_log.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

$(PATH_BIN)/test_eventloop: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_eventloop.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

$(PATH_BIN)/test_tcp: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_tcp.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

$(PATH_BIN)/test_client: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_client.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread


$(LIB_OUT): $(TOOL_OBJ) $(NET_OBJ) $(TCP_OBJ)
	cd $(PATH_OBJ) && ar rcv libtinyrpc.a *.o && cp libtinyrpc.a ../lib/

$(PATH_OBJ)/%.o : $(PATH_TOOL)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(PATH_OBJ)/%.o : $(PATH_NET)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_OBJ)/%.o : $(PATH_TCP)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# print something test
# like this: make PRINT-PATH_BIN, and then will print variable PATH_BIN
PRINT-% : ; @echo $* = $($*)


# to clean 
clean :
	rm -f $(TOOL_OBJ) $(NET_OBJ) $(TESTCASES) $(TEST_CASE_OUT) $(PATH_LIB)/libtinyrpc.a $(PATH_OBJ)/libtinyrpc.a

# install
install:
	mkdir -p $(PATH_INSTALL_INC_TOOL) $(PATH_INSTALL_INC_NET) \
		&& cp $(PATH_TOOL)/*.h $(PATH_INSTALL_INC_TOOL) \
		&& cp $(PATH_NET)/*.h $(PATH_INSTALL_INC_NET) \
		&& cp $(PATH_TCP)/*.h $(PATH_INSTALL_INC_TCP) \
		&& cp $(LIB_OUT) $(PATH_INSTALL_LIB_ROOT)/


# uninstall
uninstall:
	rm -rf $(PATH_INSTALL_INC_ROOT)/TINYRPC && rm -f $(PATH_INSTALL_LIB_ROOT)/libtinyrpc.a