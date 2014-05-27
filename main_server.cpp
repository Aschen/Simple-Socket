#include "Socket.hh"

int	main(void)
{
  /* Create a Socket of type server */
  Socket	server(4242);

  /* Open the socket, bind to the port and start listening */
  server.start();

  /* Create a Socket to handle connection */
  Socket	server_handle;

  /* Accept new connection on the socket (Would block if there is no pending connection) */
  server.acceptConnection(server_handle);

  /* Add a message of type 'TYPE' and containing 'MESSAGE' in the pending send message queue */
  server.addMessage(Message("TYPE", "MESSAGE"));

  /* Add another message */
  server.addMessage(Message("TEST", "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laboru."));

  /* Send all message in the message queue to the connected client */
  while (server.sendMsg(server_handle.get()))
    ;

  /* Close the socket (automatically done in destructor */
  server.stop();

  return 0;
}
