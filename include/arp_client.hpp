#ifndef INCLUDE_INCLUDE_ARP_ACTIONS_HPP_
#define INCLUDE_INCLUDE_ARP_ACTIONS_HPP_

#include <expected>
#include <generator>
#include <optional>

#include "frames.hpp"
#include "net_interfaces.hpp"
#include "sockets.hpp"

class ArpClient {
    using NetworkActiveInterface = network_interfaces::NetworkActiveInterface;
    using Errors = network_interfaces::Errors;

    constexpr static u8 arp_frame_size =
        ArpFrame::header_size + ArpFrame::payload_size;
    constexpr static u8 frame_size =
        EthernetFrame<0>::header_size + arp_frame_size;
    constexpr static u16 arp_proto_type = 0x0806;

   public:
    ArpClient() = delete;

    [[nodiscard]] static auto create() noexcept
        -> std::expected<ArpClient, Errors>;
    auto sendFrame(ArpFrame frame) const noexcept -> void;
    [[nodiscard]] auto recvFrame() const noexcept -> std::generator<ArpFrame>;
    [[nodiscard]] auto recvResponseToFrame(ArpFrame frame) const noexcept
        -> std::optional<ArpFrame>;

   private:
    RawSocket _raw_socket;

    explicit ArpClient(RawSocket raw_socket);

    [[nodiscard]] static auto checkIsResponse(ArpFrame request,
                                              ArpFrame response) -> bool;
    [[nodiscard]] static auto getActiveInterfaceIndex()
        -> std::expected<u32, Errors>;
    [[nodiscard]] static constexpr auto getArpFromEthernetFrame(
        std::array<u8, frame_size> ethernet_frame) -> ArpFrame;
    [[nodiscard]] static constexpr auto generateEthernetFrameOverArp(
        ArpFrame arp_frame) -> EthernetFrame<arp_frame_size>;
};

#endif  // INCLUDE_INCLUDE_ARP_ACTIONS_HPP_
