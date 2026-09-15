
#include <gtest/gtest.h>

#include <print>

#include "arp_client.hpp"
#include "frames.hpp"
#include "net_interfaces.hpp"

namespace {

class TestArpClientFixture : public testing::Test {
    static constexpr u16 ethernet_hw_addr_type = 0x0001;
    static constexpr u16 ipv4_pr_addr_type = 0x0800;
    static constexpr ArpFrame::hwaddr_t broadcast_hwaddr = {0xFF, 0xFF, 0xFF,
                                                            0xFF, 0xFF, 0xFF};

   protected:
    std::expected<ArpClient, network_interfaces::Errors> _arp_client =
        ArpClient::create();

    void SetUp() override;
    void TearDown() override {}
    [[nodiscard]] static auto generateDefaultArpFrame() noexcept -> ArpFrame;

   private:
    [[nodiscard]] static auto getActiveInterface() noexcept
        -> network_interfaces::NetworkActiveInterface;
};
void TestArpClientFixture::SetUp() { ASSERT_TRUE(_arp_client.has_value()); }
[[nodiscard]] auto TestArpClientFixture::generateDefaultArpFrame() noexcept
    -> ArpFrame {
    network_interfaces::NetworkActiveInterface active_iface =
        getActiveInterface();
    ArpFrame arp_frame{};
    arp_frame.setHeaderHwAddrSize(arp_frame.getHwAddrSize());
    arp_frame.setHeaderPrAddrSize(arp_frame.getPrAddrSize());
    arp_frame.setHeaderHwAddrType(ethernet_hw_addr_type);
    arp_frame.setHeaderPrAddrType(ipv4_pr_addr_type);
    arp_frame.setHeaderOpcode(ArpFrame::request_opcode);
    arp_frame.setPayloadDestinationHwAddr(broadcast_hwaddr);
    arp_frame.setPayloadDestinationPrAddr({192, 168, 0, 1});
    arp_frame.setPayloadSourceHwAddr(active_iface.getHardwareAddr().value());
    arp_frame.setPayloadSourcePrAddr(active_iface.getProtocolAddr().value());

    return arp_frame;
}
[[nodiscard]] auto TestArpClientFixture::getActiveInterface() noexcept
    -> network_interfaces::NetworkActiveInterface {
    using network_interfaces::NetworkActiveInterface;
    auto active_iface = NetworkActiveInterface::create();
    if (not active_iface) {
        std::terminate();
    }
    return std::move(active_iface.value());
}

TEST_F(TestArpClientFixture, TestSendAndReceiveFrameWorksCorrectly) {
    ArpFrame frame = generateDefaultArpFrame();
    _arp_client->sendFrame(frame);
    for (const auto& received_frame : _arp_client->recvFrame()) {
        ASSERT_EQ(received_frame.getHeaderOpcode(), ArpFrame::response_opcode);
        ASSERT_EQ(frame.getPayloadSourceHwAddr(),
                  received_frame.getPayloadDestinationHwAddr());
        ASSERT_EQ(frame.getPayloadSourcePrAddr(),
                  received_frame.getPayloadDestinationPrAddr());
        SUCCEED();
        return;
    }
}

}  // namespace
