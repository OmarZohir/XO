#include "game_session.hpp"

GameSession::GameSession() {
  sharedMem = std::make_shared<SharedMemoryManager>();
}

GameSession::~GameSession() {
  sharedMem.reset();
}

