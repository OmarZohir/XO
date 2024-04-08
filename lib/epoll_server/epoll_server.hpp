#pragma once
#include <vector>
#include <sys/epoll.h>
#include <map>
#include <memory>
#include "game_state.hpp"
#include "shm_manager.hpp"
#include "game_session.hpp"

struct EpollServer {

  EpollServer();
  ~EpollServer();
  void startServer();
  std::map<unsigned int, std::shared_ptr<GameSession>> gameSessions;
  void runEventLoop();

private:
  int server_fd;
  int epoll_fd;
  std::vector<epoll_event> events;


  void createEpoll();
  void addToEpoll(int clientSocket);
  void handleNewClientConnection();
  void handleIncomingData(int socketFd);
  void runServer();
  void handleEvents(int num_events);
  void acceptClientConnection();
  void receiveClientData(int event_index);
  void sendGameData(int event_index);
  void addClientToGame(int client_socket);
};