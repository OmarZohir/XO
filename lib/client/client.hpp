#pragma once
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <mutex>
#include <cstdlib>
#include <iostream>
#include <memory>
#include "game_session.hpp"
#include "game_state.hpp"
struct Client {
private:
  int sock;
  struct sockaddr_in serv_addr;
  // char buffer[1024];
  int shmid;
  char *shared_memory;
  std::shared_ptr<GameState> sharedGameState;
  int playerId;
  key_t sharedMemoryKey;

public:
  Client(const char* serverIP, int serverPort);
  void connectToServer();
  void attachToSharedMemory();
  void getPlayerId();
  void waitForGameStart();
  [[nodiscard]] bool isPlayerTurn() const;
  void makeMove(const std::string& input) const;
  void playGame() const;
  ~Client();
};
