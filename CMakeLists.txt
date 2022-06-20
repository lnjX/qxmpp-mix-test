cmake_minimum_required(VERSION 3.5)

project(qxmpp-mix-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(QXmpp REQUIRED)

add_executable(qxmpp-mix-test main.cpp)
target_link_libraries(qxmpp-mix-test Qt5::Core QXmpp::QXmpp)
