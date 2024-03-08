#pragma once
#include <shared_mem/shm_manager.hpp>
#include <memory>

struct GameSession {
  GameSession();
  ~GameSession();
  int connectedClients = 0;
  int addPlayer();
  void setPlayerId(int id);
  void addPlayer();
  bool hasGameStarted();
  std::shared_ptr<SharedMemoryManager> sharedMem;
};