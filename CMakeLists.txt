create_executable(
        NAME
        epoll.server.bin

        SOURCES
        src/main.cxx
        lib/epoll_server/epoll_server.cxx
        lib/game_state/game_state.cxx
        lib/shared_mem/shm_manager.cxx
        lib/game_session/game_session.cxx


        DEPENDS_ON
        dimcli.dimcli
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        Boost::headers
        Boost::json


        COMPILE_FEATURES
        cxx_std_23
)

create_executable(
        NAME
        game.client.bin

        SOURCES
        lib/game_state/game_state.cxx
        lib/shared_mem/shm_manager.cxx
        lib/client/client.cxx
        lib/game_session/game_session.cxx

        DEPENDS_ON
        dimcli.dimcli
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        Boost::headers
        Boost::json


        COMPILE_FEATURES
        cxx_std_23
)