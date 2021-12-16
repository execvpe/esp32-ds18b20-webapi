#include "SimpleServer.hpp"

#include "StringMacros.hpp"
#include "WebServer.h"

namespace HttpHeaders {
	constexpr char f_badRequest400[] =
		"HTTP/1.1 400 Bad Request" CRLF
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
		"</body></html>" CRLF;

	constexpr char f_movedPermanently301[] =
		"HTTP/1.1 301 Moved Permanently" CRLF
		"Location: %s" CRLF;

	constexpr char f_notFound404[] =
		"HTTP/1.1 404 Not Found" CRLF
		"Content-type:text/html; charset=utf-8" CRLF
		"Connection: close" CRLF
			CRLF
		"<html><head>"
		"<title>404 Not Found</title>"
		"</head><body>"
		"<h1>Not Found</h1><br>"
		"<p>The requested element \"%s\" was not found on this server.</p>"
		"</body></html>" CRLF;

	constexpr char ok200[] =
		"HTTP/1.1 200 OK" CRLF
		"Content-type:text/html; charset=utf-8" CRLF
		"Connection: close" CRLF;

}  // namespace HttpHeaders

// private functions

void SimpleServer::httpBadRequest400(WiFiClient &client, const char *request) {
	client.printf(HttpHeaders::f_badRequest400, request, HTTP_REQUEST_SIZE);
	client.println();

	Serial.printf("[400] Bad request: \"%s\" - Client IP Address ", request);
	Serial.println(client.remoteIP());
}

void SimpleServer::httpMovedPermanently301(WiFiClient &client, const char *location) {
	client.printf(HttpHeaders::f_movedPermanently301, location);

	Serial.printf("[301] Moved Permanently: Redirect to \"%s\" Client - IP Address ", location);
	Serial.println(client.remoteIP());
}

void SimpleServer::httpNotFound404(WiFiClient &client, const char *path) {
	client.printf(HttpHeaders::f_notFound404, path);
	client.println();

	Serial.printf("[404] Not found: Element \"%s\" - Client IP Address ", path);
	Serial.println(client.remoteIP());
}

void SimpleServer::httpOK200(WiFiClient &client, const char *request) {
	// println() terminates a line with CRLF
	client.print(HttpHeaders::ok200);
	client.println();

	Serial.printf("[200] OK: \"%s\" - Client IP Address ", request);
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

void SimpleServer::handleConnection(WiFiClient &client, int32_t (*check)(const char *), void (*send)(WiFiClient &, const char *, int)) {
	char request[HTTP_REQUEST_SIZE];
	if (!readLine(client, request, HTTP_REQUEST_SIZE)) {
		httpBadRequest400(client, request);
		return;
	}

	const size_t len = strlen(request) + 1;
	char requestDup[len];
	memcpy(requestDup, request, len);

	const char *tokens[3];
	tokens[0] = strtok(request, " ");
	if (tokens[0] == NULL || !STRING_MATCH_FIRST(tokens[0], "GET", 3)) {
		httpBadRequest400(client, requestDup);
		return;
	}
	tokens[1] = strtok(NULL, " ");
	tokens[2] = strtok(NULL, " ");

	if (tokens[1] != NULL && tokens[2] != NULL) {
		if (STRING_EQUALS(tokens[2], "HTTP/1.0") || STRING_EQUALS(tokens[2], "HTTP/1.1")) {
			if (STRING_EQUALS(tokens[1], "/")) {	 // root directory
				httpMovedPermanently301(client, "/index.html");
				return;
			}

			int code = (*check)(tokens[1]);
			if (!code) {
				httpNotFound404(client, tokens[1]);
				return;
			}

			httpOK200(client, requestDup);
			(*send)(client, tokens[1], code);
			client.println();  // end the HTTP response
			return;
		}
	}
	httpBadRequest400(client, requestDup);
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