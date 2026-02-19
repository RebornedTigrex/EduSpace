#include "App/cAppCore.h"
#include <thread>
#include <chrono>

#include "modules/cNetModule.h"
#include "modules/cRtcModule.h"
#include "modules/cConferenceModule.h"
#include "modules/cRtcRelayModule.h"

#include "modules/cActionManagerModule.h"
#include "modules/cActionRouterModule.h"

#include "managers/conference/cConferenceAction.h"
#include "rtc/cWebRtcAction.h"

namespace Sys {

    cAppCore::cAppCore() {}
    cAppCore::~cAppCore() { fnShutdown(); }

    bool cAppCore::fnInit(unsigned short uWsPort, unsigned short uHttpPort)
    {
        // 1) Net
        (void)m_oRegistry.registerModule<Sys::Modules::cNetModule>(m_oBus, uWsPort, uHttpPort);

        // 2) Domain
        (void)m_oRegistry.registerModule<Sys::Modules::cConferenceModule>(m_oConf);

        // 3) RTC
        auto* pRtc = m_oRegistry.registerModule<Sys::Modules::cRtcModule>(m_oBus, m_oDir);

        // 4) RTC relay
        (void)m_oRegistry.registerModule<Sys::Modules::cRtcRelayModule>(m_oBus, m_oConf, m_oDir, *pRtc);

        // 5) Actions infra
        auto* pActionMgrMod = m_oRegistry.registerModule<Sys::Modules::cActionManagerModule>();
        (void)m_oRegistry.registerModule<Sys::Modules::cActionRouterModule>(m_oBus, m_oDir, *pActionMgrMod);
        {
            auto& rMgr = pActionMgrMod->fnMgr();

            // conf_* -> один action
            rMgr.registerAction("conf_create", [&]() {
                return std::make_unique<cConferenceAction>(m_oBus, m_oConf, m_oDir);
                });
            rMgr.registerAction("conf_join", [&]() {
                return std::make_unique<cConferenceAction>(m_oBus, m_oConf, m_oDir);
                });
            rMgr.registerAction("conf_leave", [&]() {
                return std::make_unique<cConferenceAction>(m_oBus, m_oConf, m_oDir);
                });
            rMgr.registerAction("conf_mic", [&]() {
                return std::make_unique<cConferenceAction>(m_oBus, m_oConf, m_oDir);
                });

            // webrtc_* -> один action
            rMgr.registerAction("webrtc_offer", [&]() {
                return std::make_unique<cWebRtcAction>(*pRtc);
                });
            rMgr.registerAction("webrtc_ice", [&]() {
                return std::make_unique<cWebRtcAction>(*pRtc);
                });
            rMgr.registerAction("webrtc_close", [&]() {
                return std::make_unique<cWebRtcAction>(*pRtc);
                });
        }

        const bool bOk = m_oRegistry.initializeAll();
        m_bRunning = bOk;
        return bOk;
    }

    void cAppCore::fnRun()
    {
        while (m_bRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void cAppCore::fnShutdown()
    {
        if (!m_bRunning) return;
        m_bRunning = false;
        m_oRegistry.shutdownAll();
    }

} // namespace Sys
