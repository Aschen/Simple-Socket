#include "Socket.hh"

int	main(void)
{
  int		client_socket;
  std::string	remoteIp;
  Socket	server(4242);

  server.start();

  server.acceptConnection(&client_socket, remoteIp);

  Socket	server_handle(client_socket, remoteIp);


  server.addMessage(Message("TYPE", "MESSAGE"));
  server.addMessage(Message("TEST", "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laboru."));

  while (server.getWriteQueueSize())
    server.writeMsg(server_handle.get());

  server.stop();
  return 0;
}
