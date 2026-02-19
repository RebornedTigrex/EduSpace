#pragma once
#include "managers/conference/cConferenceManager.h"
#include <optional>
#include <string>
#include <unordered_set>

namespace Sys::Services {

    class cConferenceService {
    public:
        std::pair<int, std::string> fnCreate(const std::string& sTitle)
        {
            return m_oMgr.fnCreateConference(sTitle);
        }

        std::optional<int> fnJoin(const std::string& sInvite, const std::string& sPeerKey)
        {
            return m_oMgr.fnJoinByInvite(sInvite, sPeerKey);
        }

        void fnLeave(const std::string& sPeerKey)
        {
            m_oMgr.fnLeave(sPeerKey);
        }

        std::unordered_set<std::string> fnPeers(const std::string& sPeerKey)
        {
            return m_oMgr.fnGetPeersInSameConf(sPeerKey);
        }

    private:
        Sys::Conference::cConferenceManager m_oMgr;
    };

} // namespace Sys::Services
