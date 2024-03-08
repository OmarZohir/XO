#pragma once
#include <mutex>
#include <shared_mutex>

struct GameState {

    enum class Entry {
        X,
        O,
        EMPTY
    };
    GameState();

    bool gameStarted;
    Entry grid[3][3]{};
    int playerId{};
    bool player0Turn;
    bool gameOver;

    uint8_t connectedPlayers;
    bool makeMove(int x, int y);
    bool checkGameOver();
    [[nodiscard]] Entry getCell(int row, int col) const;
    void setCell(int row, int col, Entry value);
    void setPlayerId(int id);
    void addPlayer();
    bool hasGameStarted();
};
