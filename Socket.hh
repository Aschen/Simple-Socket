#ifndef SOCKET_HH
#define SOCKET_HH

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <iostream>
#include <string>
#include <exception>
#include <sstream>
#include <list>

typedef std::pair<std::string, std::string> Message;

/**
 * @brief Classe utilisé pour gérer les communications à travers les sockets
 */
class   Socket
{
public:
    const static char               FRAME_HEAD = 1;     // DET
    const static char               FRAME_BEGIN = 2;    // FTX
    const static char               FRAME_END = 3;      // DTX

    typedef enum    SocketType
    {
        SERVER = 1,
        SERVER_HANDLE,
        CLIENT
    }               SocketType;

private:
    int                         _port;
    sa_family_t                 _addrFamily;
    std::string                 _protoName;
    int                         _communicationType;
    std::string                 _interface;
    in_addr_t                   _listeningAddress;
    int                         _maxConnections;
    struct protoent             *_protocol;
    int                         _socket;
    std::string                 _ip;
    SocketType                  _type;
    std::string                 _remoteIp;
    bool                        _active;
    std::list<Message>          _writeQueue;
    std::list<Message>          _readQueue;
    std::string                 _tmpReadBuf;
    std::string                 _tmpWriteBuf;
    std::string                 _error;

    public:
    ~Socket(void);
    /**
     * @brief Créer une socket d'écoute par défaut (type serveur)
     * @param port Port d'écoute de la socket
     */
    Socket(int port);

    /**
     * @brief Créer une socket de connection par défaut (type client)
     * @param port Port de l'hote distant
     * @param listeningAdress Adresse IP de l'hote distant
     */
    Socket(std::string listeningAdress, int port);

    /**
     * @brief Créer une socket pour communiquer avec les clients connectés au serveur (accept)
     * @param socket file descriptor
     * @param remoteIp ip du client connecté
     */
    Socket(int socket, std::string remoteIp);

    /**
     * @brief Créer une socket pour communiquer avec les clients connectés au serveur (accept)
     */
    Socket(void);

    /**
     * @brief Créer une socket d'écoute en personnalisant les options
     */
    Socket(int port,
           in_addr_t listeningAdress,
           sa_family_t addrFamily,
           std::string protoName,
           int communicationType,
           std::string interface,
           int maxConnections);
    /**
     * @brief Créer une socket de connection en personnalisant les options
     */
    Socket(int port,
           std::string listeningAdress,
           sa_family_t addrFamily,
           std::string protoName,
           int communicationType,
           std::string interface);
private:
    Socket(const Socket &cpy);
    Socket      &operator=(const Socket &cpy);

public:
    /**
     * @brief ouvre la socket selon son type
     */
    bool                start(void);
    /**
     * @brief Ferme la socket
     */
    bool		stop(void);
    /**
     * @brief Envoi le premier message de la file dans la socket
     */
    bool                sendMsg(int fd = -2);
    /**
     * @brief Lit les données présentes dans la socket et empile un ou plusieurs message
     * @return Retourne -1 en cas d'erreur et 0 si le client est deconnecté
     */
    int                 receiveMsg(int fd = -2);
    /**
     * @brief Ajoute un message dans la file d'attente d'envoi
     * @param type de message
     * @param message
     */
    bool                addMessage(Message msg);
    /**
     * @brief Renvoi le premier message présent dans la file des messages recus
     */
    bool                getMessage(Message &msg);
    /**
     * @brief Accepte une nouvelle connection depuis la socket (SERVER uniquement)
     */
    bool                acceptConnection(Socket &socket);

private:
    void                pushFrame(std::string &frames);
    void                messageToFrame(const Message &msg, std::string &frame) const;

    /* Getters/Setters*/
public:
    int                 get(void);
    bool                set(int socket);
    bool                setOption(int optName, int level = SOL_SOCKET);
    bool                setProtocol(std::string protoName);
    bool                setInterface(std::string interface);
    void                setListeningAdress(in_addr_t adress);
    void                setListeningAdress(std::string adress);
    const std::string   &getProtocol(void) const;
    bool                setRemoteIp(std::string remoteIp);
    const std::string   &getLocalIp(void) const;
    const std::string   &getRemoteIp(void);
    int                 getPort(void) const;
    Socket::SocketType  getType(void) const;

    /* Fonctions d'informations sur l'état de la socket */
public:
    std::string         getState(void) const;
    void                dumpReadQueue(void) const;
    void                dumpWriteQueue(void) const;
    int                 getReadQueueSize(void) const;
    int                 getWriteQueueSize(void) const;
    const std::string   &getError(void) const;
};

std::ostream            &operator<<(std::ostream &os, Socket::SocketType other);
std::ostream            &operator<<(std::ostream &os, const Message &other);
void                    brokenPipe(int signum);

#endif // SOCKET_HH
