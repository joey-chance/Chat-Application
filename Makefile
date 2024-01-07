all:
	g++ -std=c++11 src/server.cpp -o server
	g++ -std=c++11 src/client.cpp -o client

clean:
	rm server
	rm client