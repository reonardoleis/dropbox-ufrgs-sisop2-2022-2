#pragma once

enum SocketError : int {
    READ_HEADER_ERROR = 1,
    READ_PAYLOAD_ERROR,
    WRITE_ERROR,
    CONNECT_ERROR,
    BIND_ERROR
};

enum WatcherError : int {
    OPEN_ERROR = 6,
    FILE_WRITE_ERROR,
    READ_ERROR,
    READ_DIR_ERROR
};
