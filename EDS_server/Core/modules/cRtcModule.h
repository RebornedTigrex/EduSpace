#pragma once

#include "modules/BaseModule.h"
#include "EventBus.h"

#include "events/NetEvents.h"
#include "events/PeerEvents.h"
#include "events/RtcEvents.h"

#include "services/cPeerDirectoryService.h"
#include "rtc/cRtcManager.h"

#include <boost/signals2.hpp>
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Sys::Modules {

    class cRtcModule final : public BaseModule {
    public:
        cRtcModule(EventBus& rBus, Sys::Services::cPeerDirectoryService& rDir)
            : BaseModule("RtcModule"),
            m_rBus(rBus),
            m_rDir(rDir)
        {
        }

        ~cRtcModule() override = default;

        std::shared_ptr<Sys::Rtc::cRtcPeer> fnGetPeer(const std::string& sPeerKey)
        {
            return (m_spRtc ? m_spRtc->fnGetPeer(sPeerKey) : nullptr);
        }

        bool fnOnSignaling(void* pSession,
            const nlohmann::json& jMsg,
            const std::string& sPeerKey);

    protected:
        bool onInitialize() override;
        void onShutdown() override;

    private:
        void fnOnWsConnected(const Sys::Events::sWsConnected& oEv);
        void fnOnWsDisconnected(const Sys::Events::sWsDisconnected& oEv);
        std::string fnGeneratePeerKey() const;
        void fnSendPeerAssigned(void* pSession, const std::string& sPeerKey);
        void fnPublishPeerAssigned(void* pSession, const std::string& sPeerKey);
        void fnPublishRtcBinaryIn(const std::string& sPeerKey, const std::vector<uint8_t>& vData);

    private:
        EventBus& m_rBus;
        Sys::Services::cPeerDirectoryService& m_rDir;

        std::shared_ptr<Sys::Rtc::cRtcManager> m_spRtc;

        boost::signals2::connection m_cConnConn;
        boost::signals2::connection m_cConnDisc;
    };

} // namespace Sys::Modules
