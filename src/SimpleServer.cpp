#include "SimpleServer.hpp"

#include "StringMacros.hpp"
#include "WebServer.h"

// private functions

void SimpleServer::httpBadRequest(WiFiClient &client, const char *request) {
	client.printf("HTTP/1.1 400 Bad Request" CRLF
				  "Content-type:text/html; charset=utf-8" CRLF
				  "Connection: close" CRLF
					  CRLF
				  "<html><head>"
				  "<title>400 Bad Request</title>"
				  "</head><body>"
				  "<h1>Bad Request</h1><br>"
				  "<p>Your browser sent a request that this server could not understand.</p><br>"
				  "<p>The requested line \"%s\" is probably too long.<br>"
				  "(Limit: %i bytes)</p>"
				  "</body></html>" CRLF,
				  request, HTTP_REQUEST_SIZE);
	client.println();

	Serial.printf("[400] Bad request: \"%s\" from IP Address ", request);
	Serial.println(client.remoteIP());
}

void SimpleServer::httpNotFound(WiFiClient &client, const char *path) {
	client.printf("HTTP/1.1 404 Not Found" CRLF
				  "Content-type:text/html; charset=utf-8" CRLF
				  "Connection: close" CRLF
					  CRLF
				  "<html><head>"
				  "<title>404 Not Found</title>"
				  "</head><body>"
				  "<h1>Not Found</h1><br>"
				  "<p>The requested element \"%s\" was not found on this server.</p>"
				  "</body></html>" CRLF,
				  path);
	client.println();

	Serial.printf("[404] Not found: Element \"%s\" from IP Address ", path);
	Serial.println(client.remoteIP());
}

void SimpleServer::httpOK(WiFiClient &client, const char *request) {
	// println() terminates a line with CRLF
	client.print("HTTP/1.1 200 OK" CRLF
				 "Content-type:text/html; charset=utf-8" CRLF
				 "Connection: close" CRLF);
	client.println();

	Serial.printf("[200] OK: \"%s\" from IP Address ", request);
	Serial.println(client.remoteIP());
}

bool SimpleServer::readLine(WiFiClient &client, char *buf, size_t len) {
	for (size_t i = 0; i < len; i++) {
		int c = client.read();
		if (c == '\r' || c == '\n' || c == -1) {
			buf[i] = '\0';
			return true;
		}
		buf[i] = (char) c;
	}
	buf[len - 1] = '\0';
	return false;
}

// public constructors

SimpleServer::SimpleServer() : server(80){};

// public functions

WiFiClient SimpleServer::accept() {
	while (1) {
		WiFiClient client = server.available();

		if (client)
			return client;
	}
}

void SimpleServer::begin() {
	server.begin();
}

void SimpleServer::handleConnection(WiFiClient &client, int (*check)(const char *), void (*send)(WiFiClient &, const char *, int)) {
	char request[HTTP_REQUEST_SIZE];
	if (!readLine(client, request, HTTP_REQUEST_SIZE)) {
		httpBadRequest(client, request);
		return;
	}

	const size_t len = strlen(request) + 1;
	char requestDup[len];
	memcpy(requestDup, request, len);

	const char *tokens[3];
	tokens[0] = strtok(request, " ");
	if (tokens[0] == NULL || !S_MATCH_FIRST(tokens[0], "GET", 3)) {
		Serial.println(1);
		httpBadRequest(client, requestDup);
		return;
	}
	tokens[1] = strtok(NULL, " ");
	tokens[2] = strtok(NULL, " ");

	if (tokens[1] != NULL && tokens[2] != NULL) {
		if (S_EQUALS(tokens[2], "HTTP/1.0") || S_EQUALS(tokens[2], "HTTP/1.1")) {
			int code = (*check)(tokens[1]);
			if (!code) {
				httpNotFound(client, tokens[1]);
				return;
			}
			httpOK(client, requestDup);
			(*send)(client, tokens[1], code);
			client.println();  // end the HTTP response
			return;
		}
	}
	Serial.println(2);
	httpBadRequest(client, requestDup);
}

bool SimpleServer::isAvailable(WiFiClient &client) {
	unsigned long currentTime = millis();
	unsigned long previousTime = currentTime;

	while (client.connected()) {
		if ((currentTime - previousTime) > (CLIENT_TIMEOUT_SEC * 1000))
			return false;

		currentTime = millis();

		if (client.available()) {
			return true;
		}

		client.stop();
	}

	return false;
}