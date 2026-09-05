
#include <gtest/gtest.h>

#include <array>
#include <print>

#include "frames.hpp"

constexpr u16 default_byte = 0xff;

namespace {
template <u64 Size>
constexpr auto generateArraySizeN(u8 value) -> std::array<u8, Size> {
    std::array<u8, Size> array{};
    memset(array.data(), value, array.size());
    return array;
}

template <typename Iter>
constexpr auto compareArrayElements(Iter begin, Iter end, u8 value) -> bool {
    while (begin != end) {
        if (*begin != value) {
            return false;
        }
        std::advance(begin, 1);
    }
    std::println();
    return true;
}
}  // namespace

TEST(FramesTest, NetworkFrameSetGetHeaderWorksCorrectly) {
    constexpr u16 payload_size = 1;
    EthernetFrame<payload_size> ethernet_frame;

    ethernet_frame.setHeader(
        generateArraySizeN<ethernet_frame.header_size>(default_byte));

    auto res = ethernet_frame.getHeader();
    ASSERT_TRUE(compareArrayElements(res.begin(), res.end(), default_byte));
}

TEST(FramesTest, NetworkFrameSetGetPayloadWorksCorrectly) {
    constexpr u16 payload_size = 100;
    EthernetFrame<payload_size> ethernet_frame;

    ethernet_frame.setPayload(generateArraySizeN<payload_size>(default_byte));

    auto res = ethernet_frame.getPayload();
    ASSERT_TRUE(compareArrayElements(res.begin(), res.end(), default_byte));
}

TEST(FramesTest, NetworkFrameTou8ArrayWorksCorrectly) {
    ArpFrame arp_frame;
    constexpr u8 header_byte = 0xCC;
    constexpr u8 payload_byte = 0xAA;

    arp_frame.setHeader(generateArraySizeN<ArpFrame::header_size>(header_byte));
    arp_frame.setPayload(
        generateArraySizeN<ArpFrame::payload_size>(payload_byte));

    auto res = arp_frame.toU8Array();
    ASSERT_TRUE(compareArrayElements(
        res.begin(), std::next(res.begin(), ArpFrame::header_size),
        header_byte));
    ASSERT_TRUE(
        compareArrayElements(std::next(res.begin(), ArpFrame::header_size),
                             res.end(), payload_byte));
}
