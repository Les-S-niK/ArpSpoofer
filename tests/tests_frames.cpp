
#include <gtest/gtest.h>

#include <array>

#include "frames.hpp"
#include "utils.hpp"

constexpr u16 default_byte = utils::max_byte;

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

TEST(FramesTest, EthernetFrameSetGetDstHwaddrWorksCorrectly) {
    EthernetFrame<0> ethernet_frame;
    constexpr u16 hwaddr_size = ethernet_frame.getHwAddrSize();

    ethernet_frame.setHeaderDestinationHwAddr(
        generateArraySizeN<hwaddr_size>(default_byte));
    auto header = ethernet_frame.getHeader();
    auto hwaddr = ethernet_frame.getHeaderDestinationHwAddr();

    ASSERT_EQ(header.at(hwaddr_size + 1), 0x00);
    ASSERT_TRUE(
        compareArrayElements(hwaddr.begin(), hwaddr.end(), default_byte));
    ASSERT_TRUE(compareArrayElements(
        header.begin(), std::next(header.begin(), hwaddr_size), default_byte));
}

TEST(FramesTest, EthernetFrameSetSrcHwaddrWorksCorrectly) {
    EthernetFrame<0> ethernet_frame;
    constexpr u16 hwaddr_size = ethernet_frame.getHwAddrSize();

    ethernet_frame.setHeaderSourceHwAddr(
        generateArraySizeN<hwaddr_size>(default_byte));
    auto header = ethernet_frame.getHeader();
    auto hwaddr = ethernet_frame.getHeaderSourceHwAddr();

    ASSERT_EQ(header.at(hwaddr_size - 1), 0x00);
    ASSERT_EQ(header.at((hwaddr_size * 2) + 1), 0x00);
    ASSERT_TRUE(
        compareArrayElements(hwaddr.begin(), hwaddr.end(), default_byte));
    ASSERT_TRUE(compareArrayElements(std::next(header.begin(), hwaddr_size),
                                     std::next(header.begin(), hwaddr_size * 2),
                                     default_byte));
}

TEST(FramesTest, EthernetFrameSetGetPayloadTypeWorksCorrectly) {
    EthernetFrame<0> ethernet_frame;
    constexpr u16 hwaddr_size = ethernet_frame.getHwAddrSize();
    constexpr u16 default_payload_type = 0x0806;

    ethernet_frame.setHeaderPayloadType(default_payload_type);
    auto header = ethernet_frame.getHeader();
    u16 payload_type = ethernet_frame.getHeaderPayloadType();

    ASSERT_EQ(header.at((hwaddr_size * 2) - 1), 0x00);
    ASSERT_EQ(payload_type, default_payload_type);
    ASSERT_EQ(header.at(12), 0x08);
    ASSERT_EQ(header.at(13), 0x06);
}

TEST(FramesTest, ArpFrameSetGetHeaderHwTypeWorksCorrectly) {
    ArpFrame arp_frame;
    constexpr u8 hwaddr_type_offset = 0;
    constexpr u16 ether_proto_type = 0x0001;

    arp_frame.setHeaderHwAddrType(ether_proto_type);
    u16 hwtype = arp_frame.getHeaderHwAddrType();

    ASSERT_EQ(hwtype, ether_proto_type);
    ASSERT_EQ(arp_frame.getHeader().at(hwaddr_type_offset), 0x00);
    ASSERT_EQ(arp_frame.getHeader().at(hwaddr_type_offset + 1), 0x01);
}

TEST(FramesTest, ArpFrameSetGetHeaderPrTypeWorksCorrectly) {
    ArpFrame arp_frame;
    constexpr u8 praddr_type_offset = 2;
    constexpr u16 ipv4_proto_type = 0x0800;

    arp_frame.setHeaderPrAddrType(ipv4_proto_type);
    u16 prtype = arp_frame.getHeaderPrAddrType();

    ASSERT_EQ(prtype, ipv4_proto_type);
    ASSERT_EQ(arp_frame.getHeader().at(praddr_type_offset), 0x08);
    ASSERT_EQ(arp_frame.getHeader().at(praddr_type_offset + 1), 0x00);
}

TEST(FramesTest, ArpFrameSetGetHeaderHwSizeWorksCorrectly) {
    ArpFrame arp_frame;
    constexpr u8 hwaddr_size_offset = 4;
    constexpr u8 hwaddr_size = 6;

    arp_frame.setHeaderHwAddrSize(hwaddr_size);
    u8 hwsize = arp_frame.getHeaderHwAddrSize();

    ASSERT_EQ(hwsize, hwaddr_size);
    ASSERT_EQ(arp_frame.getHeader().at(hwaddr_size_offset), 6);
}

