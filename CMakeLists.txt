cmake_minimum_required(VERSION 2.8.9)

project (cakar-tr069-cpe)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "-O0 -ggdb -Wall -Werror")

add_executable(cakar-tr069-cpe
  cakar-cpe.c
  common.c
  datamodel.c
  http_generator.c
  http_parser.c
  xml_generator.c
  connection.c
  external/sqlite-3.36.0/sqlite3.c
  xml_parser.c
  inform.c
  inform_queue.c
  http_connection_request.c
  )

TARGET_LINK_LIBRARIES(cakar-tr069-cpe pthread dl)

