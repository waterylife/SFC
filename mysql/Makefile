#gcc version
CC  = gcc
CXX = g++
GCCVER := $(shell $(CC) -dumpversion | awk -F. '{ print $$1"."$$2}' )

HOM=/home/shawn/sfc/mysql
INC=-I$(HOM)/mysql-connector/include -I$(HOM)
LIB += -L$(HOM)/mysql-connector/lib -lmysqlclient

#BUILD_DIR=../../build 
OBJ_DIR=.
LIB_DIR=.

LIB_NAME=spp_async_test.so 
EXE_NAME=mysql_demo

TARGET = $(LIB_DIR)/$(EXE_NAME)

OBJ = $(OBJ_DIR)/basic.o

OBJ_EXT= .o
CXXSRC_EXT= .cpp
CSRC_EXT= .c

ifeq ($(ARCH),32)
	CFLAGS += -m32 -march=pentium4
endif
CFLAGS  += -g -fPIC -shared -Wall -O2 -pipe -fno-ident -MMD -D_GNU_SOURCE -D_REENTRANT
CXXFLAGS+=$(CFLAGS)
LFLAG =  #-shared

all:$(TARGET)

$(TARGET): $(OBJ)
	@$(CC) -o $@ $(OBJ) $(INC) $(LIB)
	@echo $@
	# cp $(TARGET) /data/shawn/spp/module/
	# scp  $(TAR) shawnmao@10.12.194.107:/data/servers/spp/icson_order/lib

%.o: %.c
	$(CC) $(CFLAGS) -g -c $*.c -o $*.o $(INC)
	@echo $@

# $(OBJ_DIR)/%$(OBJ_EXT): %$(CXXSRC_EXT)
# 	@echo
# 	@echo "Compiling $< ==> $@..."
# 	# @-mkdir -p $(OBJ_DIR)
# 	$(CXX) $(CXXFLAGS) -c $< -o $@ -I/data/shawn/demo

# $(OBJ_DIR)/%$(OBJ_EXT): %$(CSRC_EXT)
# 	@echo
# 	@echo "Compiling $< ==> $@..."
# 	@-mkdir -p $(OBJ_DIR)
# 	$(CC) $(INC) $(CFLAGS) -c $< -o $@


# $(TARGET) : $(OBJ)
# 	@echo
# 	@echo "Building $^ ==> $@..."
# 	@-mkdir -p $(LIB_DIR)
# 	$(CXX) -o $@ $^ $(CXXFLAGS)
	# install *.so ../../bin

clean:
	@echo
	@echo "Cleaning..." 
	rm -rf $(OBJ_DIR)/*.o
	rm -f $(TARGET)	

all32:
	make -e ARCH=32
