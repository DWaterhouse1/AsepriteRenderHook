#pragma once

// websocketpp
#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"

//boost
#include "boost/asio.hpp"

//std
#include <string>
#include <thread>
#include <set>
#include <functional>
#include <mutex>
#include <iostream>

namespace
{
namespace OpCode = websocketpp::frame::opcode;
} // namespace

class WebsocketServer
{
public:
	using MessageType = websocketpp::config::asio::message_type::ptr;
	using Endpoint = websocketpp::server<websocketpp::config::asio>;

	WebsocketServer(uint16_t port = 30001);
	~WebsocketServer();

	// not copyable
	WebsocketServer(const WebsocketServer&) = delete;
	WebsocketServer& operator=(const WebsocketServer&) = delete;
	void bindMessageHandler(std::function<void(MessageType)> callback);
	void sendAll(std::string msg, OpCode::value opCode = OpCode::TEXT);

private:
	void onOpen(Endpoint* endpoint, websocketpp::connection_hdl handle);
	void onClose(Endpoint* endpoint, websocketpp::connection_hdl handle);
	void onMessageInternal(
		websocketpp::server<websocketpp::config::asio>* server,
		websocketpp::connection_hdl handle,
		MessageType message);

	uint16_t m_port;
	Endpoint m_endpoint{};
	std::set<
		websocketpp::connection_hdl,
		std::owner_less<websocketpp::connection_hdl>> m_connections{};
	std::thread m_thread;
	std::function<void(MessageType message)> m_messageCallback = [](auto&&...){};
	std::mutex m_connectionMutex;
};