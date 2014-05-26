#include "Socket.hh"


Socket::Socket(int port)
    : _port(port), _addrFamily(AF_INET), _protoName("TCP"),
      _communicationType(SOCK_STREAM), _interface("wlan0"),
      _listeningAddress(INADDR_ANY), _maxConnections(16),
      _protocol(NULL), _socket(-1), _ip("127.0.0.1"), _type(Socket::SERVER),
      _remoteIp(""), _active(false)
{
}

Socket::Socket(std::string listeningAdress, int port)
    : _port(port), _addrFamily(AF_INET), _protoName("TCP"),
      _communicationType(SOCK_STREAM), _interface("wlan0"),
      _listeningAddress(inet_addr(listeningAdress.c_str())), _maxConnections(-1),
      _protocol(NULL), _socket(-1), _ip("127.0.0.1"), _type(Socket::CLIENT),
      _remoteIp(""), _active(false)
{
}

Socket::Socket(int socket, std::string remoteIp)
    : _port(-1), _addrFamily(AF_INET), _protoName("TCP"),
      _communicationType(SOCK_STREAM), _interface("wlan0"),
      _listeningAddress(-1), _maxConnections(-1),
      _protocol(NULL), _socket(socket), _ip("127.0.0.1"), _type(Socket::SERVER_HANDLE),
      _remoteIp(remoteIp), _active(true)
{
}

Socket::Socket(int port,
               in_addr_t listeningAdress,
               sa_family_t addrFamily,
               std::string protoName,
               int communicationType,
               std::string interface,
               int maxConnections)
    : _port(port), _addrFamily(addrFamily), _protoName(protoName),
      _communicationType(communicationType), _interface(interface),
      _listeningAddress(listeningAdress), _maxConnections(maxConnections),
      _protocol(NULL), _socket(-1), _ip("127.0.0.1"), _type(Socket::SERVER),
      _remoteIp(""), _active(false)
{
}

Socket::Socket(int port,
               std::string listeningAdress,
               sa_family_t addrFamily,
               std::string protoName,
               int communicationType,
               std::string interface)
    : _port(port), _addrFamily(addrFamily), _protoName(protoName),
      _communicationType(communicationType), _interface(interface),
      _listeningAddress(inet_addr(listeningAdress.c_str())), _maxConnections(-1),
      _protocol(NULL), _socket(-1), _ip("127.0.0.1"), _type(Socket::CLIENT),
      _remoteIp(""), _active(false)
{
}

Socket::~Socket(void)
{
    stop();
}

bool Socket::start(void)
{
    int                 t = 1;
    struct sockaddr_in  sin;

    if (_type == Socket::SERVER_HANDLE)
    {
        _error.assign("Can't start Socket of type Socket::SERVER_HANDLE");
        return false;
    }

    signal(SIGPIPE, &brokenPipe);

    /* Set protocol */
    if (!setProtocol(_protoName))
    {
        return false;
    }

    /* Create the socket */
    _socket = socket(_addrFamily, _communicationType, _protocol->p_proto);
    if (_socket < 0)
    {
        _error.assign(std::string("Can't create socket : ") + std::string(strerror(errno)));
        return false;
    }

    /* Get local ip addr */
    getLocalIp();

    sin.sin_family = _addrFamily;
    sin.sin_port = htons(_port);
    sin.sin_addr.s_addr = _listeningAddress;
    if (_type == Socket::SERVER)
    {
        /* Bind socket on port and listening addr */
        if (bind(_socket, (struct sockaddr*)&sin, sizeof(sin)) < 0)
        {
            _error.assign(std::string("Can't bind socket : ") + std::string(strerror(errno)));
            return false;
        }

        /* Start listening */
        if (listen(_socket, _maxConnections) < 0)
        {
            _error.assign(std::string("Can't listen on socket : ") + std::string(strerror(errno)));
            return false;
        }
        _active = true;
    }
    else if (_type == Socket::CLIENT)
    {
        /* Connect to remote host */
        if (connect(_socket, (const struct sockaddr *)(&sin), sizeof(sin)) < 0)
        {
            _error.assign(std::string("Can't connect to host : ") + std::string(strerror(errno)));
            return false;
        }
        _active = true;
        _remoteIp.assign(inet_ntoa(sin.sin_addr));
    }
    return true;
}

