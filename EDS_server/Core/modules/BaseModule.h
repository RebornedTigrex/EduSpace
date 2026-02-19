#pragma once
#include "interfaces/iModule.h"
#include <atomic>
#include <string>

class BaseModule : public iModule {
protected:
    std::string m_sName;
    std::atomic<bool> m_bEnabled;
    std::atomic<bool> m_bInitialized;
    int m_iId;

public:
    BaseModule(const std::string& sName = "Dev-Name", const int& iId = -1)
        : m_sName(sName),
        m_bEnabled(true),
        m_bInitialized(false),
        m_iId(iId)
    {
    }

    virtual ~BaseModule() = default;

    int getId() const override { return m_iId; }
    std::string getName() const override { return m_sName; }
    bool isEnabled() const override { return m_bEnabled.load(); }
    void setEnabled(bool bEnabled) override { m_bEnabled.store(bEnabled); }

    bool initialize() override
    {
        if (!m_bEnabled.load()) return false;
        if (m_bInitialized.load()) return true;

        const bool bOk = onInitialize();
        if (bOk) m_bInitialized.store(true);
        return bOk;
    }

    void shutdown() override
    {
        if (!m_bInitialized.load()) return;
        onShutdown();
        m_bInitialized.store(false);
    }

    friend class ModuleRegistry;

protected:
    virtual bool onInitialize() = 0;
    virtual void onShutdown() = 0;

private:
    void setId(int iId) { m_iId = iId; }
};
