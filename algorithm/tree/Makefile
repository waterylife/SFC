#gcc version
CC  = gcc
CXX = g++
GCCVER := $(shell $(CC) -dumpversion | awk -F. '{ print $$1"."$$2}' )

HOM=/Users/Shawn/Workspace/sfc

#LIB += -lsppasync -lasync_epoll -lcore

#BUILD_DIR=../../build 
PUB_DIR=$(HOM)/public
OBJ_DIR=./
LIB_DIR=.

INC=-I$(HOM) -I$(PUB_DIR)

LIB_NAME1=test_rbtree
LIB_NAME2=test_bintree

TARGET1 = $(LIB_DIR)/$(LIB_NAME1)
TARGET2 = $(LIB_DIR)/$(LIB_NAME2)

OBJ1 = $(OBJ_DIR)/test_rbtree.o \
      $(PUB_DIR)/common.o

OBJ2 = $(OBJ_DIR)/test_binsearchtree.o \
      $(PUB_DIR)/common.o

OBJ_EXT= .o
CXXSRC_EXT= .cpp
CSRC_EXT= .c

ifeq ($(ARCH),32)
	CFLAGS += -m32 -march=pentium4
endif
CFLAGS  += -g -fPIC -shared -Wall -O2 -pipe -fno-ident -MMD -D_GNU_SOURCE -D_REENTRANT
CXXFLAGS+=$(CFLAGS)
LFLAG =

rbtree:$(TARGET1)
bintree:$(TARGET2)

$(TARGET1): $(OBJ1)
	@$(CXX) $(LFLAG) -g -o $@ $(OBJ1) $(INC)
	@echo $@

$(TARGET2): $(OBJ2)
	@$(CXX) $(LFLAG) -g -o $@ $(OBJ2) $(INC)
	@echo $@

%.o: %.cpp
	$(CXX)  -g -c $*.cpp -o $*.o $(INC)
	@echo $@

clean:
	@echo
	@echo "Cleaning..." 
	rm -rf $(OBJ_DIR)/*.o
	rm -rf $(PUB_DIR)/*.o
	rm -f $(TARGET1)
	rm -f $(TARGET2)