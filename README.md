Simple-Socket
=============

A simple way to send and receive data through Linux socket API written in C++


Functionality
-------------
 * Create server type and client type Socket
 * Add message in queue who can be send through socket later
 * Can read the socket and push full messages in a queue who can be recover later
 * Manages the chunk and concatenate them to recreate full message
 * See Socket.hh for other functionality 

Informations
------------

 * Some function calls may be blocking like acceptConnection() and receiveMsg() and you need to handle the file descriptors    with select() to avoid this. (May be implemented in futur version)

 * If a function fail and return false, you can get the last error with getError()

Examples
--------
See main_server.cpp and main_client.cpp for exemple of utilisation.

