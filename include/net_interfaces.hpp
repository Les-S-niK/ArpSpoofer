
#include <net/if.h>
#include <sys/ioctl.h>

#include <array>
#include <cstring>

#include "number_usings.hpp"
#include "raw_socket.hpp"

class SystemIoctl {
    static constexpr u8 ifname_size = IFNAMSIZ;
    using ifname_t = std::array<char, ifname_size>;

   public:
    SystemIoctl(RawSocket::sock_fd sock_fd, ifname_t ifname)
        : _sock_fd(sock_fd), _ifname(ifname) {
        std::memset(&_ifr, 0, sizeof(struct ifreq));
        std::memcpy(&_ifr.ifr_ifrn.ifrn_name, _ifname.data(), ifname_size);
    }

   private:
    RawSocket::sock_fd _sock_fd{};
    ifname_t _ifname;
    struct ifreq _ifr{0};  // NOLINT.
};
