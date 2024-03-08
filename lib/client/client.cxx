#include "client.hpp"
#include <csignal>
#include <unistd.h>
#include <spdlog/spdlog.h>

// TODO: don't use volatile
volatile sig_atomic_t signalReceived = 0;
// constexpr int MAX_C_STR_INT_SIZE = 12;
int constexpr SHARED_MEM_SIZE = sizeof(GameState);
Client::Client(const char* serverIP, int serverPort) {
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cout << "\n Socket creation error \n";
    exit(EXIT_FAILURE);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(serverPort);

  if(inet_pton(AF_INET, serverIP, &serv_addr.sin_addr)<=0) {
    std::cout << "\nInvalid address/ Address not supported \n";
    exit(EXIT_FAILURE);
  }
}

// Signal handler function
void signalHandler(int signum) {
  signalReceived = 1;
  spdlog::info("Interrupt signal ({}) received.", signum);
  // exit(signum);
}

void Client::connectToServer() {
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cout << "\nConnection Failed \n";
    exit(EXIT_FAILURE);
  }

  spdlog::info("Client Side: If no failed seen, Client connected");
}

void Client::attachToSharedMemory() {
  // read the shared memory key sent from the server

  read(sock, &sharedMemoryKey, sizeof(key_t));

  if ((shmid = shmget(sharedMemoryKey, SHARED_MEM_SIZE, 0666)) < 0) {
    std::cout << "Failed to attach to shared memory\n";
    exit(EXIT_FAILURE);
  }

  // Attach to the shared memory segment
  void* shared_memory = shmat(shmid, nullptr, 0);
  if (shared_memory == (void*) -1) {
    std::cout << "Failed to attach to shared memory\n";
    exit(EXIT_FAILURE);
  }

  // Cast the shared memory to a GameState object
  sharedGameState = std::make_shared<GameState>(*static_cast<GameState*>(shared_memory));
  spdlog::info("Client fd {} : key {} received, maps to shm id {} ", sock , sharedMemoryKey, shmid);
  spdlog::info("Client: connected to game with pointer address id {} ", static_cast<void*>(sharedGameState.get()));

}

void Client::getPlayerId() {
  playerId = sharedGameState->playerId;
}

void Client::waitForGameStart() {
  spdlog::info("Waiting for the game to start...");
  while (!sharedGameState->hasGameStarted()) {
    sleep(10);
    spdlog::info("connected players to the game are {}", sharedGameState->connectedPlayers);
  }
}

bool Client::isPlayerTurn() const {
  return (playerId == 0 && sharedGameState->player0Turn) || (playerId == 1 && !sharedGameState->player0Turn);
}

void Client::makeMove(const std::string& input) const {
  int x = input[0] - 'A';
  int y = input[1] - '1';

  // std::scoped_lock lock(sharedGameState->mtx);

  if (isPlayerTurn() && sharedGameState->makeMove(x, y)) {
    std::cout << "Player " << playerId + 1 << " made a move at " << input << std::endl;
  } else {
    std::cout << "Invalid move. It's not your turn or the cell is already taken." << std::endl;
  }
}

void Client::playGame() const {
  // Game loop for client interaction
  while (!sharedGameState->gameOver) {
    if (isPlayerTurn()) {
      std::string input;
      std::cout << "Player " << playerId + 1 << ", enter cell number (A1-C3): ";
      std::cin >> input;
      makeMove(input);
    } else {
      std::cout << "Waiting for the other player's move..." << std::endl;
      sleep(1); // Simulate waiting for the other player
    }
  }
}

Client::~Client() {
  shmdt(shared_memory);
}

int main() {
  Client client("127.0.0.1", 5000);
  client.connectToServer();

  client.attachToSharedMemory();
  client.getPlayerId();
  client.waitForGameStart();
  client.playGame();
  return 0;
}