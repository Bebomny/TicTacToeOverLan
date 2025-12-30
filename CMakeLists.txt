cmake_minimum_required(VERSION 4.0)
project(TicTacToeOverLan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)

set(CMAKE_EXE_LINKER_FLAGS "-static")

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(TicTacToeOverLan src/main.cpp
        src/common/GameDefinitions.h
        src/common/NetworkProtocol.h
        src/client/GameClient.cpp
        src/client/GameClient.h
        src/client/NetworkManager.cpp
        src/client/NetworkManager.h
        src/common/Utils.h
        src/server/InternalGameServer.cpp
        src/server/InternalGameServer.h
        src/server/ClientContext.h
        src/server/ServerUtils.cpp
        src/server/ServerUtils.h
        src/client/ui/ButtonWidget.cpp
        src/client/ui/ButtonWidget.h
        src/client/ui/ButtonBuilder.cpp
        src/client/ui/ButtonBuilder.h)

target_link_libraries(TicTacToeOverLan PRIVATE Ws2_32)
target_link_libraries(TicTacToeOverLan PRIVATE SFML::Graphics)