bool Socket::stop(void)
{
    if (_active == false)
    {
        _error.assign(std::string("Socket is not running."));
        return false;
    }

    close(_socket);
    _socket = -1;
    _active = false;
    return true;
}

bool Socket::writeMsg(int fd)
{
    std::string     frame;
    int             ret = 0;

    if (_active == false)
    {
        _error.assign(std::string("Socket is not running."));
        return false;
    }

    if (fd == -2)
    {
        fd = _socket;
    }
    /* Verification si il y a un message a envoyer */
    if (_writeQueue.size() == 0)
    {
        _error.assign(std::string("No message to send in socket."));
        return false;
    }

    /* On verifie si la derniere frame a été entierement envoyée */
    if (_tmpWriteBuf.size())
    {
        frame.append(_tmpWriteBuf);
        _tmpWriteBuf.clear();
    }

    /* On transforme le message en frame */
    messageToFrame(_writeQueue.front(), frame);

    /* On envoi le maximum du message dans la socket */
    if ((ret = write(fd, frame.c_str(), frame.size())) < 0)
    {
        _error.assign(std::string("Can't write on socket : ") + std::string(strerror(errno)));
        return false;
    }

    /* On supprime le bout du message déja envoyé */
    frame.erase(0, ret);

    /* On supprime le message de la queue */
    _writeQueue.pop_front();

    /* Si la frame n'a pas été envoyé en entier, on le sauvegarde */
    if (frame.size())
    {
        _tmpWriteBuf.assign(frame);
    }

    return true;
}

int Socket::readMsg(int fd)
{
    char            buf[512] = {0};
    int             ret = 0;
    std::string     frame;

    if (_active == false)
    {
        _error.assign(std::string("Socket is not running."));
        return false;
    }

    if (fd == -2)
    {
        fd = _socket;
    }

    /* On read 512 caractères dans la socket */
    if ((ret = read(fd, &buf[0], 512)) < 0)
    {
        _error.assign(std::string("Can't read on socket : ") + std::string(strerror(errno)));
        return -1;
    }
    else if (ret == 0) // Si client deconnecté
    {
        return 0;
    }

    /* On copie le nombre de caractères lut dans le buffer */
    frame.assign(buf, ret);

    /* Si il existait une frame fragmenté */
    if (_tmpReadBuf.size())
    {
        frame.insert(0, _tmpReadBuf);
        _tmpReadBuf.clear();
    }

    /* On ajoute la/les frame(s) lu(ent) a la liste des messages recu */
    pushFrame(frame);
    return 1;
}

void Socket::pushFrame(std::string &frames)
{
    Message             tmp;
    size_t              head;
    size_t              begin;
    size_t              end;

    /* Temps qu'on trouve le caractère de controle de fin de frame */
    while ((end = frames.find(Socket::FRAME_END)) != std::string::npos)
    {
        if ((head = frames.find(Socket::FRAME_HEAD)) != std::string::npos
                && (begin = frames.find(Socket::FRAME_BEGIN)) != std::string::npos)
        {
            /* On extrait le type de message */
            tmp.first = frames.substr(head + 1, begin - head - 1);
            /* On extrait le message */
            tmp.second = frames.substr(begin + 1, end - begin - 1);
            /* On supprime entierement la frame extraite */
            frames.erase(head, end - head + 1);
            /* On push le message dans la file des messages recu */
            _readQueue.push_back(tmp);
        }
        else
        {
            std::cout << "Ne devrait pas arriver" << std::endl;
        }
    }

    /* Si il reste un bout de frame, on le sauvegarde */
    if (frames.size())
    {
        _tmpReadBuf.assign(frames);
    }
}

void Socket::messageToFrame(const Message &msg, std::string &frame) const
{
    /* On insère le caractère de controle de début de frame */
    frame.push_back(Socket::FRAME_HEAD);

    /* On insère le type de message */
    frame.append(msg.first);

    /* On insère le caractère de controle de début de message*/
    frame.push_back(Socket::FRAME_BEGIN);

    /* On insère le message */
    frame.append(msg.second);

    /* On insère le caractère de controle de fin de frame */
    frame.push_back(Socket::FRAME_END);
}

bool Socket::addMessage(Message msg)
{
    if (msg.first.size() == 0 && msg.second.size() == 0)
    {
        _error.assign(std::string("You can't push an empty message to the queue."));
        return false;
    }
    _writeQueue.push_back(msg);
    return true;
}

