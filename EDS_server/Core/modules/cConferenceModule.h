#pragma once
#include "modules/BaseModule.h"
#include "EventBus.h"
#include "services/cConferenceService.h"

namespace Sys::Modules {

    class cConferenceModule final : public BaseModule {
    public:
        explicit cConferenceModule(Sys::Services::cConferenceService& rSvc)
            : BaseModule("ConferenceModule"),
            m_rSvc(rSvc)
        {
        }

        Sys::Services::cConferenceService& fnSvc() { return m_rSvc; }

    protected:
        bool onInitialize() override { return true; }
        void onShutdown() override {}

    private:
        Sys::Services::cConferenceService& m_rSvc;
    };

} // namespace Sys::Modules
