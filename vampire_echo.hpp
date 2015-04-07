#ifndef __VAMPIRE_ECHO_H__
#define __VAMPIRE_ECHO_H__
#include <sys/select.h>
#include <netinet/in.h>

class VampireEcho {
public:
  VampireEcho(int port);
  virtual ~VampireEcho();
  void StartMainLoop();

private:
  void BindSock();
  int port;
  int listenfd;
  struct sockaddr_in server_addr;
  fd_set rdset;
  fd_set wtset;
};

#endif
