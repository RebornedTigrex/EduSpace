#pragma once
#include "EventBus.h"
#include "managers/ModuleRegistry.h"

#include "services/cPeerDirectoryService.h"
#include "services/cConferenceService.h"

namespace Sys {

    class cAppCore {
    public:
        cAppCore();
        ~cAppCore();

        bool fnInit(unsigned short uWsPort, unsigned short uHttpPort);
        void fnRun();
        void fnShutdown();

    private:
        std::shared_ptr<EventBus> m_oBus = EventBus::instance();
        ModuleRegistry m_oRegistry;

        Sys::Services::cPeerDirectoryService m_oDir;
        Sys::Services::cConferenceService m_oConf;

        bool m_bRunning{ false };
    };

} // namespace Sys
