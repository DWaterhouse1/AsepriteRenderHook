#include "Websocket.h"

WebsocketServer::WebsocketServer(uint16_t port) : m_port{ port }
{
	// disable logging
	m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
	m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

	m_endpoint.init_asio();

	// internal message callback
	m_endpoint.set_message_handler(websocketpp::lib::bind(
		&WebsocketServer::onMessageInternal,
		this,
		&m_endpoint,
		websocketpp::lib::placeholders::_1,
		websocketpp::lib::placeholders::_2));

	// bind connect-disconnect handlers
	m_endpoint.set_open_handler(websocketpp::lib::bind(
		&WebsocketServer::onOpen,
		this,
		&m_endpoint,
		websocketpp::lib::placeholders::_1));

	m_endpoint.set_close_handler(websocketpp::lib::bind(
		&WebsocketServer::onClose,
		this,
		&m_endpoint,
		websocketpp::lib::placeholders::_1));

	m_endpoint.listen(m_port);
	m_endpoint.start_accept();

	// dispatch running of server to a new thread
	m_thread = std::thread{ &Endpoint::run, &m_endpoint };
}

WebsocketServer::~WebsocketServer()
{
	m_endpoint.stop_listening();

	// close each open connection
	std::unique_lock<std::mutex> lock(m_connectionMutex);
	for (
		auto connection = m_connections.begin();
		connection != m_connections.end();)
	{
		websocketpp::lib::error_code errorCode;
		m_endpoint.close(
			*connection,
			websocketpp::close::status::normal,
			"shutdown",
			errorCode);
		if (errorCode)
		{
			std::cerr <<
				"Error closing connection: " <<
				errorCode.message() <<
				std::endl;
		}
		m_connections.erase(connection++);
	}
	lock.unlock();

	// wait for Endpoint::run to clean up
	m_thread.join();
}

void WebsocketServer::bindMessageHandler(std::function<void(MessageType msg)> callback)
{
	m_messageCallback = callback;
}

void WebsocketServer::onOpen(
	Endpoint* endpoint,
	websocketpp::connection_hdl handle)
{
	std::lock_guard<std::mutex> lock(m_connectionMutex);
	m_connections.insert(handle);
}

void WebsocketServer::onClose(
	Endpoint* endpoint,
	websocketpp::connection_hdl handle)
{
	std::lock_guard<std::mutex> lock(m_connectionMutex);
	m_connections.erase(handle);
}

void WebsocketServer::onMessageInternal(
	websocketpp::server<websocketpp::config::asio>* server,
	websocketpp::connection_hdl handle,
	MessageType message)
{
	m_messageCallback(message);
}