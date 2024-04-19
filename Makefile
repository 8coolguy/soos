compile:
	g++ -std=c++11 server.cpp -o server.o
	g++ -std=c++11 client.cpp -o client.o
clean:
	rm client.o
	rm server.o
	rm *.txt

