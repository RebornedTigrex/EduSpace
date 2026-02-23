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
        cActionRouterModule(
            Sys::Services::cPeerDirectoryService& rDir,
            cActionManagerModule& rMgr)
            : BaseModule("ActionRouter"),
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
        std::shared_ptr<EventBus> m_rBus = EventBus::instance();
        Sys::Services::cPeerDirectoryService& m_rDir;
        cActionManagerModule& m_rMgr;

        boost::signals2::connection m_cConnWs;
    };

} // namespace Sys::Modules
