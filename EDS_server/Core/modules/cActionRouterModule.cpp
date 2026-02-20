#include "modules/cActionRouterModule.h"
#include "utils/cLogger.h"

using namespace Sys::Modules;

bool cActionRouterModule::onInitialize()
{
    m_cConnWs = m_rBus.subscribe<Sys::Events::sWsMessageText>(
        [this](const Sys::Events::sWsMessageText& e) { fnOnWsText(e); }
    );
    return true;
}

void cActionRouterModule::onShutdown()
{
    m_cConnWs.disconnect();
}

void cActionRouterModule::fnOnWsText(const Sys::Events::sWsMessageText& e)
{
    nlohmann::json jMsg;
    try { jMsg = nlohmann::json::parse(e.sText); }
    catch (...) { return; }

    const std::string sType = jMsg.value("type", "");
    if (sType.empty()) return;

    const std::string sPeerKey = m_rDir.fnGetPeerBySession(e.pSession);
    if (sPeerKey.empty()) return;

    const std::string sClientPeer = jMsg.value("peer", "");
    if (!sClientPeer.empty() && sClientPeer != sPeerKey) {
        Sys::cLogger::fnLog(Sys::cLogger::Level::Warning,
            "Peer impersonation attempt! Expected: " + sPeerKey + ", got: " + sClientPeer);
        return;
    }

    sExecutionContext ctx;
    ctx.pSession = e.pSession;
    ctx.sPeerKey = sPeerKey;

    (void)m_rMgr.fnMgr().handleMessage(jMsg, ctx);
}
