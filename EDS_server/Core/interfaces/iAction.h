#pragma once
#include <nlohmann/json.hpp>
#include <string>

struct sExecutionContext {
    void* pSession{};
    std::string sPeerKey; // trusted peer
};

class iAction {
public:
    virtual ~iAction() = default;
    virtual std::string fnType() const = 0; // для дебага/инфо
    virtual bool fnExecute(const nlohmann::json& jMsg, const sExecutionContext& ctx) = 0;
};
