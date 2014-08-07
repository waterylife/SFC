
RUNDIR	= /usr/local/esf/bin/

BIN	:= asn_complete.so web_complete.so json_complete.so net_complete.so
all: $(BIN)

asn_complete.so: asn_complete.o
	g++ -Wall -fPIC -o $@ $^ -shared

web_complete.so: web_complete.o
	g++ -Wall -fPIC -o $@ $^ -shared

json_complete.so: json_complete.o
	g++ -Wall -fPIC -o $@ $^ -shared

net_complete.so: net_complete.o
	g++ -Wall -fPIC -o $@ $^ -shared

%.o: %.cpp
	g++ -g -Wall -fPIC $(INCLUDE) -c -o $@ $<

clean: 
	rm -rfv $(OBJ) *.o *.out *~ core* $(BIN)
	
install: all
	mkdir -p $(RUNDIR)
	mv -v $(BIN) $(RUNDIR)
	
love: clean all install
