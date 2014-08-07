
CFLAGS = -g -Wall -fPIC

INCLUDE += -I ../../sys/ -I ../../ipc/ -I ../../net/
LIB     += -ldl

RUNDIR	= /usr/local/esf/bin/

OBJ	:= ../../sys/esf_sys_config_file.o \
	../../sys/esf_sys_binlog.o \
	../../net/esf_net_epoll_flow.o \
	../../ipc/esf_ipc_sv.o \
	../../ipc/esf_ipc_mq.o \
	../../ipc/esf_ipc_open_mq.o \
	../../ipc/esf_ipc_cache_chunk_alloc.o \
	../../ipc/esf_ipc_cache_hash_map.o \
	../../ipc/esf_ipc_cache_access.o \
	hand_main.o

BIN     := hand

all: $(BIN)
hand: $(OBJ)
	g++ $(CFLAGS) -o $@ $(INCLUDE) $^ $(LIB)
 
%.o: %.cpp
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $^

clean: 
	rm -rfv $(OBJ) *.o *.out *~ core* $(BIN)
	
install: all
	mkdir -p $(RUNDIR)
	mv -v $(BIN) $(RUNDIR)

love: clean all install
