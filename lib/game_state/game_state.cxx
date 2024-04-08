#include "game_state.hpp"
#include <spdlog/spdlog.h>

GameState::GameState() {
  for (auto & i : grid) {
    for (auto & j : i) {
      j = Entry::EMPTY;
    }
  }
  connectedPlayers = 0;
  player0Turn = true;
  gameOver = false;
  gameStarted = false;
}

bool GameState::makeMove(int x, int y) {
  if (grid[x][y] == Entry::EMPTY) {
    if (player0Turn) {
      grid[x][y] = Entry::X;
    } else {
      grid[x][y] = Entry::O;
    }
    player0Turn = !player0Turn;
    return true;
  }
    return false;
}

bool GameState::checkGameOver(){
  // Horizontal lines match
  for (auto & i : grid) {
    if (i[0] != Entry::EMPTY && i[0] == i[1] && i[1] == i[2]) {
      gameOver = true;
      return true;
    }
  }
  //Vertical lines match
  for (int i = 0; i < 3; ++i) {
    if (grid[0][i] != Entry::EMPTY && grid[0][i] == grid[1][i] && grid[1][i] == grid[2][i]) {
      gameOver = true;
      return true;
    }
  }
  //Diagonal lines match
  if (grid[0][0] != Entry::EMPTY && grid[0][0] == grid[1][1] && grid[1][1] == grid[2][2]) {
    gameOver = true;
    return true;
  }
  if (grid[0][2] != Entry::EMPTY && grid[0][2] == grid[1][1] && grid[1][1] == grid[2][0]) {
    gameOver = true;
    return true;
  }
  return false;
}

GameState::Entry GameState::getCell(const int row,const int col) const {
  return grid[row][col];
}

void GameState::setCell(const int row, const int col, const GameState::Entry value) {
  grid[row][col] = value;
}

void GameState::setPlayerId(const int id) {
  playerId = id;
}

void GameState::addPlayer() {
  connectedPlayers++;
  spdlog::info("Game session: from server, Player {} connected to the game with pointer {}", connectedPlayers, static_cast<void*>(this));
  if (connectedPlayers == 2) {
    gameStarted = true;
    spdlog::info("Server Side: Game session: Game started!");
  }

}
bool GameState::hasGameStarted() {
  return gameStarted;
}
