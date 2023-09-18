############################################################
# makefile
# showcode
# 2023-09-18
############################################################

PATH_BIN = bin
PATH_LIB = lib
PATH_OBJ = obj

PATH_TINYRPC = tinyrpc
PATH_TOOL = $(PATH_TINYRPC)/tool


PATH_TESTCASES = testcases

# will install lib to /usr/lib/libtinyrpc.a
PATH_INSTALL_LIB_ROOT = /usr/lib

## install all header file to /usr/include/tinyrpc
PATH_INSTALL_INC_ROOT = /usr/include

PATH_INSTALL_INC_TOOL = $(PATH_INSTALL_INC_ROOT)/$(PATH_TOOL)


CXX := g++

CXXFLAGS += -g -O0 -std=c++11 -Wall -Wno-deprecated -Wno-unused-but-set-variable

CXXFLAGS += -I./ -I$(PATH_TINYRPC)	-I$(PATH_TOOL) 

LIBS += /usr/lib64/libprotobuf.a	/usr/lib/libtinyxml.a

TOOL_OBJ := $(patsubst $(PATH_TOOL)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_TOOL)/*.cc))

ALL_TESTS : $(PATH_BIN)/test_log 

TEST_CASE_OUT := $(PATH_BIN)/test_log

LIB_OUT := $(PATH_LIB)/libtinyrpc.a

$(PATH_BIN)/test_log: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_log.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

$(LIB_OUT): $(TOOL_OBJ)
	cd $(PATH_OBJ) && ar rcv libtinyrpc.a *.o && cp libtinyrpc.a ../lib/

$(PATH_OBJ)/%.o : $(PATH_TOOL)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# print something test
# like this: make PRINT-PATH_BIN, and then will print variable PATH_BIN
PRINT-% : ; @echo $* = $($*)

# to clean 
clean :
	rm -f $(TOOL_OBJ) $(TESTCASES) $(TEST_CASE_OUT) $(PATH_LIB)/libtinyrpc.a $(PATH_OBJ)/libtinyrpc.a

# install
install:
	mkdir -p $(PATH_INSTALL_INC_TOOL) \
		&& cp $(PATH_TOOL)/*.h $(PATH_INSTALL_INC_TOOL) \
		&& cp $(LIB_OUT) $(PATH_INSTALL_LIB_ROOT)/


# uninstall
uninstall:
	rm -rf $(PATH_INSTALL_INC_ROOT)/TINYRPC && rm -f $(PATH_INSTALL_LIB_ROOT)/libtinyrpc.a


