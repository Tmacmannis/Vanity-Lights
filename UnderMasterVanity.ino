#include <FastLED.h>
#include <credentials.h>

#include "EspMQTTClient.h"
#include "OTA.h"
#include "time.h"

#define DATA_PIN1 32

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 113

CRGB wineLeds[NUM_LEDS];
int sensorPin = 4;
int brightness = 200;

int motionCount = 0;

const long interval = 6000;
unsigned long previousMillis = 0;
unsigned long ledsOntime = 0;

boolean allowReadings = true;
boolean ledsOn = false;
boolean remoteOn = false;
boolean remoteOff = false;

int ledBrightness[NUM_LEDS + 30];

int hue = 50;
int saturation = 140;

EspMQTTClient client(
    mySSID,
    myPASSWORD,
    mqttIP,                // MQTT Broker server ip
    "tim",                 // Can be omitted if not needed
    "14Q4YsC6YrXl",        // Can be omitted if not needed
    "MasterVanityLights",  // Client name that uniquely identify your device
    haPORT                 // The MQTT port, default to 1883. this line can be omitted
);

TaskHandle_t Task1;

void setup() {
    setupOTA("MasterVanityLights", mySSID, myPASSWORD);
    TelnetStream.begin();
    pinMode(sensorPin, INPUT);
    btStop();

    client.enableDebuggingMessages();                                           // Enable debugging messages sent to serial output
    client.enableHTTPWebUpdater();                                              // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
    client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
    client.enableDebuggingMessages(true);

    xTaskCreatePinnedToCore(
        Task1code, /* Task function. */
        "Task1",   /* name of task. */
        10000,     /* Stack size of task */
        NULL,      /* parameter of the task */
        1,         /* priority of the task */
        &Task1,    /* Task handle to keep track of created task */
        0);        /* pin task to core 0 */
    delay(500);

    FastLED.addLeds<LED_TYPE, DATA_PIN1, COLOR_ORDER>(wineLeds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    // FastLED.setMaxPowerInVoltsAndMilliamps(5,3000);
    FastLED.setBrightness(255);

    delay(5000);
}

void loop() {
    EVERY_N_MILLISECONDS(8) {
        handleClientTest();
    }

    EVERY_N_MILLISECONDS(3000) {
        // TelnetStream.print("allowReadings: ");
        // TelnetStream.print(allowReadings);
        // TelnetStream.print("  ledsOn: ");
        // TelnetStream.print(ledsOn);
        TelnetStream.print("  Sat: ");
        TelnetStream.print(saturation);
        TelnetStream.print("  Hue: ");
        TelnetStream.print(hue);
        TelnetStream.print("  Bright: ");
        TelnetStream.println(brightness);
        // TelnetStream.print("  motionCount: ");
        // TelnetStream.print(motionCount);
        // TelnetStream.print("  last on: ");
        // int lastOnTime = (millis() - ledsOntime) / 1000;
        // TelnetStream.println(lastOnTime);
    }

    int val = digitalRead(sensorPin);
    if (val == HIGH && allowReadings) {
        TelnetStream.print("Motion detected: ");
        TelnetStream.println(motionCount++);
        allowReadings = false;
        if (!ledsOn) {
            ledsOn = true;
            fadeOn();
        }

        ledsOntime = millis();
    } else if(remoteOn){
        ledsOn = true;
        fadeOn();
        remoteOn = false;
        ledsOntime = millis();
    } else if(remoteOff){
        turnOff();
        ledsOn = false;
        remoteOff = false;
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        allowReadings = true;
    }

    delay(10);
    ledsOnState();
}

void ledsOnState() {
    if (ledsOn) {
        unsigned long currentMillis = millis();
        if ( currentMillis - ledsOntime > (180 * 1000)) {
            ledsOn = false;
            turnOff();
        }
    }
}

void fadeOn() {
    for (int i = 0; i < NUM_LEDS; i++) {
        wineLeds[i] = CHSV(hue, saturation, brightness / 2);
        FastLED.show();
        delay(5);
    }

    for (int i = brightness / 2; i <= brightness; i++) {
        for (int y = 0; y < NUM_LEDS; y++) {
            wineLeds[y] = CHSV(hue, saturation, i);
        }
        FastLED.show();
        delay(5);
    }
}

void turnOff() {
    for (int i = 0; i < NUM_LEDS; i++) {
        wineLeds[i] = CHSV(0, 0, 0);
    }
    FastLED.show();
}
