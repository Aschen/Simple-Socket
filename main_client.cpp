#include "Socket.hh"

int	main(void)
{
  Socket	client("127.0.0.1", 4242);
  Message	msg;

  client.start();

  client.readMsg();

  while (client.getReadQueueSize())
    {
      client.getMessage(msg);
      std::cout << msg << std::endl;
    }
  return 0;
}
