# Simple One-On-One Chat

This application is a simple one-on-one chat between a client and a server connected using TCP.\
After a connection is established, user on either side can send messages to the other side.\
After the connection is terminated by the client, the server will continue waiting for another client.

After a message is sent, the receiving side will acknowledge it with a "message received" message. Then the sending side will show the roundtrip time for acknowledgment.

### Compilation
To compile, run:
```
make
```

To delete the executables, run:
```
make clean
```

### Usage
The application should start in two modes:
* as a server, waiting for one client to connect or;
* as a client, taking an IP address (or hostname) of server to connect to.

To run the server, open a terminal and run:
```
./server
```

To run the client, open another terminal and run:
```
./client localhost
```
or equivalently,
```
./client 127.0.0.1
```