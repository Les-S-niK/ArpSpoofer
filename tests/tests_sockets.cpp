
#include <gtest/gtest.h>
#include <linux/if_ether.h>

#include "frames.hpp"
#include "net_interfaces.hpp"
#include "sockets.hpp"

TEST(RawSocketTest, FabricMethodWorksCorrectly) {
    using network_interfaces::NetworkActiveInterface;
    auto active_iface = NetworkActiveInterface::create();
    auto socket =
        RawSocket::create(ETH_P_ARP, active_iface->getInterfaceIndex().value());

    ASSERT_TRUE(socket.has_value());
    ASSERT_NE(socket.value().getSockFd(), -1);
}

TEST(UdpSocketTest, FabricMethodWorksCorrectly) {
    auto socket = UdpSocket::create();

    ASSERT_TRUE(socket.has_value());
    ASSERT_NE(socket.value().getSockFd(), -1);
}

namespace {
class RawSocketSendRecvTest : public testing::Test {
    using NetworkActiveInterface = network_interfaces::NetworkActiveInterface;

   protected:
    constexpr static u16 arp_proto_type = 0x0806;
    void SetUp() override {
        ASSERT_TRUE(active_iface.has_value());

        ASSERT_TRUE(raw_socket.has_value());
        raw_socket = std::move(raw_socket.value());
    }
    void TearDown() override {}

    std::expected<NetworkActiveInterface, network_interfaces::Errors>
        active_iface = NetworkActiveInterface::create();
    std::optional<RawSocket> raw_socket = RawSocket::create(
        arp_proto_type, active_iface->getInterfaceIndex().value());
};
}  // namespace

TEST_F(RawSocketSendRecvTest, SendDataMethodReturnsValue) {
    ArpFrame test_frame{};
    auto sended_bytes =
        raw_socket
            ->sendData<ArpFrame, ArpFrame::header_size, ArpFrame::payload_size>(
                test_frame);
    ASSERT_TRUE(sended_bytes.has_value());
}

TEST_F(RawSocketSendRecvTest, RecvDataMethodReturnsValue) {
    constexpr u16 default_packet_size = 512;
    auto received = raw_socket->recvData<default_packet_size>();
    ASSERT_TRUE(received.has_value());
}
