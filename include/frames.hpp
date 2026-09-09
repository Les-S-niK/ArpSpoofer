
#ifndef INCLUDE_INCLUDE_FRAMES_HPP_
#define INCLUDE_INCLUDE_FRAMES_HPP_

#include <algorithm>
#include <array>
#include <concepts>

#include "number_usings.hpp"

template <typename Iter>
constexpr auto u16ToU8Array(Iter begin, u16 number) noexcept -> void {
    if (begin == nullptr or std::next(begin, 1) == nullptr) [[unlikely]] {
        std::terminate();
    }
    constexpr u8 byte_size = 0x08;
    constexpr u8 max_byte = 0xFF;
    *begin = number >> byte_size;
    *(std::next(begin, 1)) = number & max_byte;
}

[[nodiscard]] constexpr auto u16ToU8Array(u16 number) noexcept
    -> std::array<u8, 2> {
    std::array<u8, 2> result{};
    u16ToU8Array(result.begin(), number);

    return result;
}

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
    static constexpr u8 hwaddr_size = 6;
    static constexpr u8 praddr_size = 4;

   public:
    using header_t = std::array<u8, HeaderSize>;
    using payload_t = std::array<u8, PayloadSize>;
    using frame_t = std::array<u8, HeaderSize + PayloadSize>;
    using hwaddr_t = std::array<u8, hwaddr_size>;
    using praddr_t = std::array<u8, praddr_size>;

    constexpr auto setHeader(header_t header) noexcept -> NetworkFrame&;
    constexpr auto setPayload(payload_t payload) noexcept -> NetworkFrame&;
    [[nodiscard]] constexpr auto getHwAddrSize() const noexcept -> u16;
    [[nodiscard]] constexpr auto getPrAddrSize() const noexcept -> u16;
    [[nodiscard]] constexpr auto getHeader() const noexcept -> header_t;
    [[nodiscard]] constexpr auto getPayload() const noexcept -> payload_t;
    [[nodiscard]] constexpr auto toU8Array() const noexcept -> frame_t;

   protected:
    [[nodiscard]] constexpr auto getHeaderRef() noexcept -> header_t&;
    [[nodiscard]] constexpr auto getPayloadRef() noexcept -> payload_t&;

   private:
    std::array<u8, HeaderSize> _header{};
    std::array<u8, PayloadSize> _payload{};
};

constexpr u8 ethernet_header_size = 14;
template <u16 PayloadSize>
class EthernetFrame : public NetworkFrame<ethernet_header_size, PayloadSize> {
    static constexpr u8 source_hwaddr_offset = 6;
    static constexpr u8 payload_type_offset = 12;

   public:
    static constexpr u8 header_size = ethernet_header_size;

    constexpr auto setHeaderDestinationHwAddr(
        EthernetFrame::hwaddr_t hwaddr) noexcept -> EthernetFrame&;
    constexpr auto setHeaderSourceHwAddr(
        EthernetFrame::hwaddr_t hwaddr) noexcept -> EthernetFrame&;
    constexpr auto setHeaderPayloadType(u16 payload_type) noexcept
        -> EthernetFrame&;
};

constexpr u8 arp_header_size = 8;
constexpr u8 arp_payload_size = 20;
class ArpFrame : public NetworkFrame<arp_header_size, arp_payload_size> {
   public:
    static constexpr u8 header_size = arp_header_size;
    static constexpr u8 payload_size = arp_payload_size;

    static constexpr u8 header_hw_type_offset = 0;
    static constexpr u8 header_pr_type_offset = 2;
    static constexpr u8 header_hw_size_offset = 4;
    static constexpr u8 header_pr_size_offset = 5;
    static constexpr u8 header_opcode_offset = 6;

    static constexpr u8 payload_src_hwaddr_offset = 0;
    static constexpr u8 payload_src_praddr_offset = 6;
    static constexpr u8 payload_dst_hwaddr_offset = 10;
    static constexpr u8 payload_dst_praddr_offset = 16;

    constexpr auto setHeaderHwAddrType(u16 hwaddr_type) noexcept -> ArpFrame&;
    constexpr auto setHeaderPrAddrType(u16 praddr_type) noexcept -> ArpFrame&;
    constexpr auto setHeaderHwAddrSize(u8 hwaddr_size) noexcept -> ArpFrame&;
    constexpr auto setHeaderPrAddrSize(u8 praddr_size) noexcept -> ArpFrame&;
    constexpr auto setHeaderOpcode(u16 opcode) noexcept -> ArpFrame&;

    constexpr auto setPayloadSourceHwAddr(
        ArpFrame::hwaddr_t src_hwaddr) noexcept -> ArpFrame&;
    constexpr auto setPayloadSourcePrAddr(
        ArpFrame::praddr_t src_praddr) noexcept -> ArpFrame&;
    constexpr auto setPayloadDestinationHwAddr(
        ArpFrame::hwaddr_t dst_hwaddr) noexcept -> ArpFrame&;
    constexpr auto setPayloadDestinationPrAddr(
        ArpFrame::praddr_t dst_praddr) noexcept -> ArpFrame&;
};

