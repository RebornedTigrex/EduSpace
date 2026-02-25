#include "modules/cRtcModule.h"
#include "utils/cLogger.h"

#include <random>

using namespace Sys::Modules;

bool cRtcModule::onInitialize()
{
    m_spRtc = std::make_shared<Sys::Rtc::cRtcManager>(
        [this](void* pSession, const std::string& sMsg)
        {
            m_rBus->publish(Sys::Events::sWsSendText{ pSession, sMsg });
        }
    );
    if (!m_spRtc->fnInit()) {
        m_spRtc.reset();
        Sys::cLogger::fnLog(Sys::cLogger::Level::Error, "[RtcModule] RtcManager init failed");
        return false;
    }

    m_spRtc->fnSetOnPeerBinary(
        [this](const std::string& sPeerKey, const std::vector<uint8_t>& vData)
        {
            fnPublishRtcBinaryIn(sPeerKey, vData);
        }
    );
    m_cConnConn = m_rBus->subscribe<Sys::Events::sWsConnected>(
        [this](const Sys::Events::sWsConnected& oEv)
        {
            fnOnWsConnected(oEv);
        }
    );

    m_cConnDisc = m_rBus->subscribe<Sys::Events::sWsDisconnected>(
        [this](const Sys::Events::sWsDisconnected& oEv)
        {
            fnOnWsDisconnected(oEv);
        }
    );

    return true;
}

void cRtcModule::onShutdown()
{
    m_cConnConn.disconnect();
    m_cConnDisc.disconnect();
    if (m_spRtc) {
        m_spRtc->fnShutdown();
        m_spRtc.reset();
    }
}

void cRtcModule::fnOnWsConnected(const Sys::Events::sWsConnected& oEv)
{
    const std::string sPeerKey = fnGeneratePeerKey();

    // bind peer <-> session
    m_rDir.fnBind(oEv.pSession, sPeerKey);
    fnSendPeerAssigned(oEv.pSession, sPeerKey);
    fnPublishPeerAssigned(oEv.pSession, sPeerKey);

    Sys::cLogger::fnLog(Sys::cLogger::Level::Info, "Peer assigned: " + sPeerKey);
}

void cRtcModule::fnOnWsDisconnected(const Sys::Events::sWsDisconnected& oEv)
{
    const std::string sPeerKey = m_rDir.fnGetPeerBySession(oEv.pSession);

    // RTC cleanup 
    if (m_spRtc) {
        (void)m_spRtc->fnOnWsDisconnected(oEv.pSession);
    }

    // Directory cleanup
    m_rDir.fnUnbindBySession(oEv.pSession);

    if (!sPeerKey.empty()) {
        Sys::cLogger::fnLog(Sys::cLogger::Level::Info, "Peer disconnected: " + sPeerKey);
    }
}

bool cRtcModule::fnOnSignaling(void* pSession,
    const nlohmann::json& jMsg,
    const std::string& sPeerKey)
{
    if (!m_spRtc) return false;
    if (!pSession) return false;
    if (sPeerKey.empty()) return false;
    const std::string sType = jMsg.value("type", "");
    if (sType.empty()) return false;

    if (sType.rfind("webrtc_", 0) != 0) {
        return false;
    }
    nlohmann::json jFixed = jMsg;
    jFixed["peer"] = sPeerKey;

    m_spRtc->fnOnSignalingMessage(pSession, jFixed);
    return true;
}

[[deprecated("Используйте utils/wordGenerator.h")]]std::string cRtcModule::fnGeneratePeerKey() const //Заменить на 
{
    static const char* sHex = "0123456789abcdef";

    std::random_device oRd;
    std::mt19937_64 oRng(oRd());
    std::uniform_int_distribution<int> oDist(0, 15);

    std::string sKey;
    sKey.reserve(32);

    for (int i = 0; i < 32; ++i) {
        sKey.push_back(sHex[oDist(oRng)]);
    }

    return sKey;
}

void cRtcModule::fnSendPeerAssigned(void* pSession, const std::string& sPeerKey)
{
    nlohmann::json jAssign;
    jAssign["type"] = "peer_assigned";
    jAssign["peer"] = sPeerKey;

    m_rBus->publish(Sys::Events::sWsSendText{ pSession, jAssign.dump() });
}

void cRtcModule::fnPublishPeerAssigned(void* pSession, const std::string& sPeerKey)
{
    m_rBus->publish(Sys::Events::sPeerAssigned{ pSession, sPeerKey });
}

void cRtcModule::fnPublishRtcBinaryIn(const std::string& sPeerKey, const std::vector<uint8_t>& vData)
{
    m_rBus->publish(Sys::Events::sRtcBinaryIn{ sPeerKey, vData });
}
