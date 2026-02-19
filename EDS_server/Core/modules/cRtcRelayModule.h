#pragma once
#include "modules/BaseModule.h"
#include "EventBus.h"
#include "events/RtcEvents.h"

#include "services/cPeerDirectoryService.h"
#include "services/cConferenceService.h"
#include "modules/cRtcModule.h"

namespace Sys::Modules {

    class cRtcRelayModule final : public BaseModule {
    public:
        cRtcRelayModule(EventBus& rBus,
            Sys::Services::cConferenceService& rConf,
            Sys::Services::cPeerDirectoryService& rDir,
            cRtcModule& rRtc)
            : BaseModule("RtcRelayModule"),
            m_rBus(rBus),
            m_rConf(rConf),
            m_rDir(rDir),
            m_rRtc(rRtc)
        {
        }

    protected:
        bool onInitialize() override;
        void onShutdown() override;

    private:
        void fnOnRtcBinary(const Sys::Events::sRtcBinaryIn& e);

    private:
        EventBus& m_rBus;
        Sys::Services::cConferenceService& m_rConf;
        Sys::Services::cPeerDirectoryService& m_rDir;
        cRtcModule& m_rRtc;

        boost::signals2::connection m_cConnBin;
    };

} // namespace Sys::Modules
