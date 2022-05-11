all: server client

server: server
	gcc -Wall -g -o server server.c -lpthread -l protobuf-c -lprotobuf
	
client: client
	gcc -Wall -g -o client client.c -lpthread -l protobuf-c -lprotobuf

clean:
	rm -f *.o client
	rm -f *.o server
