#pragma once
#include <__mutex_base>
#include <game_state/game_state.hpp>
#include <shared_mutex>

struct SharedMemoryManager {

  SharedMemoryManager();
  ~SharedMemoryManager();
  std::shared_ptr<GameState> getGameState();
  std::shared_mutex& getMutex();
  [[nodiscard]] key_t get_key() const;
  key_t get_shm_id() const;

private:
    key_t key{};
    int shm_id{};
    std::shared_ptr<GameState> sharedGameState;
    std::shared_mutex mtx{};

};
