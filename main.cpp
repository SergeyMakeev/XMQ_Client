#include <conio.h>
#include "xmqclient.h"



int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		printf("HELP:\n");
		printf("  xmqclient.exe user resource domain password\n\n");
		printf("For example:\n");
		printf("  xmqclient.exe romeo garden montague.example secret\n\n");
		return -1;
	}

	const char* user = argv[1];
	const char* resource = argv[2];
	const char* domain = argv[3];
	const char* password = argv[4];
	printf("User : %s\n", user);
	printf("Resource : %s\n", resource);
	printf("Domain : %s\n\n", domain);

	std::unique_ptr<XMQClient> client(new XMQClient());

	if (!client->Connect(user, domain, resource, password, 5224))
	{
		XMQClient::WriteLine("Can't connect to XMPP server", ConsoleColor::Black, ConsoleColor::BrightRed);
		return -1;
	}

	XMQClient::WriteLine("Press F1 to help. ESC to exit.", ConsoleColor::Black, ConsoleColor::BrightWhite);

	const char* topic = "topic";

	for (;;)
	{
		if (!client->Tick())
		{
			XMQClient::WriteLine("XMPP connection terminated", ConsoleColor::Black, ConsoleColor::BrightRed);
			break;
		}

		Sleep(10);

		// console window is in focus
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			bool isKeyPressed = false;

			if (GetAsyncKeyState(VK_ESCAPE) < 0)
			{
				XMQClient::WriteLine("ESC pressed. Exiting.", ConsoleColor::Black, ConsoleColor::Red);
				break;
			}

			if (GetAsyncKeyState(VK_F1))
			{
				isKeyPressed = true;

				XMQClient::WriteLine("Help:", ConsoleColor::Black, ConsoleColor::BrightWhite);
				XMQClient::WriteLine("  ESC - Exit", ConsoleColor::Black, ConsoleColor::BrightWhite);
				XMQClient::WriteLine("  F1 - Show this help", ConsoleColor::Black, ConsoleColor::BrightWhite);
				
				XMQClient::WriteLine("  F2 - Subscribe to 'queue1' for 60 sec.", ConsoleColor::Black, ConsoleColor::BrightWhite);
				XMQClient::WriteLine("  F3 - Send message 'Hello queue1' to 'queue1'", ConsoleColor::Black, ConsoleColor::BrightWhite);
				XMQClient::WriteLine("  F4 - Send iq to 'queue1'", ConsoleColor::Black, ConsoleColor::BrightWhite);

				XMQClient::WriteLine("  F5 - Subscribe to 'queue2' for 60 sec.", ConsoleColor::Black, ConsoleColor::BrightWhite);
				XMQClient::WriteLine("  F6 - Send message 'Hello queue2' to 'queue2'", ConsoleColor::Black, ConsoleColor::BrightWhite);
				XMQClient::WriteLine("  F7 - Send iq to 'queue2'", ConsoleColor::Black, ConsoleColor::BrightWhite);
			}

			if (GetAsyncKeyState(VK_F2))
			{
				isKeyPressed = true;
				client->Subscribe("queue1", topic, 60);
				XMQClient::WriteLine("Subscribe to 'queue1'", ConsoleColor::Black, ConsoleColor::BrightWhite);
			}

			if (GetAsyncKeyState(VK_F3))
			{
				isKeyPressed = true;
				client->SendMessageToQueue("queue1", topic, "Hello queue1", "Subject", "Thread");
				XMQClient::WriteLine("Send message to 'queue1'", ConsoleColor::Black, ConsoleColor::BrightWhite);
			}

			if (GetAsyncKeyState(VK_F4))
			{
				isKeyPressed = true;
				client->SendCustomStanzaToQueue("queue1", topic, "this is my custom query data to queue1");
				XMQClient::WriteLine("Send stanza to 'queue1'", ConsoleColor::Black, ConsoleColor::BrightWhite);
			}

			if (GetAsyncKeyState(VK_F5))
			{
				isKeyPressed = true;
				client->Subscribe("queue2", topic, 60);
				XMQClient::WriteLine("Subscribe to 'queue2'", ConsoleColor::Black, ConsoleColor::BrightWhite);
			}

			if (GetAsyncKeyState(VK_F6))
			{
				isKeyPressed = true;
				client->SendMessageToQueue("queue2", topic, "Hello queue2", "Subject", "Thread");
				XMQClient::WriteLine("Send message to 'queue2'", ConsoleColor::Black, ConsoleColor::BrightWhite);
			}

			if (GetAsyncKeyState(VK_F7))
			{
				isKeyPressed = true;
				client->SendCustomStanzaToQueue("queue2", topic, "this is my custom query data to queue2");
				XMQClient::WriteLine("Send stanza to 'queue2'", ConsoleColor::Black, ConsoleColor::BrightWhite);
			}


			if (isKeyPressed)
			{
				Sleep(300);
			}

		}

	}

	printf("Finished. Press any key to exit.\n");
	_getch();
	return 0;
}