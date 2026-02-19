#pragma once
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

#include "interfaces/iAction.h"

class iActionManager {
public:
    using tActionFactory = std::function<std::unique_ptr<iAction>()>;

    virtual ~iActionManager() = default;

    virtual void registerAction(const std::string& type, tActionFactory factory) = 0;
    virtual void unregisterAction(const std::string& type) = 0;

    virtual bool handleMessage(const nlohmann::json& msg, const sExecutionContext& ctx) = 0;
};
