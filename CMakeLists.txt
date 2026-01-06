cmake_minimum_required(VERSION 3.10)
project(todo_api)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lws2_32")
endif()

add_executable(todo_api
    main.cpp
    Task.cpp
    TaskManager.cpp
    HTTPServer.cpp
)

if(WIN32)
    target_link_libraries(todo_api ws2_32)
else()
    target_link_libraries(todo_api pthread)
endif()