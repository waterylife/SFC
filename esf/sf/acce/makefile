INCLUDE += -I ../../sys/ -I ../../ipc/ -I ../../net/
LIB	+= -ldl -lpthread

RUNDIR	= /usr/local/esf/bin/

OBJ	:= ../../ipc/esf_ipc_sv.o \
	../../ipc/esf_ipc_thread_sync.o \
	../../ipc/esf_ipc_mq.o \
	../../ipc/esf_ipc_open_mq.o \
	../../sys/esf_sys_debug_log.o \
    ../../sys/esf_sys_error.o \
	../../sys/esf_sys_config_file.o \
	../../net/esf_net_cconn.o \
	../../net/esf_net_epoll_flow.o \
	../../net/esf_net_mem_pool.o \
	../../net/esf_net_raw_cache.o \
	../../net/esf_net_socket_tcp.o \
	../../net/esf_net_accept_t.o \
	acce_main.o

BIN	:= acce

all: $(BIN)
acce: $(OBJ)
	g++ -fPIC -Wall -o $@ $(INCLUDE) $^ $(LIB)

%.o: %.cpp
	g++ -g -fPIC -Wall $(INCLUDE) -c -o $@ $<

clean: 
	rm -rfv $(OBJ) *.o *.out *~ core* $(BIN)
	
install: all
	mkdir -p $(RUNDIR)
	mv -v $(BIN) $(RUNDIR)

love: clean all install
