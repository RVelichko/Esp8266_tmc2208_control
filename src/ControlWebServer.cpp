
#include <Arduino.h>
#include <ArduinoJson.h>

#include "utils.hpp"
#include "ControlWebServer.hpp"

using namespace mdrv;
using namespace utils;


static const time_t MOVE_TIMEOUT = 600; ///< Количество секунд работы режима конфигурирования.

// extern const uint8_t index_html_start[]        asm("_binary_src_ControlPage_index_html_gz_start");
// extern const uint8_t index_html_end[]          asm("_binary_src_ControlPage_index_html_gz_end");
// extern const uint8_t favicon_ico_start[]       asm("_binary_src_ControlPage_favicon_ico_start");
// extern const uint8_t favicon_ico_end[]         asm("_binary_src_ControlPage_favicon_ico_end");
// extern const uint8_t bootstrap_min_css_start[] asm("_binary_src_ControlPage_bootstrap_min_css_gz_start");
// extern const uint8_t bootstrap_min_css_end[]   asm("_binary_src_ControlPage_bootstrap_min_css_gz_end");
// extern const uint8_t main_js_start[]           asm("_binary_src_ControlPage_main_js_gz_start");
// extern const uint8_t main_js_end[]             asm("_binary_src_ControlPage_main_js_gz_end");
// 
// static const char favicon_str[] = "/favicon.ico";
// static const char bootstrap_min_css_str[] = "/bootstrap.min.css";
// static const char main_js_str[] = "/main.js";

