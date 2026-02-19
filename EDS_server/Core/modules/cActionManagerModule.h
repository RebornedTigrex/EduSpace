#pragma once

#include "modules/BaseModule.h"
#include "BaseActionManager.h"

namespace Sys::Modules {

    class cActionManagerModule final : public BaseModule {
    public:
        cActionManagerModule()
            : BaseModule("ActionManagerModule")
        {
        }

        BaseActionManager& fnMgr() { return m_oMgr; }

    protected:
        bool onInitialize() override { return true; }
        void onShutdown() override { m_oMgr.clear(); }

    private:
        BaseActionManager m_oMgr;
    };

} // namespace Sys::Modules
