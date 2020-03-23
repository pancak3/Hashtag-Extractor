cmake_minimum_required(VERSION 3.10)
project(tp)

find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

include_directories(
        include/rapidjson
        ${MPI_INCLUDE_PATH}
)
set(SOURCE_FILES main.cpp combine.cpp combine.hpp line.hpp line.cpp
        threading.cpp threading.hpp)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
ADD_DEFINITIONS(-DDEBUG)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${MPI_LIBRARIES})