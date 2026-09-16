
#ifndef INCLUDE_INCLUDE_ARP_SPOOFER_HPP_
#define INCLUDE_INCLUDE_ARP_SPOOFER_HPP_

#include <expected>

#include "arp_client.hpp"
#include "frames.hpp"
#include "net_interfaces.hpp"

class ArpSpoofer {
    using NetworkActiveInterface = network_interfaces::NetworkActiveInterface;

   public:
    [[nodiscard]] static auto create(ArpFrame::praddr_t target,
                                     ArpFrame::praddr_t node) noexcept
        -> std::expected<ArpSpoofer, network_interfaces::Errors>;
    [[noreturn]] auto startSpoofing(u32 interval) -> void;

   private:
    ArpFrame::praddr_t _target_addr;
    ArpFrame::praddr_t _node_addr;
    ArpClient _arp_client;
    NetworkActiveInterface _active_iface;

    ArpSpoofer(ArpFrame::praddr_t target, ArpFrame::praddr_t node,
               ArpClient&& client,
               NetworkActiveInterface&& active_iface) noexcept;

    [[nodiscard]] auto generateSpoofingArpResponse(ArpFrame target_response)
        -> ArpFrame;
    [[nodiscard]] auto generateBroadcastArpRequest() -> ArpFrame;
};

#endif  // INCLUDE_INCLUDE_ARP_SPOOFER_HPP_
