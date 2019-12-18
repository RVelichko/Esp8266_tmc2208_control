#pragma once

#include <string>
#include <memory>

#include <Arduino.h>
#include <time.h>
//#include <RTCtimeUtils.h>
//#include <ArduinoNvs.h>


namespace utils {

static const int BLUE_PIN = 2; 
static const int RED_PIN = 4; 
//static time_t utc_time = 0;


//int64_t GetTime() {
//	struct timeval tv;
//	gettimeofday(&tv, NULL);
//	return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
//}

struct WifiConfig {
    String ssid;
    String pswd;
    
    WifiConfig() 
        : ssid({""}) //"wifi"})
        , pswd({""}) //"wifipassword"})
    {}
};


template <int LED_PIN>
class Blink {
    bool _is_on;

    void init() {
        static bool is_init = false;
        if (not is_init) {
            pinMode(LED_PIN, OUTPUT);
            delay(10);
            is_init = true;
        }
    }
public:
    static Blink* get() {
        static Blink blk;
        return &blk;
    }

    Blink() 
        : _is_on(false) {
        init();
    }

    ~Blink() {
        off();
    }

    void on() {
        /// Зажечь светодиод.
        if (not _is_on) {
            _is_on = true;
            digitalWrite(LED_PIN, HIGH);
        }  
    }

    void off() {
        /// Погасить светодиод.  
        if (_is_on) {
            delay(10);
            digitalWrite(LED_PIN, LOW);
            _is_on = false;
        }
    }
};


class ErrorLights {
    static constexpr int POINT_DT = 60;     
    static constexpr int LINE_DT = 400;     

    template<int LED_PIN>
    void blinkTime(int dt) {
        Blink<LED_PIN>::get()->on();
        delay(dt);
        Blink<LED_PIN>::get()->off();
        delay(POINT_DT);
    }

public:
    static ErrorLights* get() {
        static ErrorLights el;
        return &el;
    }

    ErrorLights() 
    {}

    void error() {
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
    }

    void warning() {
        blinkTime<RED_PIN>(LINE_DT);
        blinkTime<RED_PIN>(LINE_DT);
        blinkTime<RED_PIN>(LINE_DT);
        blinkTime<RED_PIN>(LINE_DT);
        blinkTime<RED_PIN>(LINE_DT);
    }

    ~ErrorLights() 
    {}
};


/**
 * \brief Структура реализует разбор URL на составляющие.
 */ 
struct Url {
    Url(const String& url) 
        : port(0) {
        #ifdef DEBUG
        Serial.println("Input URL: \"" + url + "\"");
        #endif
        String s(url);
        int iprotocol = s.indexOf("://");
        if (iprotocol not_eq -1) {
            protocol = s.substring(0, iprotocol);
            s = s.substring(iprotocol + 3);
        }
        int iport = s.indexOf(":");
        int ipath = s.indexOf("/");
        int iquery = s.indexOf("?");
        if (iport not_eq -1) {
            host = s.substring(0, iport);
            String sp = s.substring(iport + 1);
            port = static_cast<uint16_t>(sp.toInt());
            if (ipath not_eq -1) {
                if (iquery not_eq -1) {
                    path = s.substring(ipath, iquery);
                    query = s.substring(iquery);
                } else {
                    path = s.substring(ipath);
                }
            }
        } else if (ipath not_eq -1) {
            host = s.substring(0, ipath);
            if (iquery not_eq -1) {
                path = s.substring(ipath, iquery);
                query = s.substring(iquery);
            } else {
                path = s.substring(ipath);
            }
        } else {
            host = s;
        }
        #ifdef DEBUG
        Serial.println("proto: \"" + protocol + "\"; host: \"" + host + "\"; port: \"" + String(port, DEC) + "\"; path: \"" + path + "\"; query: \"" + query + "\"");
        #endif
    }

    String protocol;
    String host;
    uint16_t port;
    String path;
    String query;
};


/**
 * \brief Структура экранирует двойные кавычки в строке. 
 */ 
struct EscapeQuotes {
private:
    String _out;

public:
    EscapeQuotes(const String &in) {
        int b = 0;
        int pos = 0;
        do {
            pos = in.indexOf('"', pos);
            if (pos not_eq -1) {
                _out += in.substring(b, pos);
                _out += "\\\"";
                b = (++pos); 
            } else {
                _out += in.substring(b);
            }
        } while (pos not_eq -1);
    }

