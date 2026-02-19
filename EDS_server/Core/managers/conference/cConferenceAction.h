#pragma once
#include "interfaces/iAction.h"
#include "EventBus.h"
#include "events/NetEvents.h"
#include "services/cConferenceService.h"
#include "services/cPeerDirectoryService.h"

class cConferenceAction final : public iAction {
public:
    cConferenceAction(EventBus& rBus,
        Sys::Services::cConferenceService& rConf,
        Sys::Services::cPeerDirectoryService& rDir)
        : m_rBus(rBus), m_rConf(rConf), m_rDir(rDir)
    {
    }

    std::string fnType() const override { return "conf_*"; }

    bool fnExecute(const nlohmann::json& jMsg, const sExecutionContext& ctx) override
    {
        const std::string sType = jMsg.value("type", "");
        if (sType == "conf_create") return fnCreate(jMsg, ctx);
        if (sType == "conf_join")   return fnJoin(jMsg, ctx);
        if (sType == "conf_leave")  return fnLeave(ctx);
        if (sType == "conf_mic")    return fnMic(jMsg, ctx);
        return false;
    }

private:
    bool fnCreate(const nlohmann::json& jMsg, const sExecutionContext& ctx)
    {
        const std::string sTitle = jMsg.value("title", "Conference");
        auto pr = m_rConf.fnCreate(sTitle); // {confId, invite}
        const int iConfId = pr.first;
        const std::string sInvite = pr.second;

        (void)m_rConf.fnJoin(sInvite, ctx.sPeerKey);

        nlohmann::json jResp{
            {"type","conf_created"},
            {"confId", iConfId},
            {"invite", sInvite},
            {"peer", ctx.sPeerKey}
        };
        m_rBus.publish(Sys::Events::sWsSendText{ ctx.pSession, jResp.dump() });
        return true;
    }

    bool fnJoin(const nlohmann::json& jMsg, const sExecutionContext& ctx)
    {
        const std::string sInvite = jMsg.value("invite", "");
        auto optId = m_rConf.fnJoin(sInvite, ctx.sPeerKey);

        if (!optId.has_value()) {
            nlohmann::json jResp{
                {"type","conf_joined"},
                {"ok",false},
                {"reason","bad_invite"},
                {"peer", ctx.sPeerKey}
            };
            m_rBus.publish(Sys::Events::sWsSendText{ ctx.pSession, jResp.dump() });
            return true;
        }

        auto setPeers = m_rConf.fnPeers(ctx.sPeerKey);

        nlohmann::json jAck{
            {"type","conf_joined"},
            {"ok",true},
            {"confId", *optId},
            {"peer", ctx.sPeerKey},
            {"peers", setPeers}
        };
        m_rBus.publish(Sys::Events::sWsSendText{ ctx.pSession, jAck.dump() });

        nlohmann::json jEv{
            {"type","conf_peer_joined"},
            {"confId", *optId},
            {"peer", ctx.sPeerKey}
        };

        for (const auto& sPeer : setPeers) {
            if (sPeer == ctx.sPeerKey) continue;
            void* pSess = m_rDir.fnGetSessionByPeer(sPeer);
            if (pSess) m_rBus.publish(Sys::Events::sWsSendText{ pSess, jEv.dump() });
        }
        return true;
    }

    bool fnLeave(const sExecutionContext& ctx)
    {
        auto setPeersBefore = m_rConf.fnPeers(ctx.sPeerKey);
        m_rConf.fnLeave(ctx.sPeerKey);

        nlohmann::json jEv{ {"type","conf_peer_left"}, {"peer", ctx.sPeerKey} };
        for (const auto& sPeer : setPeersBefore) {
            if (sPeer == ctx.sPeerKey) continue;
            void* pSess = m_rDir.fnGetSessionByPeer(sPeer);
            if (pSess) m_rBus.publish(Sys::Events::sWsSendText{ pSess, jEv.dump() });
        }

        nlohmann::json jAck{ {"type","conf_left"}, {"peer", ctx.sPeerKey} };
        m_rBus.publish(Sys::Events::sWsSendText{ ctx.pSession, jAck.dump() });
        return true;
    }

    bool fnMic(const nlohmann::json& jMsg, const sExecutionContext& ctx)
    {
        const bool bEnabled = jMsg.value("enabled", true);
        auto setPeers = m_rConf.fnPeers(ctx.sPeerKey);

        nlohmann::json jEv{ {"type","conf_peer_mic"}, {"peer", ctx.sPeerKey}, {"enabled", bEnabled} };
        for (const auto& sPeer : setPeers) {
            if (sPeer == ctx.sPeerKey) continue;
            void* pSess = m_rDir.fnGetSessionByPeer(sPeer);
            if (pSess) m_rBus.publish(Sys::Events::sWsSendText{ pSess, jEv.dump() });
        }

        nlohmann::json jAck{ {"type","conf_mic_ack"}, {"peer", ctx.sPeerKey}, {"enabled", bEnabled} };
        m_rBus.publish(Sys::Events::sWsSendText{ ctx.pSession, jAck.dump() });
        return true;
    }

private:
    EventBus& m_rBus;
    Sys::Services::cConferenceService& m_rConf;
    Sys::Services::cPeerDirectoryService& m_rDir;
};
