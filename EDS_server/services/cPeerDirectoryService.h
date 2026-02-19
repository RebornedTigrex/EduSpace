#pragma once
#include <unordered_map>
#include <mutex>
#include <string>

namespace Sys::Services {

    class cPeerDirectoryService {
    public:
        void fnBind(void* pSession, const std::string& sPeerKey)
        {
            std::lock_guard<std::mutex> lg(m_mtx);
            m_mapSessToPeer[pSession] = sPeerKey;
            m_mapPeerToSess[sPeerKey] = pSession;
        }

        void fnUnbindBySession(void* pSession)
        {
            std::lock_guard<std::mutex> lg(m_mtx);
            auto it = m_mapSessToPeer.find(pSession);
            if (it == m_mapSessToPeer.end()) return;
            m_mapPeerToSess.erase(it->second);
            m_mapSessToPeer.erase(it);
        }

        std::string fnGetPeerBySession(void* pSession)
        {
            std::lock_guard<std::mutex> lg(m_mtx);
            auto it = m_mapSessToPeer.find(pSession);
            return it == m_mapSessToPeer.end() ? "" : it->second;
        }

        void* fnGetSessionByPeer(const std::string& sPeerKey)
        {
            std::lock_guard<std::mutex> lg(m_mtx);
            auto it = m_mapPeerToSess.find(sPeerKey);
            return it == m_mapPeerToSess.end() ? nullptr : it->second;
        }

    private:
        std::mutex m_mtx;
        std::unordered_map<void*, std::string> m_mapSessToPeer;
        std::unordered_map<std::string, void*> m_mapPeerToSess;
    };

} // namespace Sys::Services
