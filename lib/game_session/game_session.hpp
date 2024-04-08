#pragma once
#include <memory>
#include "shm_manager.hpp"

struct GameSession {
  GameSession();
  ~GameSession();
  int connectedClients = 0;
  int addPlayer();
  void setPlayerId(int id);
  bool hasGameStarted();
  std::shared_ptr<SharedMemoryManager> sharedMem;
};