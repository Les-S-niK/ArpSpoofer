
#ifndef INCLUDE_INCLUDE_FRAMES_HPP_
#define INCLUDE_INCLUDE_FRAMES_HPP_

#include <algorithm>
#include <array>
#include <concepts>

#include "number_usings.hpp"

template <typename Frame, u16 FrameSize, u16 HeaderSize, u16 PayloadSize>
concept frame_trait = requires(Frame frame, std::array<u8, HeaderSize> header,
                               std::array<u8, PayloadSize> payload) {
    frame.setHeader(header);
    frame.setPayload(payload);
    { frame.getHeader() } -> std::same_as<std::array<u8, HeaderSize>>;
    { frame.getPayload() } -> std::same_as<std::array<u8, PayloadSize>>;
    { frame.toU8Array() } -> std::same_as<std::array<u8, FrameSize>>;
};

template <u16 HeaderSize, u16 PayloadSize>
class NetworkFrame {
    using header_t = std::array<u8, HeaderSize>;
    using payload_t = std::array<u8, PayloadSize>;
    using frame_t = std::array<u8, HeaderSize + PayloadSize>;

   public:
    auto setHeader(header_t header) noexcept -> NetworkFrame& {
        _header = header;
        return *this;
    }
    auto setPayload(payload_t payload) noexcept -> NetworkFrame& {
        _payload = payload;
        return *this;
    }
    [[nodiscard]] auto getHeader() const noexcept -> header_t {
        return _header;
    }
    [[nodiscard]] auto getPayload() const noexcept -> payload_t {
        return _payload;
    }
    [[nodiscard]] auto toU8Array() const noexcept -> frame_t {
        frame_t result{};
        std::copy(_header.cbegin(), _header.cend(), result.begin());
        std::copy(_payload.cbegin(), _payload.cend(),
                  std::next(result.begin(), HeaderSize));
        return result;
    }

   private:
    std::array<u8, HeaderSize> _header{};
    std::array<u8, PayloadSize> _payload{};
};

constexpr u8 ethernet_header_size = 14;
template <u16 PayloadSize>
class EthernetFrame : public NetworkFrame<ethernet_header_size, PayloadSize> {
   public:
    static constexpr u8 header_size = ethernet_header_size;
};

constexpr u8 arp_header_size = 8;
constexpr u8 arp_payload_size = 20;
class ArpFrame : public NetworkFrame<arp_header_size, arp_payload_size> {
   public:
    static constexpr u8 header_size = arp_header_size;
    static constexpr u8 payload_size = arp_payload_size;
};

// frame_trait<NetworkFrame<header_size, payload_size>, frame_size, header_size,
//   payload_size>
static_assert(frame_trait<NetworkFrame<14, 28>, 42, 14, 28>,  // NOLINT.
              "<NetworkFrame> does not satisfy <frame_trait>");
// frame_trait<EthernetFrame<payload_size>, frame_size, header_size,
//   payload_size>
static_assert(frame_trait<EthernetFrame<28>, 42, 14, 28>,  // NOLINT.
              "<EthernetFrame> does not satisfy <frame_trait>");
// frame_trait<ArpFrame, frame_size, header_size,
//   payload_size>
static_assert(frame_trait<ArpFrame, 28, 8, 20>,  // NOLINT.
              "<ArpFrame> does not satisfy <frame_trait>");

#endif  // INCLUDE_INCLUDE_FRAMES_HPP_
