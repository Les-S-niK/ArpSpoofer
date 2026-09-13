

#include <linux/if_ether.h>

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <print>

#include "include/frames.hpp"
#include "net_interfaces.hpp"
#include "sockets.hpp"

auto main() -> int {
    std::println("main");

    using network_interfaces::NetworkActiveInterface;

    auto active_iface = NetworkActiveInterface::create();
    if (not active_iface) {
        std::terminate();
    }
    auto raw_socket =
        RawSocket::create(ETH_P_ARP, active_iface->getInterfaceIndex().value());
    if (not raw_socket) {
        std::terminate();
    }

    ArpFrame arp_frame{};
    arp_frame.setHeaderHwAddrSize(6);
    arp_frame.setHeaderPrAddrSize(4);
    arp_frame.setHeaderHwAddrType(0x0001);
    arp_frame.setHeaderPrAddrType(0x0800);
    arp_frame.setHeaderOpcode(0x0001);
    arp_frame.setPayloadDestinationHwAddr({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
    arp_frame.setPayloadDestinationPrAddr({192, 168, 0, 200});
    arp_frame.setPayloadSourceHwAddr(active_iface->getHardwareAddr().value());
    arp_frame.setPayloadSourcePrAddr(active_iface->getProtocolAddr().value());

    EthernetFrame<ArpFrame::header_size + ArpFrame::payload_size> ether_frame{};
    ether_frame.setHeaderSourceHwAddr(active_iface->getHardwareAddr().value());
    ether_frame.setHeaderDestinationHwAddr(
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
    ether_frame.setHeaderPayloadType(0x0806);
    ether_frame.setPayload(arp_frame.toU8Array());

    raw_socket->sendData<EthernetFrame<28>, 14, 28>(ether_frame);

    auto buffer = raw_socket->recvData<42>();
    std::println("{}", buffer.value());

    return EXIT_SUCCESS;
}