    operator String () {
        return _out;
    }
};


/**
 * \brief Структура хранения настроек контроллера, конфигурируемых по Wifi точке доступа.
 */ 
struct DeviceConfig {
    String uid;       
    WifiConfig wc;      
    String service_url; 
    String token;       
    time_t send_timeout;       
};


// /**
//  * \brief Класс обработки энергонезависимой памяти контроллера.
//  */ 
// struct Nvs {
//     typedef std::unique_ptr<Nvs> PNvs;

//     /**
//      * \brief Метод возвращает статичный указатель объект обработчик постоянной памяти.
//      */ 
//     static Nvs* get() {
//         static PNvs _nvs;
//         if (not _nvs) {
//             _nvs.reset(new Nvs());
//         }
//         return _nvs.get();
//     }

//     /**
//      * \brief Метод выполняет преобразование числового идентификатора устройства в строковое представление.
//      */ 
//     static String idToStr(uint64_t did) {
//         return String((unsigned long)((did & 0xFFFF0000) >> 16 ), DEC) + String((unsigned long)((did & 0x0000FFFF)), DEC);
//     } 

//     /**
//      * \brief Конструктор инициализирует доступ к постоянной памяти.
//      */ 
//     Nvs() {
//         NVS.begin();
//     }

//     ~Nvs() {
//         NVS.close();
//     }

//     /**
//      * \brief Метод выполняет установку флага.
//      */ 
//     void setFlag(const String &name, uint32_t flag) {
//         NVS.setInt(name, flag);
//     }

//     /**
//      * \brief Метод выполняет возврат флага.
//      */ 
//     uint32_t getFlag(const String &name) {
//         return NVS.getInt(name);
//     }

//     /**
//      * \brief Метод выполняет запись идентификатора (должен выполняться 1 раз).
//      */ 
//     void setUid(const String& uid) {
//         #ifdef DEBUG
//         Serial.println("$ Set uid: " +uid);
//         #endif
//         NVS.setString("uid", uid);
//     }

//     /**
//      * \brief Метод возвращает идентификатор контроллера.
//      */ 
//     String getUid() {
//         String uid = NVS.getString("uid");    
//         #ifdef DEBUG
//         Serial.println("$ Get uid: " + uid);
//         #endif
//         return uid;
//     }

//     void setWifiSsid(const String& ssid) {
//         NVS.setString("ssid", ssid);
//         #ifdef DEBUG
//         Serial.println("$ Set SSID: " + ssid);
//         #endif
//     }

//     String getWifiSsid() {
//         String ssid = NVS.getString("ssid");
//         #ifdef DEBUG
//         Serial.println("$ Get SSID: " + ssid);
//         #endif
//         return ssid;
//     }

//     void setWifiPswd(const String& pswd) {
//         NVS.setString("pswd", pswd);
//         #ifdef DEBUG
//         Serial.println("$ Set PSWD: " + pswd);
//         #endif
//     }

//     String getWifiPswd() {
//         String pswd = NVS.getString("pswd");
//         #ifdef DEBUG
//         Serial.println("$ Get PSWD: " + pswd);
//         #endif
//         return pswd;
//     }

//     void setServiceUrl(const String& url) {
//         NVS.setString("url", url);
//         #ifdef DEBUG
//         Serial.println("$ Set service URL: " + url);
//         #endif
//     }

//     String getServiceUrl() {
//         String url = NVS.getString("url");
//         #ifdef DEBUG
//         Serial.println("$ Get service URL: " + url);
//         #endif
//         return url;
//     }

//     void setToken(const String& token) {
//         NVS.setString("token", token);
//         #ifdef DEBUG
//         Serial.println("$ Set token: " + token);
//         #endif
//     }

//     String getToken() {
//         String token = NVS.getString("token");
//         #ifdef DEBUG
//         Serial.println("$ Get token: " + token);
//         #endif
//         return token;
//     }

//     /**
//      * \brief Метод выполняет запись номера кватриты.
//      * \param apmt  Номер квартиры.
//      */ 
//     void setSendTimeout(time_t timeout) {
//         #ifdef DEBUG
//         Serial.println("$ Set send timeout: " + String(timeout, DEC));
//         #endif
//         NVS.setInt("timeout", static_cast<uint64_t>(timeout));
//     }

//     /**
//      * \brief Метод возвращает номер квартиры.
//      */ 
//     time_t getSendTimeout() {
//         time_t timeout = static_cast<time_t>(NVS.getInt("timeout"));
//         #ifdef DEBUG
//         Serial.println("$ Get send timeout: " + String(timeout, DEC));
//         #endif
//         return timeout;
//     }
//};
}