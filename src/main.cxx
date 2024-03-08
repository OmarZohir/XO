#include <epoll_server/epoll_server.hpp>
#include <csignal>
#include <spdlog/spdlog.h>

int main() {
  EpollServer server;
  server.startServer();

  return 0;
}
