#include "cNetHttpServer.h"
#include <boost/beast/http.hpp>
#include <iostream>
#include <utility> 
namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;

namespace Sys {
    namespace Network {

        cNetHttpServer::cNetHttpServer(boost::asio::io_context& ioCtx, unsigned short port)
            : BaseModule("NetHttpServer")
            , m_rIoCtx(ioCtx)
            , m_uPort(port)
            , m_bRunning(false)
        {
        }


        cNetHttpServer::~cNetHttpServer()
        {
            fnStop();
        }


        bool cNetHttpServer::fnStart()
        {
            if (m_bRunning) return true;

            boost::system::error_code ec;
            tcp::endpoint ep(tcp::v4(), m_uPort);

            if (!m_pAcceptor)
                m_pAcceptor = std::make_unique<tcp::acceptor>(m_rIoCtx);

            m_pAcceptor->open(ep.protocol(), ec);
            if (ec) { std::cerr << "[HTTP] open error: " << ec.message() << "\n"; return false; }

            m_pAcceptor->set_option(boost::asio::socket_base::reuse_address(true), ec);
            if (ec) { std::cerr << "[HTTP] reuse_address error: " << ec.message() << "\n"; return false; }

            m_pAcceptor->bind(ep, ec);
            if (ec) { std::cerr << "[HTTP] bind error: " << ec.message() << "\n"; return false; }

            m_pAcceptor->listen(boost::asio::socket_base::max_listen_connections, ec);
            if (ec) { std::cerr << "[HTTP] listen error: " << ec.message() << "\n"; return false; }

            m_bRunning = true;
            fnDoAccept();
            return true;
        }

        void cNetHttpServer::fnStop()
        {
            if (!m_bRunning) return;
            m_bRunning = false;

            if (m_pAcceptor) {
                boost::system::error_code ec;
                m_pAcceptor->close(ec);
                m_pAcceptor.reset();
            }
        }

        void cNetHttpServer::fnDoAccept()
        {
            if (!m_bRunning || !m_pAcceptor) return;

            m_pAcceptor->async_accept(
                [this](boost::system::error_code ec, tcp::socket socket)
                {
                    if (ec) {
                        boost::asio::post(m_rIoCtx, [this]() { fnDoAccept(); });
                        return;
                    }

                    auto stream = std::make_shared<boost::beast::tcp_stream>(std::move(socket));
                    auto buffer = std::make_shared<boost::beast::flat_buffer>();
                    auto req = std::make_shared<http::request<http::string_body>>();

                    http::async_read(*stream, *buffer, *req,
                        [this, stream, buffer, req](boost::system::error_code ec2, std::size_t) mutable
                        {
                            if (ec2) {
                                // клиент мог разорвать соединение — просто выходим
                                return;
                            }

                            auto res = std::make_shared<http::response<http::string_body>>(http::status::ok, req->version());
                            res->set(http::field::server, "cNetHttpServer");
                            res->set(http::field::content_type, "application/json");
                            res->keep_alive(false);

                            std::string target = std::string(req->target());
                            if (target == "/health")
                                res->body() = m_fnHealth ? m_fnHealth() : R"({"status":"ok"})";
                            else if (target == "/metrics")
                                res->body() = m_fnMetrics ? m_fnMetrics() : R"({"metrics":{}})";
                            else {
                                res->result(http::status::not_found);
                                res->body() = R"({"error":"not_found"})";
                            }
                            res->prepare_payload();

                            http::async_write(*stream, *res,
                                [stream, res](boost::system::error_code, std::size_t)
                                {
                                    boost::system::error_code ec3;
                                    stream->socket().shutdown(tcp::socket::shutdown_send, ec3);
                                });
                        });

                    boost::asio::post(m_rIoCtx, [this]() { fnDoAccept(); });
                });
        }
    } // namespace Network
} // namespace Sys
