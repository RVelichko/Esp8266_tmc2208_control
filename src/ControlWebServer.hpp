/**
 * \brief Класс реализующий режим конфигурирования. 
 */ 

#pragma once

#include <string>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "utils.hpp"


namespace mdrv {

class ControlWebServer;
typedef std::shared_ptr<ControlWebServer> PControlWebServer;
typedef std::shared_ptr<AsyncWebServer> PAsyncWebServer;


class ControlWebServer {
public:
    enum class Moving {
        STATE_OF_REST,
        FORWARD,
        BACKWARD,
    };

private:
    static bool _is_forward;
    static bool _is_backward;
    static time_t _start_time;

    String _srv_ssid;
    String _srv_pswd;
    PAsyncWebServer _server;

public:
    static PControlWebServer& getPtr(const String& srv_ssid = "motor_control", const String& srv_pswd = "pswd");

    static void handleNotFound(AsyncWebServerRequest *request);
    static void handleRoot(AsyncWebServerRequest *request);
    static void handleForward(AsyncWebServerRequest *request);
    static void handleBackward(AsyncWebServerRequest *request);

    ControlWebServer(const String& srv_ssid = "motor_control", const String& srv_pswd = "pswd");
    ~ControlWebServer();

    Moving update();
};
}