// Filename: socket.h
// Author: liubj77 - liubj77@gmail.com

#ifndef  _COMMON_SOCKET_H_
#define  _COMMON_SOCKET_H_

#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace common {

class Socket {
public:
    static int tcp_set_blocking(int sfd, int blocking);
    static int tcp_set_nodelay(int sfd, int nodelay);
    
    static int tcp_create_bind(const char *addr, int port);

    static int tcp_connect(int sfd, 
                           const char *addr,
                           int port,
                           const struct timeval *timeout,
                           const char *source_addr);

    static int tcp_create_connect(int *sfd,
                                  const char *addr,
                                  int port,
                                  const struct timeval *timeout,
                                  const char *source_addr); 

    static int tcp_bind_local(int sfd, const char *source_addr);  

    static int get_interface_ip(const char *interface, char *address, int size);

private:
    static int _tcp_connect(int sfd, 
                            const struct sockaddr *addr,
                            socklen_t addrlen,
                            const struct timeval *timeout);

    static int _tcp_connect_wait_ready(int sfd, const struct timeval *timeout);
};

} /* namespace common */


#endif //SOCKET_H_

