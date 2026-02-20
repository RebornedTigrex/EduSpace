#pragma once
#include "modules/BaseModule.h"
#include "EventBus.h"
#include "events/NetEvents.h"
#include "services/cPeerDirectoryService.h"
#include "modules/cActionManagerModule.h"

#include <boost/signals2.hpp>

namespace Sys::Modules {

    class cActionRouterModule final : public BaseModule {
    public:
        cActionRouterModule(EventBus& rBus,
            Sys::Services::cPeerDirectoryService& rDir,
            cActionManagerModule& rMgr)
            : BaseModule("ActionRouter"),
            m_rBus(rBus),
            m_rDir(rDir),
            m_rMgr(rMgr)
        {
        }

    protected:
        bool onInitialize() override;
        void onShutdown() override;

    private:
        void fnOnWsText(const Sys::Events::sWsMessageText& e);

    private:
        EventBus& m_rBus;
        Sys::Services::cPeerDirectoryService& m_rDir;
        cActionManagerModule& m_rMgr;

        boost::signals2::connection m_cConnWs;
    };

} // namespace Sys::Modules
