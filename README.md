# Socket Programming

A client-server socket program to demonstrate a TCP connection.

## How to run?
Open 1 terminal instance for server. To start the server:
```
make server
```
Open 1 (or several) terminal instance(s) for client. To start the client:
```
make client
```

## What is going on?
1. Server sets up TCP socket to listen for connections.
2. Client sets up TCP socket and initiates the connection.
3. Once connection is established, server creates a new thread for the client and sends a client-id.
4. Client requests for the server's top N CPU consuming processes.
5. Server stores the info in `server_info.txt` and sends the file to the client.
6. Client recieves it and stores it as `recvd_info_<client-id>.txt`.
7. Client identifies the highest CPU consuming process from the file and sends it back to the server.
8. Client closes the connection.