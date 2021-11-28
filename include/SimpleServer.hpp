#pragma once
#ifndef SIMPLESERVER_HPP
#define SIMPLESERVER_HPP

#include <WiFiClient.h>
#include <WiFiServer.h>

#define CLIENT_TIMEOUT_SEC 5
#define HTTP_REQUEST_SIZE 256  // max. 8192

#define CRLF "\r\n"

class SimpleServer {
   private:
	// private varibales
	WiFiServer server;

	// private functions
	void httpBadRequest400(WiFiClient &client, const char *request);
	void httpMovedPermanently301(WiFiClient &client, const char *location);
	void httpNotFound404(WiFiClient &client, const char *path);
	void httpOK200(WiFiClient &client, const char *request);

	bool readLine(WiFiClient &client, char *buf, size_t len);

   public:
	// public constructors
	SimpleServer();

	// public functions
	WiFiClient accept();
	void begin();
	void handleConnection(WiFiClient &client, int32_t (*check)(const char *), void (*send)(WiFiClient &, const char *, int));
	bool isAvailable(WiFiClient &client);
};

#endif	// SIMPLESERVER_HPP