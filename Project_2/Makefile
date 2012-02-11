all: mm2 queue.o time_551.o clean.o

mm2: mm2.o queue.o time_551.o
	g++ -o mm2 mm2.o queue.o time_551.o -lpthread 

mm2.o: mm2.cc
	g++ -c mm2.cc

queue.o: queue.cc
	g++ -c queue.cc

time_551.o: time_551.cc
	g++ -c time_551.cc

clean: clean.o
	rm -f mm2

clean.o:
	rm -f *.o
