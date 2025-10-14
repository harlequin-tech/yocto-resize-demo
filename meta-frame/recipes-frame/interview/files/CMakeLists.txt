cmake_minimum_required(VERSION 3.15)
project(frame)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
find_package(ZeroMQ REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} ${ZeroMQ_INCLUDE_DIRS} include)

add_library(${PROJECT_NAME} SHARED frame.cpp)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${ZeroMQ_LIBS})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION 1.0.0
        SOVERSION 1
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
