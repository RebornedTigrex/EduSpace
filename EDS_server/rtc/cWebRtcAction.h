#pragma once
#include "interfaces/iAction.h"
#include "modules/cRtcModule.h"

class cWebRtcAction final : public iAction {
public:
    explicit cWebRtcAction(Sys::Modules::cRtcModule& rRtc)
        : m_rRtc(rRtc)
    {
    }

    std::string fnType() const override { return "webrtc_*"; }

    bool fnExecute(const nlohmann::json& jMsg, const sExecutionContext& ctx) override
    {
        return m_rRtc.fnOnSignaling(ctx.pSession, jMsg, ctx.sPeerKey);
    }

private:
    Sys::Modules::cRtcModule& m_rRtc;
};