TEST(FramesTest, ArpFrameSetGetHeaderPrSizeWorksCorrectly) {
    ArpFrame arp_frame;
    constexpr u8 praddr_size_offset = 5;
    constexpr u8 praddr_size = 4;

    arp_frame.setHeaderPrAddrSize(praddr_size);
    u8 prsize = arp_frame.getHeaderPrAddrSize();

    ASSERT_EQ(prsize, praddr_size);
    ASSERT_EQ(arp_frame.getHeader().at(praddr_size_offset), 4);
}

TEST(FramesTest, ArpFrameSetGetHeaderOpcodeWorksCorrectly) {
    ArpFrame arp_frame;
    constexpr u8 opcode_offset = 6;
    constexpr u16 opcode_request = 0x0001;

    arp_frame.setHeaderOpcode(opcode_request);
    u16 opcode = arp_frame.getHeaderOpcode();

    ASSERT_EQ(opcode, opcode_request);
    ASSERT_EQ(arp_frame.getHeader().at(opcode_offset + 1), 0x0001);
}

TEST(FramesTest, ArpFrameSetGetSrcHwAddrWorksCorrectly) {
    ArpFrame arp_frame;
    constexpr u8 max_byte = 0xFF;

    arp_frame.setPayloadSourceHwAddr(
        generateArraySizeN<arp_frame.getHwAddrSize()>(max_byte));
    auto payload = arp_frame.getPayload();
    auto hwaddr = arp_frame.getPayloadSourceHwAddr();

    ASSERT_TRUE(
        compareArrayElements(hwaddr.begin(), hwaddr.end(), default_byte));
    ASSERT_TRUE(compareArrayElements(
        payload.begin(), std::next(payload.begin(), arp_frame.getHwAddrSize()),
        max_byte));
}

TEST(FramesTest, ArpFrameSetGetSrcPrAddrWorksCorrectly) {
    ArpFrame arp_frame;
    constexpr u8 srcpraddr_offset = 6;
    constexpr u8 max_byte = 0xFF;

    arp_frame.setPayloadSourcePrAddr(
        generateArraySizeN<arp_frame.getPrAddrSize()>(max_byte));
    auto payload = arp_frame.getPayload();
    auto praddr = arp_frame.getPayloadSourcePrAddr();

    ASSERT_TRUE(
        compareArrayElements(praddr.begin(), praddr.end(), default_byte));
    ASSERT_TRUE(compareArrayElements(
        std::next(payload.begin(), srcpraddr_offset),
        std::next(payload.begin(),
                  srcpraddr_offset + arp_frame.getPrAddrSize()),
        max_byte));
}

TEST(FramesTest, ArpFrameSetGetDstHwAddrWorksCorrectly) {
    ArpFrame arp_frame;
    constexpr u8 dsthwaddr_offset = 10;
    constexpr u8 max_byte = 0xFF;

    arp_frame.setPayloadDestinationHwAddr(
        generateArraySizeN<arp_frame.getHwAddrSize()>(max_byte));
    auto payload = arp_frame.getPayload();
    auto hwaddr = arp_frame.getPayloadDestinationHwAddr();

    ASSERT_TRUE(
        compareArrayElements(hwaddr.begin(), hwaddr.end(), default_byte));
    ASSERT_TRUE(compareArrayElements(
        std::next(payload.begin(), dsthwaddr_offset),
        std::next(payload.begin(),
                  dsthwaddr_offset + arp_frame.getHwAddrSize()),
        max_byte));
}

TEST(FramesTest, ArpFrameSetGetDstPrAddrWorksCorrectly) {
    ArpFrame arp_frame;
    constexpr u8 dstpraddr_offset = 16;
    constexpr u8 max_byte = 0xFF;

    arp_frame.setPayloadDestinationPrAddr(
        generateArraySizeN<arp_frame.getPrAddrSize()>(max_byte));
    auto payload = arp_frame.getPayload();
    auto praddr = arp_frame.getPayloadDestinationPrAddr();

    ASSERT_TRUE(
        compareArrayElements(praddr.begin(), praddr.end(), default_byte));
    ASSERT_TRUE(compareArrayElements(
        std::next(payload.begin(), dstpraddr_offset),
        std::next(payload.begin(),
                  dstpraddr_offset + arp_frame.getPrAddrSize()),
        max_byte));
}
