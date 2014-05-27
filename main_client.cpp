#include "Socket.hh"

int	main(void)
{
  /* Create a Socket of type client to connect to 127.0.0.1:4242 */
  Socket	client("127.0.0.1", 4242);

  /* Message is a typedef on std::pair<std::string, std::string> */
  Message	msg;

  /* Try to connect to remote socket */
  client.start();

  /* Read a message on the socket (Would block if there is no message sended) */
  client.receiveMsg();

  /* While there is a message in the received messages queue.. */
  while (client.getReadQueueSize())
    {
      /* Get the message */
      client.getMessage(msg);

      /* Operator << overloaded, will display Message content */
      std::cout << msg << std::endl;
    }

  return 0;
}
