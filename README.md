Simple-Socket
=============

A simple way to send and receive data through Linux socket API written in C++


See main_server.cpp and main_client.cpp for exemple of utilisation.

Some function calls may be blocking like acceptConnection() and receiveMsg() and you need to handle the file descriptors with select() to avoid this.
(May be implemented in futur version)

If a function fail and return false, you can get the last error with getError()
