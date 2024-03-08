#include "shm_manager.hpp"
#include <random>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <spdlog/spdlog.h>

SharedMemoryManager::SharedMemoryManager() {
  // Set up shared memory for the game state object
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr;
  int random_num = distr(gen);
  const char* cur_path = "/storage/developer/projects/b3-common/ouro/apps/cli/epoll/lib/epoll_server";
  key = ftok(cur_path, random_num);
  if (key == -1) {
    std::cerr << "ftok error: Unable to generate key. Error number: " << errno << " - " << std::strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }

  shm_id = shmget(key, sizeof(GameState), 0666 | IPC_CREAT);
  if (shm_id == -1) {
    spdlog::error("SharedMemoryManager: Failed to create shared memory segment");
    exit(EXIT_FAILURE);
  }

  // Check if the shared memory segment already exists
  void* shared_memory = shmat(shm_id, nullptr, 0);
  if (*static_cast<int*>(shared_memory) != 0) {
    // The shared memory segment already exists, so attach to it
    sharedGameState = std::make_shared<GameState>(*static_cast<GameState*>(shared_memory));
  } else {
    // The shared memory segment does not exist, so create a new GameState object and attach to it
    sharedGameState = std::make_shared<GameState>();
    memcpy(shared_memory, sharedGameState.get(), sizeof(GameState));
  }

  spdlog::info("SharedMemoryManager: Created shared memory with id {} and attached to game state", shm_id);
}

SharedMemoryManager::~SharedMemoryManager() {
  // Detach shared memory segment
  shmdt(sharedGameState.get());
  // Remove shared memory segment
  shmctl(shm_id, IPC_RMID, NULL);
}

std::shared_ptr<GameState> SharedMemoryManager::getGameState() {
  return sharedGameState;
}

std::shared_mutex& SharedMemoryManager::getMutex() {
  return mtx;
}

key_t SharedMemoryManager::get_key() const {
  return key;
}

key_t SharedMemoryManager::get_shm_id() const {
  return shm_id;
}