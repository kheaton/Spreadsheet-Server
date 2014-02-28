all: main.o server.o ss_session.o
	g++ main.o server.o ss_session.o -lboost_system -lpthread

main.o: main.cc
	g++ -c main.cc

server.o: server.cc server.h
	g++ -c server.cc

ss_session.o: ss_session.cc ss_session.h
	g++ -c ss_session.cc

spreadsheetserver.o: spreadsheetserver.cc
	g++ -c spreadsheetserver.cc

test: test.o server.o
	g++ test.o server.o -lboost_system -lpthread -o test.out

test.o: test.cc
	g++ -c test.cc

clean:
	rm *.o *.out

do:
	./a.out 1984 & 

do2:
	./a.out 2001 &

dotest:
	./test.out 30000 & 

coutserver: coutserver.o
	g++ coutserver.o -lboost_system -lpthread  -o coutserver.out

runcout:
	./coutserver.out 30001