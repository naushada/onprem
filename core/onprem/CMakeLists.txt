cmake_minimum_required(VERSION 3.16.3)
project(uniimage)

add_subdirectory(mongodb)

include_directories(/usr/local/include/bsoncxx/v_noabi)
include_directories(/usr/local/include/mongocxx/v_noabi)

#Adding Complier flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -g -Wall -Wextra")

include_directories(mongodb/inc)
include_directories(inc)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cc")

add_executable(uniimage ${SOURCES})

target_link_libraries(uniimage mongodbcxx)
target_link_libraries(uniimage pthread ssl crypto)

install(TARGETS uniimage
	CONFIGURATIONS Release
        RUNTIME DESTINATION /usr/local/bin
)

