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

INC=-I$(HOM)/public

LIB_NAME=sort_test 

TARGET = $(LIB_DIR)/$(LIB_NAME)

OBJ = $(OBJ_DIR)/test.o \
	  $(PUB_DIR)/common.o \
      

OBJ_EXT= .o
CXXSRC_EXT= .cpp
CSRC_EXT= .c

ifeq ($(ARCH),32)
	CFLAGS += -m32 -march=pentium4
endif
CFLAGS  += -g -fPIC -shared -Wall -O2 -pipe -fno-ident -MMD -D_GNU_SOURCE -D_REENTRANT
CXXFLAGS+=$(CFLAGS)
LFLAG =

all:$(TARGET)

$(TARGET): $(OBJ)
	@$(CXX) $(LFLAG) -o $@ $(OBJ)
	@echo $@

%.o: %.cpp
	$(CXX) -g -c $*.cpp -o $*.o $(INC)
	@echo $@

clean:
	@echo
	@echo "Cleaning..." 
	rm -rf $(OBJ_DIR)/*.o
	rm -f $(TARGET)	
