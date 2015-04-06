#include <iostream>
#include <unistd.h>
#include <cstdlib>

#include "./vampire_echo.hpp"

using std::endl;
using std::cout;
using std::cerr;
using std::atoi;
using std::exit;

int main(int argc, char * argv[]) {

  int port;
  if(argc < 2) {
    cerr << "No port number provided." << endl;
    exit(1);
  }
  port = atoi(argv[1]);
  VampireEcho s = port;
  s.StartMainLoop();
  return 0;
}
