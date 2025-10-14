cmake_minimum_required(VERSION 3.15)
project(framePublisher)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
find_package(ZeroMQ REQUIRED)

message(INFO "ZeroMQ_INCLUDE_DIRS: $ZeroMQ_INCLUDE_DIRS")

include_directories(${OpenCV_INCLUDE_DIRS} ${ZeroMQ_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} framePublisher.cpp)

target_link_libraries(${PROJECT_NAME} -lframe ${OpenCV_LIBS} ${ZeroMQ_LIBS} -lzmq)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
