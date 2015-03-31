// Filename: socket.cpp
// Author: liubj77 - liubj77@gmail.com

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <limits.h>
#include <poll.h>
#include <errno.h>
#include "common/socket.h"

namespace common {

int Socket::tcp_set_blocking(int sfd, int blocking) 
{
    int flags;

    if ((flags = fcntl(sfd, F_GETFL, 0)) == -1) {
        perror("fcntl");
        return -1;
    }
    
    if (blocking) {
        flags &= ~O_NONBLOCK;
    } else {
        flags != O_NONBLOCK;
    }

    if (fcntl(sfd, F_SETFL, flags) == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

int Socket::tcp_set_nodelay(int sfd, int nodelay) 
{
    int optval = nodelay ? 1 : 0;
    if (setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, &optval, 
                static_cast<socklen_t>(sizeof optval)) != 0) {
        perror("setsockopt");
        return -1;
    }

    return 0;
}

int Socket::tcp_create_bind(const char *addr, int port) 
{
    int s, sfd = -1;
    char c_port[6]; /* strlen("65535") */
    struct addrinfo hints;
    struct addrinfo *serverinfo, *sp;

    snprintf(c_port, 6, "%d", port);
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;  // return IPv4 and IPv6 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // all interfaces

    if ((s = getaddrinfo(addr, c_port, &hints, &serverinfo)) != 0) {
        return -1;
    }

    for (sp = serverinfo; sp != NULL; sp = sp->ai_next) {
        if ((sfd = socket(sp->ai_family, sp->ai_socktype, sp->ai_protocol)) == -1) {
            continue;
        }

        if ((s = bind(sfd, sp->ai_addr, sp->ai_addrlen)) == 0) {
            break;
        } 

        close(sfd);
    }

    if (sp == NULL) {
        fprintf(stderr, "counld not create and bind %s:%d\n", addr, port);
        freeaddrinfo(serverinfo);
        return -1;
    }
    
    freeaddrinfo(serverinfo);

    return sfd;
}

int Socket::tcp_connect(int sfd, const char *addr, int port,
                        const struct timeval *timeout, 
                        const char *source_addr) 
{
    struct sockaddr_in sock_addr;
    socklen_t addrlen = sizeof(sock_addr);

    if (source_addr && !tcp_bind_local(sfd, source_addr) != 0) {
        return -1;
    }

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    inet_aton(addr, &(sock_addr.sin_addr));
    sock_addr.sin_port = htons(port);
    
    if (_tcp_connect(sfd, (struct sockaddr *)&sock_addr, addrlen, timeout) != 0) {
        return -1;
    }
    
    return 0;
}

int Socket::tcp_create_connect(int *sfd, 
                              const char *addr, int port, 
                              const struct timeval *timeout, 
                              const char *source_addr) {
    int s, _sfd, ret = -1;
    char c_port[6];
    struct addrinfo hints;
    struct addrinfo *serverinfo, *sp;

    snprintf(c_port, 6, "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((s = getaddrinfo(addr, c_port, &hints, &serverinfo)) != 0) {
        /* try IPv6 if IPv4 fail */
        hints.ai_family = AF_INET6;
        if ((s = getaddrinfo(addr, c_port, &hints, &serverinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            return -1;
        }
    }

    for (sp = serverinfo; sp != NULL; sp = sp->ai_next) {
        if ((_sfd = socket(sp->ai_family, sp->ai_socktype, sp->ai_protocol)) == -1) {
            continue;
        }
        
        if (tcp_set_blocking(_sfd, 0) != 0) {
            goto error;
        }

        // client bind local address
        if (source_addr && !tcp_bind_local(_sfd, source_addr)) {
            fprintf(stderr, "can not bind local address: %s\n", source_addr);
            goto error;
        }

        /* connect server */
        if (_tcp_connect(_sfd, sp->ai_addr, sp->ai_addrlen, timeout) != 0) {
            goto error;
        } else {
            *sfd = _sfd;
            ret = 0;
            goto end;
        }
    }
error:
    if (_sfd != -1) {
        close(_sfd);
    }
end:
    freeaddrinfo(serverinfo);

    return ret;
}

int Socket::tcp_bind_local(int sfd, const char *source_addr)
{
    if (source_addr == NULL) {
        return -1;
    }

    struct addrinfo hints, *bservinfo, *bp;
    int res;

    if ((res = getaddrinfo(source_addr, NULL, &hints, &bservinfo)) != 0) {
        fprintf(stderr, "can not get addrinfo: %s", gai_strerror(res));
        return -1;
    }

    for (bp = bservinfo; bp != NULL; bp = bp->ai_next) {
        if (bind(sfd, bp->ai_addr, bp->ai_addrlen) != -1) {
            // bind local success
            break;
        }
    }

    freeaddrinfo(bservinfo);

    return 0;
}

int Socket::_tcp_connect(int sfd, const struct sockaddr *addr, 
                        socklen_t addrlen, const struct timeval *timeout)
{
    if (connect(sfd, addr, addrlen) == -1) {
        if (errno == EINPROGRESS) {
           return  _tcp_connect_wait_ready(sfd, timeout); 
        } else {
            return -1;
        }
    }

    return 0;
}

#define __MAX_MSEC (((LONG_MAX) - 999) / 1000)

int Socket::_tcp_connect_wait_ready(int sfd, const struct timeval *timeout) {
    struct pollfd wfd[1];
    int64_t msec = -1;

    wfd[0].fd = sfd;
    wfd[0].events = POLLOUT;

    if (timeout) {
        if (timeout->tv_usec > 1000000 || timeout->tv_sec > __MAX_MSEC) {
            return -1;
        }
        
        msec = (timeout->tv_sec * 1000) + ((timeout->tv_usec + 999) / 1000);

        if (msec < 0 || msec > INT_MAX) {
            msec = INT_MAX;
        }
    }

    int res, err;
    socklen_t err_len = sizeof(err);
    if ((res = poll(wfd, 1, msec)) == -1) {
        return -1;
    } else if (res == 0) {
        return -1;
    }

    /* getsockopt SO_ERROR */
    if (getsockopt(sfd, SOL_SOCKET, SO_ERROR, &err, &err_len) == -1) {
        fprintf(stderr, "getsockopt:%s\n", strerror(err));
        return -1;
    }

    return 0;
}

int Socket::get_interface_ip(const char *interface, char *address, int size)
{
    struct ifaddrs *ifap, *p;
    struct sockaddr_in *sain;
    struct sockaddr_in6 *sain6;
    int ret = -1;
    char buf[NI_MAXHOST];

    if (getifaddrs(&ifap) == -1) {
        fprintf(stderr, "getifaddrs: %s\n", strerror(errno));
        return -1;
    }

    for (p = ifap; p != NULL; p = p->ifa_next) {
        if (p->ifa_addr == NULL) {
            continue;
        }

        if (strcmp(p->ifa_name, interface) != 0) {
            continue;
        }
        switch (p->ifa_addr->sa_family) {
        case AF_INET:
            sain = (struct sockaddr_in *)p->ifa_addr;
            inet_ntop(AF_INET, (void*)&(sain->sin_addr), buf, NI_MAXHOST);
            if (size <= strlen(buf)) {
                break;
            } else {
                strncpy(address, buf, size);
                ret = 0;
            }
            break;
        case AF_INET6:
            sain6 = (struct sockaddr_in6 *)p->ifa_addr;
            inet_ntop(AF_INET6, (void*)&(sain6->sin6_addr), buf, NI_MAXHOST);
            if (size <= strlen(buf)) {
                break;
            } else {
                strncpy(address, buf, size);
                ret = 0;
            }
            break;
        default:
            continue;
        }
        
        // after get interface sucess, may IPv4 or IPv6
        if (ret == 0) {
            break;
        }
    }

    freeifaddrs(ifap);

    return ret;
}

} /* namespace common */
