static const char PAGE[] = 
"<!DOCTYPE html>\n"
"<html>\n"
"    <head>\n"
"        <title>Motor Controller</title>\n"
"        <meta charset='utf-8'>\n"
"        <meta name='viewport' content='width=device-width, initial-scale=1'>\n"
"        <link rel='stylesheet' href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.0/css/bootstrap.min.css\">\n"
"    </head>\n"
"    <style>\n"
"        html{width:100%;height:100%;}\n"
"        body{font-family:'Roboto',Times,serif;overflow-x: hidden;}\n"
"        input{margin-bottom:3px;margin-top:0px;}\n"
"        label{padding:5px 10px 5px 10px; margin-bottom:0; margin-left:3px;margin-right:3px;font-size:18pt;align-items:center;justify-content:center;height:50%;display:flex;}\n"
"        *{box-sizing:border-box;}\n"
"        .clear{clear:both;}\n"
"        .right{text-align:right;}\n"
"        .left{text-align:left;}\n"
"        .row{margin-right:-5px;margin-left:-5px;}\n"
"        .row:before,.row:after{display:table;content:' ';}\n"
"        .row:after{clear: both;}\n"
"        .col-xs-1,.col-xs-2,.col-xs-4,.col-xs-6,.col-xs-7,.col-xs-8,.col-xs-9,.col-xs-10,.col-xs-11,.col-xs-12{position:relative;min-height:1px;padding-bottom:5px;padding-top:5px;padding-right:10px;padding-left:10px;float:left;align-items:center;}\n"
"        .border{border:1px solid #1b9c1a;border-radius:3px;}\n"
"    </style>\n"
"    <body>\n"
"        <div class='col-xs-12'>\n"
"            <div id='esion_container' class='border col-xs-12'>\n"
"                <div class='row col-xs-12'>\n"
"                    <div id='esion_label' class='col-xs-10'>\n"
"                        <h2 style='color: green;'>Драйвер мотора</h2>\n"
"                    </div>\n"
"                    <div id='esion_version' class='col-xs-2' style='text-align: right;'>\n"
"                        <h3>V 1.0</h3>\n"
"                    </div>\n"
"                </div>\n"
"                <div id='devices_list_container'class='col-xs-12' >\n"
"                    <div class='border col-xs-12'>\n"
"                        <div class='col-xs-12'>\n"
"                        </div>\n"
"                        <div class='row col-xs-12'>\n"
"                            <div class='row col-xs-3'></div>\n"
"                            <div class='row col-xs-6'>\n"
"                                <div class='btn-group-justified'>\n"
"                                    <div class='btn-group'>\n"
"                                      <button id='btn_backward' type='button' class='btn btn-success'><<< BACKWARD</button>\n"
"                                    </div>\n"
"                                    <div class='btn-group'></div>\n"
"                                    <div class='btn-group'>\n"
"                                      <button id='btn_forward' type='button' class=\"btn btn-primary\">FORWARD >>></button>\n"
"                                    </div>\n"
"                                </div>\n"
"                            </div>\n"
"                            <div class='row col-xs-3'><label id='id_pos_num'>0</label></div>\n"
"                        </div>\n"
"                    </div>\n"
"                </div>\n"
"                <div class='row col-xs-12'>\n"
"                    <div class='row col-xs-2 vcenter'>\n"
"                        <h2 style='color: green;'>status:</h2>\n"
"                    </div>\n"
"                    <div class='row col-xs-10 vcenter'>\n"
"                        <h2 id='status' style='color: green;'>init</label>\n"
"                    </div>\n"
"                </div>\n"
"            </div>\n"
"        </div>\n"
"    </body>\n"
"    <script type=\"text/javascript\">\n"
"        document.addEventListener(\"DOMContentLoaded\", function() {\n"
"            console.info(\"Start page\");\n"
"            var is_forward = false;\n"
"            var is_backward = false;\n"
"            var num = 0;\n"
"            document.getElementById('btn_forward').onmousedown = function() {\n"
"                is_forward = true;\n"
"                is_backward = false;\n"
"                console.log('Forw DOWN');\n"
"            }\n"
"            document.getElementById('btn_forward').onmouseup = function() {\n"
"                is_forward = false;\n"
"                console.log('Forw UP');\n"
"            }\n"
"            document.getElementById('btn_backward').onmousedown = function() {\n"
"                is_forward = false;\n"
"                is_backward = true;\n"
"                console.log('Back DOWN');\n"
"            }\n"
"            document.getElementById('btn_backward').onmouseup = function() {\n"
"                is_backward = false;\n"
"                console.log('Back UP');\n"
"            }\n"
"            var timer = setInterval(function() {\n"
"                if (is_forward) {\n"
"                    var xhr = new XMLHttpRequest();\n"
"                    var url = window.location.origin;\n"
"                    xhr.open('GET', url + '/forward', true);\n"
"                    xhr.onload = function() {\n"
"                        var js = JSON.parse(this.response);\n"
"                        console.log(JSON.stringify(js));\n"
"                        var status = document.getElementById('status');\n"
"                        if (typeof js[\"status\"] !== 'undefined') {\n"
"                            status.innerHTML = js.status;\n"
"                        } else {\n"
"                            status.innerHTML = \"ERROR\";\n"
"                        }\n"
"                    };\n"
"                    xhr.send();\n"
"                    document.getElementById('id_pos_num').innerHTML = ++num;\n"
"                };\n"
"                if (is_backward) {\n"
"                    var xhr = new XMLHttpRequest();\n"
"                    var url = window.location.origin;\n"
"                    xhr.open('GET', url + '/backward', true);\n"
"                    xhr.onload = function() {\n"
"                        var js = JSON.parse(this.response);\n"
"                        console.log(JSON.stringify(js));\n"
"                        var status = document.getElementById('status');\n"
"                        if (typeof js[\"status\"] !== 'undefined') {\n"
"                            status.innerHTML = js.status;\n"
"                        } else {\n"
"                            status.innerHTML = \"ERROR\";\n"
"                        }\n"
"                    };\n"
"                    xhr.send();\n"
"                    document.getElementById('id_pos_num').innerHTML = --num;\n"
"                };\n"
"            }, 500);\n"
"        });\n"
"    </script>\n"
"</html>\n";

//typedef StaticJsonDocument<500> JsonBufferType;

bool ControlWebServer::_is_forward = false;
bool ControlWebServer::_is_backward = false;
time_t ControlWebServer::_start_time = 0;


