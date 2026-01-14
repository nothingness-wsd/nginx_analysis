#ifndef _NIGHT_SOCKET_H_
#define _NIGHT_SOCKET_H_

typedef union night_sockaddr_s night_sockaddr_t;

union night_sockaddr_s
{
    struct sockaddr           sockaddr;
    struct sockaddr_in        sockaddr_in;
    struct sockaddr_in6       sockaddr_in6;
    struct sockaddr_un        sockaddr_un;
};

#endif /* _NIGHT_SOCKET_H_ */
