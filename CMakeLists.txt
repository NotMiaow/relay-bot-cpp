cmake_minimum_required(VERSION 3.7)
project(relay_bot)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(lib/discordpp)          # discordpp
add_subdirectory(lib/rest-beast)         # discordpp-rest-beast
add_subdirectory(lib/websocket-beast)    # discordpp-websocket-beast
add_subdirectory(lib/plugin-overload)    # discordpp-plugin-overload
add_subdirectory(lib/plugin-responder)   # discordpp-plugin-new

set(SOURCE_FILES main.cc)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost 1.72.0 REQUIRED system)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

INCLUDE_DIRECTORIES(
        ${Boost_INCLUDE_DIRS}
        ${discordpp_SOURCE_DIR}
        ${discordpp-rest-beast_SOURCE_DIR}
        ${discordpp-websocket-beast_SOURCE_DIR}
        ${discordpp-plugin-overload_SOURCE_DIR}
        ${discordpp-plugin-responder_SOURCE_DIR}
        ${json_SOURCE_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/_lib/queue
        ${PROJECT_SOURCE_DIR}/_lib/basic_lib
        ${PROJECT_SOURCE_DIR}/headers
)

add_subdirectory(source)

TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        ${Boost_LIBRARIES}
        discordpp
        discordpp-rest-beast
        discordpp-websocket-beast
        Threads::Threads
        crypto
        ssl
        RELAY_BOT_SOURCE
)
