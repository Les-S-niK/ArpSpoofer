
#include <gtest/gtest.h>

#include "raw_socket.hpp"

TEST(RawSocketTest, FabricMethodWorksCorrectly) {
    auto socket = RawSocket::create();

    ASSERT_TRUE(socket.has_value());
    ASSERT_NE(socket.value().getSockFd(), -1);
}