bool Socket::getMessage(Message &msg)
{
    if (_readQueue.size() == 0)
    {
        _error.assign(std::string("No pending message in the queue."));
        return false;
    }
    msg = _readQueue.front();
    _readQueue.pop_front();
    return true;
}

bool Socket::setOption(int optName, int level)
{
    int                 t = 1;

    if (setsockopt(_socket, level, optName, &t, sizeof(int)) < 0)
    {
        _error.assign(std::string("Can't set socket option : ") + std::string(strerror(errno)));
        return false;
    }
    return true;
}

bool Socket::setProtocol(std::string protoName)
{
    struct protoent *protocol;

    protocol = getprotobyname(protoName.c_str());
    if (!protocol)
    {
        _error.assign(std::string("Protocol : " + protoName + " incorrect."));
        return false;
    }
    _protocol = protocol;
    return true;
}

const std::string &Socket::getProtocol(void) const
{
    return _protoName;
}

const std::string &Socket::getLocalIp(std::string interface)
{
    struct ifreq        ifr;

    /* Get local ip addr */
    ifr.ifr_addr.sa_family = _addrFamily;
    strncpy(ifr.ifr_name, _interface.c_str(), IFNAMSIZ - 1);
    if (ioctl(_socket, SIOCGIFADDR, &ifr) < 0)
    {
        _error.assign(std::string("Invalid interface : " + _interface));
        _ip = "127.0.0.1";
    }
    else
    {
        _ip.assign(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    }
    return _ip;
}

const std::string &Socket::getRemoteIp(void)
{
    if (_type == Socket::SERVER)
    {
        _error.assign(std::string("No remote IP available."));
    }
    return _remoteIp;
}

int Socket::get(void)
{
    if (_active == false)
    {
        _error.assign(std::string("Socket is not running."));
    }
    return _socket;
}

int Socket::getPort(void) const
{
    return _port;
}

Socket::SocketType Socket::getType(void) const
{
    return _type;
}

std::string Socket::getState(void) const
{
    std::stringstream       ss;
    std::string             state;

    if (_active)
    {
        if (_type == Socket::SERVER)
        {
            ss << "Socket listening on port " << _port;
        }
        else if (_type == Socket::CLIENT)
        {
            ss << "Socket connected to host " << _remoteIp << ":" << _port;
        }
        else
        {
            ss << "Socket connected to host " << _remoteIp;
        }
    }
    else
    {
        ss << "Socket closed.";
    }
    state.assign(ss.str());
    return state;
}

void Socket::dumpReadQueue(void) const
{
    std::list<Message>::const_iterator  it = _readQueue.begin();

    if (_readQueue.size())
    {
        std::cout << "Messages received : " << std::endl;
        for (; it != _readQueue.end(); ++it)
        {
            std::cout << *it << std::endl;
        }
    }
    else
    {
        std::cout << "No messages received." << std::endl;
    }
}

void Socket::dumpWriteQueue(void) const
{
    std::list<Message>::const_iterator  it = _writeQueue.begin();

    if (_writeQueue.size())
    {
        std::cout << "Messages to send : " << std::endl;
        for (; it != _writeQueue.end(); ++it)
        {
            std::cout << *it << std::endl;
        }
    }
    else
    {
        std::cout << "No messages to send" << std::endl;
    }
}

int Socket::getReadQueueSize(void) const
{
    return _readQueue.size();
}

int Socket::getWriteQueueSize(void) const
{
    return _writeQueue.size();
}

const std::string &Socket::getError(void) const
{
    return _error;
}



std::ostream            &operator<<(std::ostream &os, Socket::SocketType other)
{
    switch (other)
    {
    case Socket::CLIENT:
        os << "CLIENT";
        break;
    case Socket::SERVER:
        os << "SERVER";
        break;
    case Socket::SERVER_HANDLE:
        os << "SERVER_HANDLE";
        break;
    }
    return os;
}

std::ostream            &operator<<(std::ostream &os, const Message &other)
{
    os << "Type : " << other.first << ", Message : " << other.second;
    return os;
}

void                    brokenPipe(int signum)
{
    (void)signum;
    std::cout << "Broken pipe." << std::endl;
}
