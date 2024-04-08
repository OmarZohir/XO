#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <csignal> // Include for signal handling
#include <sys/types.h>
#include <sys/wait.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "epoll_server.hpp"
using json = nlohmann::json;

static constexpr int MAX_EVENTS = 20;
static constexpr int PORT = 5000;

EpollServer::EpollServer() : server_fd(-1), epoll_fd(-1) {
    // Initialize game session and shared memory manager for the server
    auto game_session = std::make_shared<GameSession>();
    gameSessions[0] = game_session;
}

EpollServer::~EpollServer() {
    if (epoll_fd != -1)
        close(epoll_fd);

    if (server_fd != -1)
        close(server_fd);
}
void EpollServer::createEpoll() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Error creating socket\n";
        throw std::runtime_error("Socket creation failed");
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Set SO_REUSEADDR option
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Setsockopt failed");
        close(server_fd);
    }

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Bind failed with error code: " << errno << std::endl;
        throw std::runtime_error("Bind failed");
    }

    if (listen(server_fd, 5) == -1) {
        std::cerr << "Listen failed\n";
        throw std::runtime_error("Listen failed");
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create error");
        exit(EXIT_FAILURE);
    }

    // add server socket to epoll event list
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        perror("epoll_ctl error for serverSocket");
        exit(EXIT_FAILURE);
    }

    events.resize(MAX_EVENTS);
}

void EpollServer::addToEpoll(const int clientSocket) {
    struct epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = clientSocket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &event) == -1) {
        perror("epoll_ctl error");
        exit(EXIT_FAILURE);
    }

    spdlog::info("Client with fd {} added to epoll list", clientSocket);
}

void EpollServer::handleNewClientConnection() {
    int clientSocket;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    clientSocket = accept(server_fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
        perror("Accept failed");
        return;
    }

    spdlog::info("New client connected with fd {}", clientSocket);

    addToEpoll(clientSocket); // Add the new client socket to epoll for event monitoring
    // Create shared memory and attach game state
    addClientToGame(clientSocket);
}

void EpollServer::handleIncomingData(int socketFd) {
    char buffer[1024] = {};

    long bytesReceived = recv(socketFd, buffer, sizeof(buffer), 0);
    if (bytesReceived == -1) {
        perror("Recv failed");
    } else if (bytesReceived == 0) {
        // Client disconnected
        std::cout << "Client disconnected" << std::endl;
        close(socketFd);
    } else {
        // Process received data
        std::cout << "Received data from client: " << buffer << std::endl;
    }
}
void EpollServer::runServer() {
    while (true) {
        int num_events = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
        handleEvents(num_events);
    }
}

void EpollServer::handleEvents(int num_events) {
    for (int i = 0; i < num_events; ++i) {
        if (events[i].data.fd == server_fd) {
            acceptClientConnection();
        } else {
            receiveClientData(i);
        }
    }
}

void EpollServer::runEventLoop() {
    while (true) {
        int numEvents = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
        if (numEvents == -1) {
            perror("epoll_wait error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < numEvents; ++i) {
            if (events[i].data.fd == server_fd) {
                handleNewClientConnection();
            } else if (events[i].events & EPOLLIN) {
                handleIncomingData(events[i].data.fd);
            }
        }
    }
}

void EpollServer::addClientToGame(const int client_socket) {
    if (gameSessions.empty() || gameSessions.rbegin()->second->connectedClients == 2) {
        // Create a new game session if no sessions exist or the current session is full
        auto game_session = std::make_shared<GameSession>();
        gameSessions[gameSessions.size()] = game_session;
    }

    // Add the client to the latest game session
    auto game_session = gameSessions.rbegin()->second;
    //  increment numbner of players in the game session
    //TODO: CLEAR THE CODE STRUCTURE, THIS IS UGLY!!!
    {
        std::shared_lock lock(game_session->sharedMem->getMutex());
        game_session->sharedMem->getGameState()->addPlayer();

    }


    // Send the shared memory key to the client
    int key_server = game_session->sharedMem->get_key();
    write(client_socket, &key_server, sizeof(key_server));
    spdlog::info("Server: key {} sent to client {}", key_server, client_socket);
    spdlog::info("Server: key {} maps to shm id {}", key_server, game_session->sharedMem->get_shm_id());

    //TODO: UGLY UGLY UGLY, DO PROPER SEPARATION OF CONCERNS
    spdlog::info("Server: Client {} is supposed to be in the session, {} players connected to the game", client_socket, game_session->sharedMem->getGameState()->connectedPlayers);
}

void EpollServer::acceptClientConnection() {
    struct sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    struct epoll_event client_event{};
    client_event.events = EPOLLIN | EPOLLONESHOT;
    client_event.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);
    spdlog::info("Client connected, and registered in epoll list");

    // Create shared memory and attach game state
    addClientToGame(client_fd);
}

void EpollServer::receiveClientData(int event_index) {
    char buffer[1024];
    int bytes_received = recv(events[event_index].data.fd, buffer, sizeof(buffer), 0);

    if (bytes_received <= 0) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[event_index].data.fd, NULL);
        close(events[event_index].data.fd);
    } else {
        // Implement API data retrieval and send to client

        // Example: Fetch data from APIs and send to client
        // send(events[event_index].data.fd, api_data.c_str(), api_data.size(), 0);
    }
}

void EpollServer::startServer() {
    createEpoll();
    runEventLoop();
    // runServer();
}