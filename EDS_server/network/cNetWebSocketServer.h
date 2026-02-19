#pragma once

#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast.hpp>

#include "modules/BaseModule.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <deque>

namespace Sys::Network {

    class cNetWebSocketServer final : public BaseModule {
    public:
        using tOnMessage = std::function<void(const std::string&, void*)>;
        using tOnConnected = std::function<void(void*)>;
        using tOnDisconnected = std::function<void(void*)>;

        cNetWebSocketServer(boost::asio::io_context& ctx, unsigned short port);
        ~cNetWebSocketServer() override;

        void fnSetOnMessage(tOnMessage fn) { m_fnOnMessage = std::move(fn); }
        void fnSetOnConnected(tOnConnected fn) { m_fnOnConnected = std::move(fn); }
        void fnSetOnDisconnected(tOnDisconnected fn) { m_fnOnDisconnected = std::move(fn); }

        bool fnStart();
        void fnStop();

        bool fnSendText(void* pSession, const std::string& txt);

    protected:
        bool onInitialize() override { return fnStart(); }
        void onShutdown() override { fnStop(); }

    private:
        struct sWsSession : public std::enable_shared_from_this<sWsSession> {
            using tcp = boost::asio::ip::tcp;
            using ws_stream = boost::beast::websocket::stream<tcp::socket>;

            sWsSession(tcp::socket&& sock, cNetWebSocketServer* owner, boost::asio::io_context& io);

            void fnStart();
            void fnDoRead();

            void fnSendTextQueued(std::string txt);
            void fnDoWrite();
            void fnClose();

            ws_stream m_ws;

           
            boost::asio::strand<boost::asio::io_context::executor_type> m_strand;

            boost::beast::flat_buffer m_buffer;
            cNetWebSocketServer* m_owner{ nullptr };

            std::deque<std::shared_ptr<std::string>> m_outQ;

            bool m_open{ false };
            bool m_writing{ false };
        };

        void fnDoAccept();
        void fnRegisterSession(void* key, std::shared_ptr<sWsSession> s);
        void fnUnregisterSession(void* key);

    private:
        boost::asio::io_context& m_ctx;
        unsigned short m_port{ 0 };

        std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
        bool m_running{ false };

        tOnMessage      m_fnOnMessage;
        tOnConnected    m_fnOnConnected;
        tOnDisconnected m_fnOnDisconnected;

        std::mutex m_sessionsMtx;
        std::unordered_map<void*, std::weak_ptr<sWsSession>> m_sessions;
    };

} // namespace Sys::Network