template <u16 HeaderSize, u16 PayloadSize>
constexpr auto NetworkFrame<HeaderSize, PayloadSize>::setHeader(
    header_t header) noexcept -> NetworkFrame& {
    _header = header;
    return *this;
}
template <u16 HeaderSize, u16 PayloadSize>
constexpr auto NetworkFrame<HeaderSize, PayloadSize>::setPayload(
    payload_t payload) noexcept -> NetworkFrame& {
    _payload = payload;
    return *this;
}
template <u16 HeaderSize, u16 PayloadSize>
[[nodiscard]] constexpr auto
NetworkFrame<HeaderSize, PayloadSize>::getHwAddrSize() const noexcept -> u16 {
    return hwaddr_size;
}
template <u16 HeaderSize, u16 PayloadSize>
[[nodiscard]] constexpr auto
NetworkFrame<HeaderSize, PayloadSize>::getPrAddrSize() const noexcept -> u16 {
    return praddr_size;
}
template <u16 HeaderSize, u16 PayloadSize>
[[nodiscard]] constexpr auto NetworkFrame<HeaderSize, PayloadSize>::getHeader()
    const noexcept -> header_t {
    return _header;
}
template <u16 HeaderSize, u16 PayloadSize>
[[nodiscard]] constexpr auto NetworkFrame<HeaderSize, PayloadSize>::getPayload()
    const noexcept -> payload_t {
    return _payload;
}
template <u16 HeaderSize, u16 PayloadSize>
[[nodiscard]] constexpr auto NetworkFrame<HeaderSize, PayloadSize>::toU8Array()
    const noexcept -> frame_t {
    frame_t result{};
    std::copy(_header.cbegin(), _header.cend(), result.begin());
    std::copy(_payload.cbegin(), _payload.cend(),
              std::next(result.begin(), HeaderSize));
    return result;
}
template <u16 HeaderSize, u16 PayloadSize>
[[nodiscard]] constexpr auto
NetworkFrame<HeaderSize, PayloadSize>::getHeaderRef() noexcept -> header_t& {
    return _header;
}
template <u16 HeaderSize, u16 PayloadSize>
[[nodiscard]] constexpr auto
NetworkFrame<HeaderSize, PayloadSize>::getPayloadRef() noexcept -> payload_t& {
    return _payload;
}

template <u16 PayloadSize>
constexpr auto EthernetFrame<PayloadSize>::setHeaderDestinationHwAddr(
    EthernetFrame::hwaddr_t hwaddr) noexcept -> EthernetFrame& {
    std::copy(hwaddr.begin(), hwaddr.end(), this->getHeaderRef().begin());
    return *this;
}
template <u16 PayloadSize>
constexpr auto EthernetFrame<PayloadSize>::setHeaderSourceHwAddr(
    EthernetFrame::hwaddr_t hwaddr) noexcept -> EthernetFrame& {
    std::copy(hwaddr.begin(), hwaddr.end(),
              std::next(this->getHeaderRef().begin(), source_hwaddr_offset));
    return *this;
}
template <u16 PayloadSize>
constexpr auto EthernetFrame<PayloadSize>::setHeaderPayloadType(
    u16 payload_type) noexcept -> EthernetFrame& {
    typename EthernetFrame::header_t& header = this->getHeaderRef();
    u16ToU8Array(header.begin() + payload_type_offset, payload_type);
    return *this;
}

constexpr auto ArpFrame::setHeaderHwAddrType(u16 hwaddr_type) noexcept
    -> ArpFrame& {
    header_t& header = this->getHeaderRef();
    u16ToU8Array(std::next(header.begin(), header_hw_type_offset), hwaddr_type);
    return *this;
}
constexpr auto ArpFrame::setHeaderPrAddrType(u16 praddr_type) noexcept
    -> ArpFrame& {
    header_t& header = this->getHeaderRef();
    u16ToU8Array(std::next(header.begin(), header_pr_type_offset), praddr_type);
    return *this;
}
constexpr auto ArpFrame::setHeaderHwAddrSize(u8 hwaddr_size) noexcept
    -> ArpFrame& {
    header_t& header = this->getHeaderRef();
    header.at(header_hw_size_offset) = hwaddr_size;
    return *this;
}
constexpr auto ArpFrame::setHeaderPrAddrSize(u8 praddr_size) noexcept
    -> ArpFrame& {
    header_t& header = this->getHeaderRef();
    header.at(header_pr_size_offset) = praddr_size;
    return *this;
}
constexpr auto ArpFrame::setHeaderOpcode(u16 opcode) noexcept -> ArpFrame& {
    header_t& header = this->getHeaderRef();
    u16ToU8Array(std::next(header.begin(), header_opcode_offset), opcode);
    return *this;
}
constexpr auto ArpFrame::setPayloadSourceHwAddr(
    ArpFrame::hwaddr_t src_hwaddr) noexcept -> ArpFrame& {
    payload_t& payload = this->getPayloadRef();
    std::ranges::copy(src_hwaddr,
                      std::next(payload.begin(), payload_src_hwaddr_offset));
    return *this;
}
constexpr auto ArpFrame::setPayloadSourcePrAddr(
    ArpFrame::praddr_t src_praddr) noexcept -> ArpFrame& {
    payload_t& payload = this->getPayloadRef();
    std::ranges::copy(src_praddr,
                      std::next(payload.begin(), payload_src_praddr_offset));
    return *this;
}
constexpr auto ArpFrame::setPayloadDestinationHwAddr(
    ArpFrame::hwaddr_t dst_hwaddr) noexcept -> ArpFrame& {
    payload_t& payload = this->getPayloadRef();
    std::ranges::copy(dst_hwaddr,
                      std::next(payload.begin(), payload_dst_hwaddr_offset));
    return *this;
}
constexpr auto ArpFrame::setPayloadDestinationPrAddr(
    ArpFrame::praddr_t dst_praddr) noexcept -> ArpFrame& {
    payload_t& payload = this->getPayloadRef();
    std::ranges::copy(dst_praddr,
                      std::next(payload.begin(), payload_dst_praddr_offset));
    return *this;
}

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
