#pragma once
#ifndef __XMQ_CLIENT_H__
#define __XMQ_CLIENT_H__

#include <memory>

#include <client.h>
#include <connectionlistener.h>
#include <base64.h>
#include <message.h>
#include <messagehandler.h>


namespace ConsoleColor
{
	enum Type
	{
		Black = 0x0000,
		Blue = 0x0001,
		Green = 0x0002,
		Red = 0x0004,
		Bright = 0x0008,

		Cyan = Blue | Green,
		Magenta = Blue | Red,
		Yellow = Green | Red,
		White = Blue | Green | Red,

		BrightBlue = Bright | Blue,
		BrightGreen = Bright | Green,
		BrightRed = Bright | Red,
		BrightCyan = Bright | Cyan,
		BrightMagenta = Bright | Magenta,
		BrightYellow = Bright | Yellow,
		BrightWhite = Bright | White,
	};
}



class XMQClient : public gloox::ConnectionListener, gloox::LogHandler, gloox::IqHandler, gloox::MessageHandler
{
	enum State
	{
		XMQ_NOTCONNECTED,
		XMQ_CONNECTING,
		XMQ_CONNECTED,
		XMQ_DISCONNECTED,
	};

	std::unique_ptr<gloox::Client> client;
	std::string xmppDomain;
	int id;
	State state;


	virtual void onConnect() override;
	virtual void onDisconnect(gloox::ConnectionError e) override;
	virtual bool onTLSConnect(const gloox::CertInfo& info) override;
	virtual void handleLog(gloox::LogLevel level, gloox::LogArea area, const std::string& message) override;
	virtual bool handleIq(const gloox::IQ& iq) override;
	virtual void handleIqID(const gloox::IQ& iq, int context) override;
	virtual void handleMessage(const gloox::Message& msg, gloox::MessageSession* session) override;

	std::string genID();

public:
	
	XMQClient();
	~XMQClient();


	bool Connect(const char* nodeId, const char* domain, const char* resource, const char* password, int port);


	bool Tick();

	void Subscribe(const char* queue, const char* topic, int ttl, int load = 0);
	void SendMessageToQueue(const char* queue, const char* topic, const char* message, const char* subject, const char* thread);
	void SendCustomStanzaToQueue(const char* queue, const char* topic, const char* customQueryData);


	static void WriteLine(const char* message, ConsoleColor::Type background, ConsoleColor::Type foreground);

};


#endif