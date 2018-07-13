# remote-compressor
Remote Compressor is a distributed application based on a client-server model. The client sends files to the server through a TCP socket. The server creates a compressed archive made of the files received from the client and sends this archive back to the client. The user can choose: the compression algorithm used by te server, the name of the compressed archive and the connection port. The server can accept multiple requests at a time thanks to the multithreading support. 

The application is written in C programming language using “Pthreads” library for the multithreading support. Furthermore has been compiled and tested on FreeBSD 6.2.

Remote Compressor has been developed as final project for the network organization exam, part of the Computer Engineering bachelor’s degree program at University of Pisa.
