#include "modules/cRtcRelayModule.h"
#include "rtc/cRtcPeer.h"
using namespace Sys::Modules;

bool cRtcRelayModule::onInitialize()
{
    m_cConnBin = m_rBus.subscribe<Sys::Events::sRtcBinaryIn>(
        [this](const Sys::Events::sRtcBinaryIn& e) { fnOnRtcBinary(e); }
    );
    return true;
}

void cRtcRelayModule::onShutdown()
{
    m_cConnBin.disconnect();
}

void cRtcRelayModule::fnOnRtcBinary(const Sys::Events::sRtcBinaryIn& e)
{
    auto setPeers = m_rConf.fnPeers(e.sFromPeer);

    for (const auto& sPeer : setPeers) {
        if (sPeer == e.sFromPeer) continue;
        auto spPeer = m_rRtc.fnGetPeer(sPeer);
        if (spPeer) spPeer->fnSendBinary(e.vData);
    }
}
