#pragma once
#ifndef WIFIHANDLER_HPP
#define WIFIHANDLER_HPP

class WiFiHandler {
   public:
	// public constructors
	WiFiHandler();
	WiFiHandler(const char *hostname);

	// public functions

	void begin();
};

#endif	// WIFIHANDLER_HPP
