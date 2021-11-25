#pragma once
#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <WiFiClient.h>
#include <WiFiServer.h>

#define CLIENT_TIMEOUT_SEC 5
#define HTTP_REQUEST_SIZE 256  // max. 8192

class ServerWrapper {
   private:
	// private varibales
	WiFiServer server;

	// private functions
	void httpBadRequest(WiFiClient client, const char *request);
	void httpOK(WiFiClient client);
	bool readLine(WiFiClient client, char *buf, size_t len);

   public:
	// public functions
	ServerWrapper();
	void init();

	WiFiClient accept();
	void handleConnection(WiFiClient client, void (*send)(WiFiClient));
	bool isAvailable(WiFiClient client);
};

#endif	// WEBSERVER_HPP