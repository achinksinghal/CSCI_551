all: sock.o common.o client server clean.o

client: client.o common.o sock.o
	g++ -Wall -o client client.o common.o sock.o -L/home/scf-22/csci551b/openssl/lib -lcrypto -lsocket -lnsl -lresolv

server: server.o common.o sock.o
	g++ -Wall -o server server.o common.o sock.o -L/home/scf-22/csci551b/openssl/lib -lcrypto -lsocket -lnsl -lresolv

client.o: client.cc
	g++ -Wall -c client.cc

server.o: server.cc
	g++ -Wall -c server.cc

common: common.o
	g++ -Wall -o common common.o -lsocket -lnsl -lresolv

common.o: common.cc
	g++ -Wall -c common.cc

sock.o: sock.cc
	g++ -Wall -c -I/home/scf-22/csci551b/openssl/include sock.cc

clean: clean.o
	rm -f common server client

clean.o:
	rm -f *.o
