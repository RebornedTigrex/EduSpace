#include "modules/cNetModule.h"
#include "utils/cLogger.h"

using namespace Sys::Modules;

bool cNetModule::onInitialize()
{
    if (!m_oIo.fnInit()) return false;

    m_upWs = std::make_unique<Sys::Network::cNetWebSocketServer>(m_oIo.fnIo(), m_uWsPort);
    m_upHttp = std::make_unique<Sys::Network::cNetHttpServer>(m_oIo.fnIo(), m_uHttpPort);

    m_upWs->fnSetOnConnected([this](void* p) { fnOnWsConnected(p); });
    m_upWs->fnSetOnDisconnected([this](void* p) { fnOnWsDisconnected(p); });
    m_upWs->fnSetOnMessage([this](const std::string& s, void* p) { fnOnWsMessage(s, p); });

    m_upHttp->fnSetHealthFn([]() { return R"({"status":"ok"})"; });
    m_upHttp->fnSetMetricsFn([]() { return R"({"metrics":{"ok":1}})"; });

    m_cConnSend = m_rBus->subscribe<Sys::Events::sWsSendText>(
        [this](const Sys::Events::sWsSendText& e) { fnOnSendWsText(e); }
    );
    if (!m_oIo.fnStart()) return false;
    const bool okWs = m_upWs->initialize();    
    const bool okHttp = m_upHttp->initialize(); 
    return okWs && okHttp;
}

void cNetModule::onShutdown()
{
    m_cConnSend.disconnect();

    if (m_upWs)   m_upWs->shutdown();  
    if (m_upHttp) m_upHttp->shutdown(); 

    m_oIo.fnStop();

    m_upWs.reset();
    m_upHttp.reset();
}


void cNetModule::fnOnWsConnected(void* pSession)
{
    m_rBus->publish(Sys::Events::sWsConnected{ pSession });
}

void cNetModule::fnOnWsDisconnected(void* pSession)
{
    m_rBus->publish(Sys::Events::sWsDisconnected{ pSession });
}

void cNetModule::fnOnWsMessage(const std::string& sMsg, void* pSession)
{
    m_rBus->publish(Sys::Events::sWsMessageText{ pSession, sMsg });
}

void cNetModule::fnOnSendWsText(const Sys::Events::sWsSendText& e)
{
    if (!m_upWs) return;
    m_upWs->fnSendText(e.pSession, e.sText);
}
