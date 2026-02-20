#pragma once
#include "modules/BaseModule.h"
#include "interfaces/iModule.h"

#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>
#include <stdexcept>
#include <iostream>

class ModuleRegistry {
private:
    std::unordered_map<int, std::unique_ptr<iModule>> m_mapModules;
    int m_iNextId = 1;
    std::mutex m_mtx;

    int fnGenerateId()
    {
        std::lock_guard<std::mutex> lg(m_mtx);
        return m_iNextId++;
    }

    void fnSetModuleId(BaseModule* pModule, int iId)
    {
        if (pModule) pModule->setId(iId);
    }

public:
    template<typename T, typename... Args>
    T* registerModule(Args&&... args)
    {
        auto upModule = std::make_unique<T>(std::forward<Args>(args)...);
        int iId = fnGenerateId();

        if (auto* pBase = dynamic_cast<BaseModule*>(upModule.get())) {
            fnSetModuleId(pBase, iId);
        }

        if (m_mapModules.find(iId) != m_mapModules.end()) {
            throw std::runtime_error("Duplicate module id " + std::to_string(iId));
        }

        T* pPtr = upModule.get();
        m_mapModules[iId] = std::move(upModule);
        return pPtr;
    }

    bool initializeAll()
    {
        std::lock_guard<std::mutex> lg(m_mtx);
        bool bAllOk = true;

        for (auto& [iId, upMod] : m_mapModules) {
            if (upMod->isEnabled()) {
                if (!upMod->initialize()) {
                    std::cerr << "Failed to initialize module: " << iId << "\n";
                    bAllOk = false;
                }
            }
        }
        return bAllOk;
    }

    void shutdownAll()
    {
        std::lock_guard<std::mutex> lg(m_mtx);
        for (auto& [iId, upMod] : m_mapModules) {
            if (upMod->isEnabled()) {
                upMod->shutdown();
            }
        }
    }
};
