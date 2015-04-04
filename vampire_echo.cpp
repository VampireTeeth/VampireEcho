#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>

#include "vampire_echo.hpp"

using namespace std;

VampireEcho::VampireEcho(int port)
  :port(port){
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenfd < 0) {
    cerr << "ERROR on creating socket file descriptor." << endl;
    exit(1);
  }
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  FD_ZERO(&rdset);
  FD_ZERO(&wtset);
}

VampireEcho::~VampireEcho(){
  close(this->listenfd);
}

void VampireEcho::BindSock() {
  if(bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    cerr << "ERROR on binding socket" << endl;
    exit(1);
  }
}

void VampireEcho::StartMainLoop() {
  this->BindSock();
  listen(listenfd, 5);
  fd_set tmprdset, tmpwtset;
  FD_SET(listenfd, &rdset);
  struct sockaddr_in conn_addr;
  socklen_t conn_len;
  conn_len = sizeof(conn_addr);
  int num_ready = 0;
  int maxfd = listenfd;
  int newsockfd, clidx = 0, i;
  const int MAX_SIZE = 1024;
  char buff[MAX_SIZE];
  int clients[FD_SETSIZE];
  char* outbuffs[FD_SETSIZE];
  int outsizes[FD_SETSIZE];

  for(i = 0; i < FD_SETSIZE; i++) {
    clients[i] = -1;
    outbuffs[i] = NULL;
    outsizes[i] = 0;
  }

  cout << "Waiting for connections" << endl;
  while(true) {
    tmprdset = rdset;
    tmpwtset = wtset;
    num_ready = select(maxfd+1, &tmprdset, &tmpwtset, NULL, NULL);
    if(num_ready > 0) {
      if(FD_ISSET(listenfd, &tmprdset)) {
        newsockfd = accept(listenfd, (struct sockaddr *) &conn_addr, &conn_len);
        if(newsockfd < 0) {
          cerr << "ERROR on accept" << endl;
        }
        cout << "INFO new connection: " << newsockfd << endl;
        for(i = 0; i < FD_SETSIZE && clients[i] > 0; i++);
        if(i >= FD_SETSIZE) {
          cerr << "INFO too many connections, try later" << endl;
          close(newsockfd);
        } else {
          clients[i] = newsockfd;
          FD_SET(newsockfd, &rdset);
          if(maxfd < newsockfd) {
            maxfd = newsockfd;
          }
          if(clidx < i) {
            clidx = i;
          }
        }
      }

      for(i = 0; i <= clidx; i++) {
        if(clients[i] > 0 && FD_ISSET(clients[i], &tmprdset)) {
          int num_bytes;
          if((num_bytes = read(clients[i], buff, MAX_SIZE)) > 0) {
            cout << "Bytes read: " << num_bytes << endl;
            char * msg = new char[num_bytes+outsizes[i]];
            if(outbuffs[i] != NULL) {
              bcopy(outbuffs[i], msg, outsizes[i]);
              delete [] outbuffs[i];
            }
            bcopy(buff, msg+outsizes[i], num_bytes);
            outbuffs[i] = msg;
            outsizes[i] += num_bytes;
            FD_SET(clients[i], &wtset);
          }

          if(num_bytes == 0) { // Connection closed by client
            cout << "INFO connection closed " << endl;
            if(outbuffs[i] != NULL) {
              delete [] outbuffs[i];
              outbuffs[i] = NULL;
              outsizes[i] = 0;
            }
            close(clients[i]);
            FD_CLR(clients[i], &rdset);
            clients[i] = -1;
          }
        }

        if(clients[i] > 0 && FD_ISSET(clients[i], &tmpwtset)) {
          if(outbuffs[i] != NULL && outsizes[i] > 0) {
            int num_bytes;
            num_bytes = write(clients[i], outbuffs[i], outsizes[i]);
            if(num_bytes > 0) {
              cout << "Bytes written: " << num_bytes << endl;
              delete [] outbuffs[i];
              outbuffs[i] = NULL;
              outsizes[i] = 0;
              FD_CLR(clients[i], &wtset);
            }
          }
        }
      }
    }
  }// end of while(true)
}