PControlWebServer& ControlWebServer::getPtr(const String& srv_ssid, const String& srv_pswd) {
    static PControlWebServer conf_srv;
    if (not conf_srv) {
        conf_srv.reset(new ControlWebServer(srv_ssid, srv_pswd));
    } 
    return conf_srv;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ControlWebServer::handleNotFound(AsyncWebServerRequest *request) {
    request->send(404);
}


void ControlWebServer::handleForward(AsyncWebServerRequest *request) {
    //#ifdef DEBUG    
    //Serial.println("Handle FORWARD");
    //#endif
    request->send(200, "application/json", "{\"status\":\"ok\"}");
    ControlWebServer::_is_forward = true;
    ControlWebServer::_is_backward = false;
    ControlWebServer::_start_time = micros64() / 1000; 
}


void ControlWebServer::handleBackward(AsyncWebServerRequest *request) {
    //#ifdef DEBUG    
    //Serial.println("Handle BACKWARD");
    //#endif
    request->send(200, "application/json", "{\"status\":\"ok\"}");
    ControlWebServer::_is_forward = false;
    ControlWebServer::_is_backward = true;
    ControlWebServer::_start_time = micros64() / 1000; 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ControlWebServer::ControlWebServer(const String& srv_ssid, const String& srv_pswd) 
    : _srv_ssid(srv_ssid)
    , _srv_pswd(srv_pswd)
    , _server(new AsyncWebServer(80)) {
    delay(10);
    WiFi.begin("wifi", "wifipassword");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    //WiFi.mode(WIFI_AP);
    //IPAddress local_IP(192,168,4,22);
    //IPAddress gateway(192,168,4,1);
    //IPAddress subnet(255,255,255,0);
    //WiFi.softAP(_srv_ssid.c_str(), _srv_pswd.c_str(), 1, 0, 1);
    //delay(10);
    //WiFi.softAPConfig(local_IP, gateway, subnet);
    delay(10);
    
    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        //AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html; charset=UTF-8", index_html_start, index_html_end - index_html_start - 1);
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html; charset=UTF-8", PAGE);
        //response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });
    //_server->on(favicon_str, HTTP_GET, [](AsyncWebServerRequest *request) {
    //    request->send_P(200, "image/x-icon", favicon_ico_start, favicon_ico_end - favicon_ico_start - 1); 
    //});
    //_server->on(main_js_str, HTTP_GET, [](AsyncWebServerRequest *request) {
    //    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", main_js_start, main_js_end - main_js_start - 1);
    //    response->addHeader("Content-Encoding", "gzip");
    //    request->send(response);
    //});
    //_server->on(bootstrap_min_css_str, HTTP_GET, [](AsyncWebServerRequest *request) {
    //    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", bootstrap_min_css_start, bootstrap_min_css_end - bootstrap_min_css_start - 1);
    //    response->addHeader("Content-Encoding", "gzip");
    //    request->send(response);
    //});

    _server->on("/forward", HTTP_GET, handleForward);
    _server->on("/backward", HTTP_GET, handleBackward);
    _server->onNotFound(handleNotFound);
    _server->begin();

    ControlWebServer::_start_time = micros64() / 1000; 

    #ifdef DEBUG
    //Serial.println(String("Control server[ " + srv_ssid + "@" + srv_pswd + " ] IP: \"") + WiFi.softAPIP().toString() + String("\""));
    Serial.println(String("Control server IP: \"") + WiFi.localIP().toString() + String("\""));
    #endif
     
    ControlWebServer::_is_forward = false;
    ControlWebServer::_is_backward = false;
}


ControlWebServer::~ControlWebServer() {
    ControlWebServer::_is_forward = false;
    ControlWebServer::_is_backward = false;

    //Blink<RED_PIN>::get()->off();
    #ifdef DEBUG
    Serial.println("End server");
    #endif
    if(_server) {
        _server->reset();
    }
    delay(100);
    WiFi.disconnect(); // обрываем WIFI соединения
    WiFi.softAPdisconnect(); // отключаем отчку доступа(если она была
    WiFi.mode(WIFI_OFF); // отключаем WIFI
    delay(100);
    #ifdef DEBUG
    Serial.println(String("Control is complete."));
    #endif
}


ControlWebServer::Moving ControlWebServer::update() {
    time_t cur_time = micros64() / 1000; 
    //#ifdef DEBUG
    //Serial.println(String(cur_time, DEC) + " : " + String(_start_time, DEC));
    //#endif
    if (MOVE_TIMEOUT <= (cur_time - ControlWebServer::_start_time)) {
        //Blink<RED_PIN>::get()->off();
        ControlWebServer::_is_forward = false;
        ControlWebServer::_is_backward = false;
        //#ifdef DEBUG
        //Serial.println(String(cur_time, DEC) + " : " + String(_start_time, DEC));
        //#endif
        //return ControlWebServer::Moving::STATE_OF_REST;
    } 
    if (ControlWebServer::_is_backward) {
        //Blink<RED_PIN>::get()->on();
        return ControlWebServer::Moving::FORWARD;
    } 
    if (ControlWebServer::_is_forward) {
        //Blink<RED_PIN>::get()->on();
        return ControlWebServer::Moving::BACKWARD;
    } 
    return ControlWebServer::Moving::STATE_OF_REST;
}
