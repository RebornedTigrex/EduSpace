#pragma once
#include <string>
#include <vector>

namespace Sys::Events {

    struct sRtcBinaryIn {
        std::string sFromPeer;
        std::vector<uint8_t> vData;
    };

} // namespace Sys::Events
