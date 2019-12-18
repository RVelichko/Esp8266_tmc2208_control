#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <TMC2208Stepper.h>

#include "ControlWebServer.hpp"

const char* ssid = "wifi";
const char* password = "wifipassword";


// Define pins
static const int DIR_PIN  = 5; 
static const int STEP_PIN = 4;
static const int EN_PIN   = 2;

static const int STEP_DELAY = 100;

typedef std::shared_ptr<TMC2208Stepper> PTMC2208Stepper;


class DriverTMC2208 {
    bool _is_f;
    bool _is_b;
    PTMC2208Stepper _drv;

public:
    DriverTMC2208() 
      : _is_f(false)
      , _is_b(false)
    {
        _drv.reset(new TMC2208Stepper(&Serial));
        Serial.println("Start...");
        _drv->push();                // Reset registers

        // Prepare pins
        pinMode(EN_PIN, OUTPUT);
        pinMode(DIR_PIN, OUTPUT);
        pinMode(STEP_PIN, OUTPUT);
        //pinMode(DIR_PIN, HIGH);
        digitalWrite(EN_PIN, HIGH);   // Disable driver in hardware

        _drv->pdn_disable(true);     // Use PDN/UART pin for communication
        _drv->I_scale_analog(false); // Use internal voltage reference
        _drv->rms_current(500);      // Set driver current 500mA
        _drv->toff(2);               // Enable driver in software

        uint32_t data = 0;
        Serial.print("DRV_STATUS = 0x");
        _drv->DRV_STATUS(&data);
        Serial.println(data, HEX);
    }

    void forward() {
        _is_b = false;
        if (not _is_f) {
            _is_f = true;
            digitalWrite(EN_PIN, LOW);    // Enable driver in hardware
            delayMicroseconds(10);
            digitalWrite(DIR_PIN, LOW);
            //_drv->shaft(true);
            #ifdef DEBUG
            Serial.println();
            Serial.println(String("FORWARD"));
            #endif
        } 
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(STEP_DELAY);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(STEP_DELAY);
        //#ifdef DEBUG
        //Serial.print(String("."));
        //#endif
    }

    void backward() {
        _is_f = false;
        if (not _is_b) {
            _is_b = true;
            digitalWrite(EN_PIN, LOW);    // Enable driver in hardware
            delayMicroseconds(10);
            digitalWrite(DIR_PIN, HIGH);
            //_drv->shaft(true);
            #ifdef DEBUG
            Serial.println();
            Serial.println(String("BACKWARD"));
            #endif
        }
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(STEP_DELAY);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(STEP_DELAY);
        //#ifdef DEBUG
        //Serial.print(String("."));
        //#endif
    }

    void stop() {
        _is_f = false;
        _is_b = false;
        digitalWrite(DIR_PIN, LOW);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(10);
        digitalWrite(EN_PIN, HIGH);    // Enable driver in hardware
        delayMicroseconds(10);
    }
};


typedef std::shared_ptr<DriverTMC2208> PDriverTMC2208;
PDriverTMC2208 __driver;


void setup() {
    Serial.begin(115200);
    #ifdef DEBUG
    Serial.println("Setup Motor control...");
    #endif
    //Rtc.Begin();
    mdrv::ControlWebServer::getPtr();
    delay(10);

    __driver.reset(new DriverTMC2208());
}


void loop() {
    while(true) {
        auto cws = mdrv::ControlWebServer::getPtr();
        if (cws and __driver) {
            if (cws->update() == mdrv::ControlWebServer::Moving::FORWARD) {
                __driver->forward();
            }
            if (cws->update() == mdrv::ControlWebServer::Moving::BACKWARD) {
                __driver->backward();
            }
            if (cws->update() == mdrv::ControlWebServer::Moving::STATE_OF_REST) {
                __driver->stop();
            }
        }
        yield();
        //delay(50);
    }
}



// void setup() {
//   Serial.begin(115200);
//   delay(1);
//   pinMode(Step, OUTPUT); //Step pin as output
//   pinMode(Dir, OUTPUT); //Direcction pin as output
//   digitalWrite(Step, LOW); // Currently no stepper motor movement
//   digitalWrite(Dir, LOW); 

//   // Connect to WiFi network
//   Serial.println();
//   Serial.println();
//   Serial.print("Connecting to ");
//   Serial.println(ssid);

//   WiFi.begin(ssid, password);

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("");
//   Serial.println("WiFi connected");

//   // Start the server
//   server.begin();
//   Serial.println("Server started");

//   // Print the IP address on serial monitor
//   Serial.print("Use this URL to connect: ");
//   Serial.print("http://"); //URL IP to be typed in mobile/desktop browser
//   Serial.print(WiFi.localIP());
//   Serial.println("/");
// }

// void loop() {
//   // Check if a client has connected
//   WiFiClient client = server.available();
//   if (!client) {
//     return;
//   }

//   // Wait until the client sends some data
//   Serial.println("new client");
//   while (!client.available()) {
//     delay(1);
//   }

//   // Read the first line of the request
//   String request = client.readStringUntil('\r');
//   Serial.println(request);
//   client.flush();

//   // Match the request
//   int i=0;
//   int value = LOW;

//   if (request.indexOf("/Command=forward") != -1) { //Move 50 steps forward
//     digitalWrite(Dir, HIGH); //Rotate stepper motor in clock wise direction
//     for( i=1;i<=100;i++){
//       digitalWrite(Step, HIGH);
//       delay(1);
//       digitalWrite(Step, LOW);
//       delay(1);
//     }
//     value = HIGH;
//   }

//   if (request.indexOf("/Command=backward") != -1) { //Move 50 steps backwards
//     digitalWrite(Dir, LOW); //Rotate stepper motor in anti clock wise direction
//     for( i=1;i<=100;i++){
//       digitalWrite(Step, HIGH);
//       delay(1);
//       digitalWrite(Step, LOW);
//       delay(1);
//     }
//     value = LOW;
//   }

//   // Return the response
//   client.println("HTTP/1.1 200 OK");
//   client.println("Content-Type: text/html");
//   client.println(""); // do not forget this one
//   client.println("");
//   client.println("");
//   client.println("stepper motor controlled over WiFi");
//   client.print("Stepper motor moving= ");

//   if(value == HIGH) {
//     client.print("Forward");
//   } else {
//     client.print("Backward");
//   }
//   client.println("");
//   client.println("");
//   delay(1);
//   Serial.println("Client disonnected");
//   Serial.println(""); 
// }
