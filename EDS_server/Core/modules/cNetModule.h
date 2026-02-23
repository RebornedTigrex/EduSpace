#pragma once
#include "modules/BaseModule.h"
#include "EventBus.h"
#include "events/NetEvents.h"

#include "network/cNetIoContext.h"
#include "network/cNetWebSocketServer.h"
#include "network/cNetHttpServer.h"
#include <boost/signals2.hpp>
#include <memory>

namespace Sys::Modules {

    class cNetModule final : public BaseModule {
    public:
        cNetModule( unsigned short uWsPort, unsigned short uHttpPort)
            : BaseModule("NetModule"),
            
            m_uWsPort(uWsPort),
            m_uHttpPort(uHttpPort)
        {
        }

    protected:
        bool onInitialize() override;
        void onShutdown() override;

    private:
        void fnOnWsConnected(void* pSession);
        void fnOnWsDisconnected(void* pSession);
        void fnOnWsMessage(const std::string& sMsg, void* pSession);
        void fnOnSendWsText(const Sys::Events::sWsSendText& e);

    private:
        std::shared_ptr<EventBus> m_rBus = EventBus::instance();
        unsigned short m_uWsPort{};
        unsigned short m_uHttpPort{};

        Sys::Network::cNetIoContext m_oIo;
        std::unique_ptr<Sys::Network::cNetWebSocketServer> m_upWs;
        std::unique_ptr<Sys::Network::cNetHttpServer> m_upHttp;

        boost::signals2::connection m_cConnSend;
    };

} // namespace Sys::Modules
