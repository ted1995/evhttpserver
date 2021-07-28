
include_directories(
    ${CMAKE_SOURCE_DIR}/
)

link_directories(
    ${CMAKE_SOURCE_DIR}/
)

set(SERVER_SRC
    main.cpp
    webserver.cpp
    workers.cpp
    handler.cpp
    sqlpool.cpp
)

ADD_EXECUTABLE(http-server ${SERVER_SRC})
target_link_libraries(http-server
    glog
    event
    pthread
    mysqlclient
)

