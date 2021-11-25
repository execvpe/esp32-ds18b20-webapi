#include "ServerWrapper.hpp"

#include "WebServer.h"

#define S_EQUALS(X, Y) (!strcmp(X, Y))
#define S_MATCH_FIRST(X, Y, N) (!strncmp(X, Y, N))

#define CRLF "\r\n"

// private functions

void ServerWrapper::httpBadRequest(WiFiClient client, const char *request) {
	client.printf("HTTP/1.1 400 Bad Request" CRLF
				  "Content-type:text/html; charset=utf-8" CRLF
				  "Connection: close" CRLF
					  CRLF
				  "<html><head>"
				  "<title>400 Bad Request</title>"
				  "</head><body>"
				  "<h1>Bad Request</h1>" CRLF
				  "<p>Your browser sent a request that this server could not understand.</p>" CRLF
				  "<p>The requested line \"%s\" is probably too long." CRLF
				  "(Limit: %i bytes)</p>" CRLF
				  "</body></html>" CRLF,
				  request, HTTP_REQUEST_SIZE);
	client.println();

	Serial.printf("Invalid request: \"%s\" from IP Address ", request);
	Serial.println(client.remoteIP());
}

void ServerWrapper::httpOK(WiFiClient client) {
	// println() terminates a line with CRLF
	client.print("HTTP/1.1 200 OK" CRLF
				 "Content-type:text/html; charset=utf-8" CRLF
				 "Connection: close" CRLF);
	client.println();

	Serial.print("OK ");
	Serial.println(client.remoteIP());
}

bool ServerWrapper::readLine(WiFiClient client, char *buf, size_t len) {
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

// public functions

ServerWrapper::ServerWrapper() {
	server = WiFiServer(80);
}

void ServerWrapper::init() {
	server.begin();
}

WiFiClient ServerWrapper::accept() {
	while (1) {
		WiFiClient client = server.available();

		if (client)
			return client;
	}
}

void ServerWrapper::handleConnection(WiFiClient client, void (*send)(WiFiClient)) {
	char request[HTTP_REQUEST_SIZE];
	if (!readLine(client, request, HTTP_REQUEST_SIZE)) {
		httpBadRequest(client, request);
		return;
	}

	const size_t len = strlen(request);
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
			httpOK(client);
			(*send)(client);
			client.println();  // end the HTTP response
			return;
		}
	}
	Serial.println(2);
	httpBadRequest(client, requestDup);
}

bool ServerWrapper::isAvailable(WiFiClient client) {
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