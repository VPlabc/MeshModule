#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

extern AsyncWebServer server;
extern DNSServer dnsServer;

void startWebServer();

#endif // WEBSERVER_H
