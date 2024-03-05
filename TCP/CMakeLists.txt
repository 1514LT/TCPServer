cmake_minimum_required(VERSION 3.10)
project(Server)

set(CMAKE_CXX_STANDARD 11)
set(PTHREAD_INCLUDE  ${CMAKE_CURRENT_SOURCE_DIR}/Poco/include)
set(PTHREAD_LIB ${CMAKE_CURRENT_SOURCE_DIR}/Poco/lib)

set(CPU_LIB ${TOP_DIR}/get_cpu_info/inc)
set(HDD_LIB ${TOP_DIR}/get_hdd_info/inc)
set(MEMORY_LIB ${TOP_DIR}/get_memory_info/inc)

set(CPU_SRC ${TOP_DIR}/get_cpu_info/src)
set(HDD_SRC ${TOP_DIR}/get_hdd_info/src)
set(MEMORY_SRC ${TOP_DIR}/get_memory_info/src)

find_package(Threads REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PTHREAD_INCLUDE} ${CPU_LIB} ${HDD_LIB} ${MEMORY_LIB})
link_directories(Server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PTHREAD_LIB})
add_executable(Server TCPServer.cpp
                      FTPServer.cpp
                      Base.cpp
                      Socket.cpp
                      ${CPU_SRC}/cpu-info.cpp
                      ${HDD_SRC}/hdd-info.cpp
                      ${MEMORY_SRC}/mem-info.cpp
              )
# 链接pthread库
target_link_libraries(Server PRIVATE  
                                    libpthread.so 
                                    ${CMAKE_DL_LIBS}
                                    libPocoNet.so
                                    libPocoUtil.so
                                    libPocoJSON.so
                                    libPocoXML.so
                                    libPocoFoundation.so
                                    libPocoData.so
                                    )


