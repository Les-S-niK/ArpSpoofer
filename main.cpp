

#include <linux/if_ether.h>

#include <cstdlib>
#include <exception>
#include <print>

#include "arp_client.hpp"
#include "include/frames.hpp"
#include "net_interfaces.hpp"

auto main() -> int {
    std::println("main");

    using network_interfaces::NetworkActiveInterface;
    auto active_iface = NetworkActiveInterface::create();
    if (not active_iface) {
        std::terminate();
    }

    ArpFrame arp_frame{};
    arp_frame.setHeaderHwAddrSize(6);
    arp_frame.setHeaderPrAddrSize(4);
    arp_frame.setHeaderHwAddrType(0x0001);
    arp_frame.setHeaderPrAddrType(0x0800);
    arp_frame.setHeaderOpcode(0x0001);
    arp_frame.setPayloadDestinationHwAddr({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
    arp_frame.setPayloadDestinationPrAddr({192, 168, 0, 1});
    arp_frame.setPayloadSourceHwAddr(active_iface->getHardwareAddr().value());
    arp_frame.setPayloadSourcePrAddr(active_iface->getProtocolAddr().value());

    auto arp_client = ArpClient::create();
    if (not arp_client) {
        std::terminate();
    }
    arp_client->sendFrame(arp_frame);
    auto response_frame = arp_client->recvResponseToFrame(arp_frame);
    if (not response_frame) {
        std::terminate();
    }
    for (u8 byte : response_frame->getPayloadSourceHwAddr()) {
        std::print("0x{:02x} ", byte);
    }
    std::println();

    return EXIT_SUCCESS;
}
