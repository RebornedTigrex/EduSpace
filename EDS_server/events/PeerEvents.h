#pragma once
#include <string>

namespace Sys::Events {

    struct sPeerAssigned {
        void* pSession{};
        std::string sPeerKey;
    };

} // namespace Sys::Events
