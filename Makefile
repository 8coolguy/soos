compile:
	g++ -std=c++11 -pthread server.cpp Table.cpp -o server.o 
	g++ -std=c++11 client.cpp Table.cpp -o client.o -lboost_system -lboost_filesystem
clean:
	rm client.o
	rm server.o
