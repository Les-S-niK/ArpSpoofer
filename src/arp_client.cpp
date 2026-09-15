
#include "arp_client.hpp"

#include <optional>

#include "frames.hpp"

[[nodiscard]] auto ArpClient::create() noexcept
    -> std::expected<ArpClient, Errors> {
    try {
        auto active_iface_index = getActiveInterfaceIndex();
        auto raw_socket = RawSocket::create(
            arp_proto_type, static_cast<i32>(active_iface_index.value()));
        return {ArpClient(std::move(raw_socket.value()))};
    } catch (...) {
        return std::unexpected{Errors::CanNotGetSocketFd};
    }
}
[[nodiscard]] auto ArpClient::getActiveInterfaceIndex()
    -> std::expected<u32, Errors> {
    std::expected<NetworkActiveInterface, Errors> active_iface =
        NetworkActiveInterface::create();
    if (not active_iface) {
        return std::unexpected{active_iface.error()};
    }
    std::optional<u32> active_iface_index = active_iface->getInterfaceIndex();
    if (not active_iface_index) {
        return std::unexpected{Errors::CanNotGetInterfaceFlags};
    }
    return {active_iface_index.value()};
}
ArpClient::ArpClient(RawSocket raw_socket)
    : _raw_socket(std::move(raw_socket)) {}

auto ArpClient::sendFrame(ArpFrame frame) const noexcept -> void {
    EthernetFrame<arp_frame_size> ether_frame =
        generateEthernetFrameOverArp(frame);
    _raw_socket.sendData<EthernetFrame<arp_frame_size>,
                         EthernetFrame<0>::header_size, arp_frame_size>(
        ether_frame);
}
auto ArpClient::recvFrame() const noexcept -> std::generator<ArpFrame> {
    while (auto received_frame = _raw_socket.recvData<frame_size>()) {
        co_yield getArpFromEthernetFrame(received_frame.value());
    }
}
[[nodiscard]] auto ArpClient::recvResponseToFrame(ArpFrame frame) const noexcept
    -> std::optional<ArpFrame> {
    sendFrame(frame);
    for (const auto& received_frame : recvFrame()) {
        if (checkIsResponse(frame, received_frame)) {
            return received_frame;
        }
        return std::nullopt;
    }
    return std::nullopt;
}
[[nodiscard]] auto ArpClient::checkIsResponse(ArpFrame request,
                                              ArpFrame response) -> bool {
    return (response.getHeaderOpcode() == ArpFrame::response_opcode and
            request.getPayloadSourceHwAddr() ==
                response.getPayloadDestinationHwAddr() and
            request.getPayloadSourcePrAddr() ==
                response.getPayloadDestinationPrAddr());
}

[[nodiscard]] constexpr auto ArpClient::getArpFromEthernetFrame(
    std::array<u8, frame_size> ethernet_frame) -> ArpFrame {
    std::array<u8, ArpFrame::header_size> arp_header{};
    std::array<u8, ArpFrame::payload_size> arp_payload{};

    std::copy_n(
        std::next(ethernet_frame.begin(), EthernetFrame<0>::header_size),
        ArpFrame::header_size, arp_header.begin());
    std::copy_n(
        std::next(ethernet_frame.begin(),
                  EthernetFrame<0>::header_size + ArpFrame::header_size),
        ArpFrame::payload_size, arp_payload.begin());

    ArpFrame arp_frame{};
    arp_frame.setHeader(arp_header);
    arp_frame.setPayload(arp_payload);

    return arp_frame;
}
[[nodiscard]] constexpr auto ArpClient::generateEthernetFrameOverArp(
    ArpFrame arp_frame) -> EthernetFrame<arp_frame_size> {
    EthernetFrame<arp_frame_size> ether_frame{};
    ether_frame.setHeaderSourceHwAddr(arp_frame.getPayloadSourceHwAddr());
    ether_frame.setHeaderDestinationHwAddr(
        arp_frame.getPayloadDestinationHwAddr());
    ether_frame.setHeaderPayloadType(arp_proto_type);
    ether_frame.setPayload(arp_frame.toU8Array());

    return ether_frame;
}
