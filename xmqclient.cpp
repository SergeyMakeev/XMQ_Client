#include "xmqclient.h"
#include <error.h>
#include <assert.h>



#define XmqCommandID (gloox::ExtUser + 0)
#define XmqCommandNS "xmq:command"
#define XmqTopicID (gloox::ExtUser + 1)
#define XmqTopicNS "xmq:topic"

#define CustomStanzaId (gloox::ExtUser + 2)
#define CustomStanzaXmlNS "custom:query"





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CustomStanza : public gloox::StanzaExtension
{
	std::string body;

public:

	CustomStanza(const gloox::Tag* tag = nullptr)
		: gloox::StanzaExtension(CustomStanzaId)
	{
		if (tag != nullptr)
		{
			assert(tag->name() == "custom_stanza");
			body = tag->cdata();
		}
	}

	CustomStanza(const char* _body)
		: gloox::StanzaExtension(CustomStanzaId)
	{
		body = _body;
	}

	const std::string& getBody() const
	{
		return body;
	}

	virtual const std::string& filterString() const
	{
		static const std::string filter = "/iq/custom_stanza[@xmlns='" CustomStanzaXmlNS "']";
		return filter;
	}

	virtual StanzaExtension* newInstance(const gloox::Tag* tag) const
	{
		return new CustomStanza(tag);
	}

	virtual gloox::Tag* tag() const
	{
		gloox::Tag* t = new gloox::Tag("custom_stanza");
		t->setXmlns(CustomStanzaXmlNS);
		t->addCData(body.c_str());
		return t;
	}

	virtual StanzaExtension* clone() const
	{
		CustomStanza* q = new CustomStanza();
		q->body = body;
		return q;
	}
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class XmqCommand : public gloox::StanzaExtension
{
	std::string command;
	std::string topic;
	int ttl;
	int load;

public:

	XmqCommand(const gloox::Tag* tag = nullptr)
		: gloox::StanzaExtension(XmqCommandID)
	{
		if (tag != nullptr)
		{
			assert(tag->name() == "xmq");
			command = tag->findAttribute("command");
			topic = tag->findAttribute("topic");
			ttl = atoi(tag->findAttribute("ttl").c_str());
			load = atoi(tag->findAttribute("load").c_str());
		}
	}

	XmqCommand(const char* _command, const char* _topic, int _ttl, int _load)
		: gloox::StanzaExtension(XmqCommandID)
	{
		command = _command;
		topic = _topic;
		ttl = _ttl;
		load = _load;
	}

	const std::string& getCommand() const
	{
		return command;
	}

	const std::string& getTopic() const
	{
		return topic;
	}

	const int getTTL() const
	{
		return ttl;
	}

	const int getLoad() const
	{
		return load;
	}



	virtual const std::string& filterString() const
	{
		static const std::string filter = "/iq/xmq[@xmlns='" XmqCommandNS "']";
		return filter;
	}

	virtual StanzaExtension* newInstance(const gloox::Tag* tag) const
	{
		return new XmqCommand(tag);
	}

	virtual gloox::Tag* tag() const
	{
		gloox::Tag* t = new gloox::Tag("xmq");
		t->setXmlns(XmqCommandNS);
		t->addAttribute("command", command.c_str());
		t->addAttribute("topic", topic.c_str());
		t->addAttribute("ttl", ttl);
		t->addAttribute("load", load);
		return t;
	}

	virtual StanzaExtension* clone() const
	{
		XmqCommand* q = new XmqCommand();
		q->command = command;
		q->topic = topic;
		q->ttl = ttl;
		q->load = load;
		return q;
	}

};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class XmqTopic : public gloox::StanzaExtension
{
	std::string topic;

public:

	XmqTopic(const gloox::Tag* tag = nullptr)
		: gloox::StanzaExtension(XmqTopicID)
	{
		if (tag != nullptr)
		{
			assert(tag->name() == "xmq");
			topic = tag->findAttribute("topic");
		}
	}

	XmqTopic(const char* _topic)
		: gloox::StanzaExtension(XmqTopicID)
	{
		topic = _topic;
	}

	const std::string& getTopic() const
	{
		return topic;
	}

	virtual const std::string& filterString() const
	{
		static const std::string filter = "/iq/xmq[@xmlns='" XmqTopicNS "']";
		return filter;
	}

	virtual StanzaExtension* newInstance(const gloox::Tag* tag) const
	{
		return new XmqTopic(tag);
	}

	virtual gloox::Tag* tag() const
	{
		gloox::Tag* t = new gloox::Tag("xmq");
		t->setXmlns(XmqTopicNS);
		t->addAttribute("topic", topic.c_str());
		return t;
	}

	virtual StanzaExtension* clone() const
	{
		XmqTopic* q = new XmqTopic();
		q->topic = topic;
		return q;
	}

};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XMQClient::XMQClient()
	: state(XMQ_NOTCONNECTED)
	, id(0)
{

}

XMQClient::~XMQClient()
{

}

std::string XMQClient::genID()
{
	static char buffer[128];
	sprintf_s(buffer, "id%08d", id);
	id++;
	return buffer;
}

bool XMQClient::Connect(const char* nodeId, const char* domain, const char* resource, const char* password, int port)
{
	xmppDomain = domain;

	std::string id = nodeId;
	id += "@";
	id += domain;
	id += "/";
	id += resource;

	gloox::JID jid(id);
	client = std::make_unique<gloox::Client>(jid, password, port);
	if (client == nullptr)
	{
		return false;
	}

	client->setTls(gloox::TLSOptional);
	client->registerConnectionListener(this);

	client->registerMessageHandler(this);

	client->registerStanzaExtension(new CustomStanza());
	client->registerIqHandler(this, CustomStanzaId);

	client->logInstance().registerLogHandler(gloox::LogLevelDebug, gloox::LogAreaAll, this);
	client->disableRoster();

	bool res = client->connect(false);
	if (!res)
	{
		client.reset();
		return false;
	}

	state = XMQ_CONNECTING;
	return true;
}


bool XMQClient::Tick()
{
	if (client == nullptr || state == XMQ_DISCONNECTED || state == XMQ_NOTCONNECTED)
	{
		return false;
	}

	client->recv(100);

	if (client->authed())
	{
		client->sendQueue();
	}
	return true;
}

void XMQClient::onConnect()
{
	WriteLine("Connected to XMPP server.", ConsoleColor::Black, ConsoleColor::Green);
	state = XMQ_CONNECTED;
}

void XMQClient::onDisconnect(gloox::ConnectionError e)
{
	WriteLine("Disconnected from XMPP server.", ConsoleColor::Black, ConsoleColor::BrightRed);
	state = XMQ_DISCONNECTED;
}

bool XMQClient::onTLSConnect(const gloox::CertInfo& info)
{
	WriteLine("Connected to XMPP server. (TLS/SSL enabled)", ConsoleColor::Black, ConsoleColor::BrightGreen);
	return true;
}

void XMQClient::handleLog(gloox::LogLevel level, gloox::LogArea area, const std::string& message)
{
	static char buffer[16384];
	sprintf_s(buffer, "Log: %d, %d, %s", level, area, message.c_str());
	WriteLine(buffer, ConsoleColor::Black, ConsoleColor::Blue);
}

bool XMQClient::handleIq(const gloox::IQ& iq)
{
	if (iq.subtype() == gloox::IQ::IqType::Error)
	{
		WriteLine("Error received", ConsoleColor::Black, ConsoleColor::BrightRed);
		WriteLine(iq.from().full().c_str(), ConsoleColor::Black, ConsoleColor::BrightRed);

		const gloox::Error* error = iq.error();
		if (error != nullptr)
		{
			WriteLine(error->text().c_str(), ConsoleColor::Black, ConsoleColor::BrightRed);
		}
	} else
	{
		const CustomStanza* q = iq.findExtension<CustomStanza>(CustomStanzaId);
		if (q)
		{
			const std::string& body = q->getBody();
			WriteLine("Custom stanza received", ConsoleColor::Black, ConsoleColor::BrightGreen);
			WriteLine(iq.from().full().c_str(), ConsoleColor::Black, ConsoleColor::BrightWhite);
			WriteLine(iq.to().full().c_str(), ConsoleColor::Black, ConsoleColor::BrightWhite);
			WriteLine(body.c_str(), ConsoleColor::Black, ConsoleColor::BrightWhite);
		}
	}


	return true;
}

void XMQClient::handleIqID(const gloox::IQ& iq, int context)
{

}

void XMQClient::handleMessage(const gloox::Message& msg, gloox::MessageSession* session)
{
	if (msg.subtype() == gloox::Message::MessageType::Error)
	{
		WriteLine("Error received", ConsoleColor::Black, ConsoleColor::BrightRed);
		WriteLine(msg.from().full().c_str(), ConsoleColor::Black, ConsoleColor::BrightRed);

		const gloox::Error* error = msg.error();
		if (error != nullptr)
		{
			WriteLine(error->text().c_str(), ConsoleColor::Black, ConsoleColor::BrightRed);
		}
	} else
	{
		WriteLine("Message received", ConsoleColor::Black, ConsoleColor::BrightGreen);
		WriteLine(msg.from().full().c_str(), ConsoleColor::Black, ConsoleColor::BrightWhite);
		WriteLine(msg.to().full().c_str(), ConsoleColor::Black, ConsoleColor::BrightWhite);
		WriteLine(msg.subject().c_str(), ConsoleColor::Black, ConsoleColor::BrightWhite);
		WriteLine(msg.thread().c_str(), ConsoleColor::Black, ConsoleColor::BrightWhite);
		WriteLine(msg.body().c_str(), ConsoleColor::Black, ConsoleColor::BrightWhite);
	}
}


void XMQClient::Subscribe(const char* queue, const char* topic, int ttl, int load)
{
	std::string queueAddr;
	queueAddr = queue;
	queueAddr += ".";
	queueAddr += xmppDomain;
	gloox::JID queueJID(queueAddr);

	gloox::IQ iq(gloox::IQ::Set, queueJID, genID());
	iq.addExtension(new XmqCommand("sub", topic, ttl, load));
	client->send(iq);
}

void XMQClient::SendMessageToQueue(const char* queue, const char* topic, const char* message, const char* subject, const char* thread)
{
	std::string queueAddr;
	queueAddr = queue;
	queueAddr += ".";
	queueAddr += xmppDomain;
	gloox::JID queueJID(queueAddr);

	gloox::Message msg(gloox::Message::Chat, queueJID, message, subject, thread);
	msg.addExtension(new XmqTopic(topic));
	client->send(msg);
}


void XMQClient::SendCustomStanzaToQueue(const char* queue, const char* topic, const char* customQueryData)
{
	std::string queueAddr;
	queueAddr = queue;
	queueAddr += ".";
	queueAddr += xmppDomain;
	gloox::JID queueJID(queueAddr);

	gloox::IQ iq(gloox::IQ::Get, queueJID, genID());
	iq.addExtension(new CustomStanza(customQueryData));
	iq.addExtension(new XmqTopic(topic));
	client->send(iq);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XMQClient::WriteLine(const char* message, ConsoleColor::Type background, ConsoleColor::Type foreground)
{
	uint16_t color = uint16_t((background << 8) + foreground);

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE)
	{
		return;
	}
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
	GetConsoleScreenBufferInfo(hConsole, &bufferInfo);
	SetConsoleTextAttribute(hConsole, color);
	printf("%s\n", message);
	SetConsoleTextAttribute(hConsole, bufferInfo.wAttributes);
}
