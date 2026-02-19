#pragma once

#include "interfaces/iActionManager.h"
#include <unordered_map>

class BaseActionManager final : public iActionManager {
public:
    BaseActionManager() = default;
    ~BaseActionManager() override = default;

    void registerAction(const std::string& sType, tActionFactory fnFactory) override
    {
        m_actions[sType] = std::move(fnFactory);
    }

    void unregisterAction(const std::string& sType) override
    {
        m_actions.erase(sType);
    }

    bool handleMessage(const nlohmann::json& jMsg, const sExecutionContext& oCtx) override
    {
        const std::string sType = jMsg.value("type", "");
        if (sType.empty()) return false;

        auto it = m_actions.find(sType);
        if (it == m_actions.end()) return false;

        std::unique_ptr<iAction> upAction = it->second ? it->second() : std::unique_ptr<iAction>();
        if (!upAction) return false;

        return upAction->fnExecute(jMsg, oCtx);
    }

    void clear()
    {
        m_actions.clear();
    }

private:
    std::unordered_map<std::string, tActionFactory> m_actions;
};
