#pragma once
#include <string>

namespace Sys::Events {

    struct sWsConnected { void* pSession{}; };
    struct sWsDisconnected { void* pSession{}; };

    struct sWsMessageText {
        void* pSession{};
        std::string sText;
    };

    struct sWsSendText {
        void* pSession{};
        std::string sText;
    };

} // namespace Sys::Events
