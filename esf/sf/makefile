DIRS = acce hand conn pkt

all:
	@for dir in $(DIRS); do make -C $$dir; echo; done
clean:
	@for dir in $(DIRS); do make -C $$dir clean; echo; done
	rm -fv */*.txt */*.out */*.err */*~ */*.o */.*.d */*.log
install:
	@for dir in $(DIRS); do make -C $$dir install; echo; done
cl: clean
	@for dir in $(DIRS); do make -C $$dir cl; echo; done
	rm -fv *.txt *.out *.err *~ *.o .*.d *.log
love:
	@for dir in $(DIRS); do make -C $$dir love; echo; done
