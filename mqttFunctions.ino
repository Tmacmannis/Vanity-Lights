void Task1code(void* pvParameters) {
    TelnetStream.print("Task1 running on core ");
    TelnetStream.println(xPortGetCoreID());

    for (;;) {
        delay(28);
        client.loop();  // takes 60 micro seconds to complete, fast...
        unsigned long currentMillis = millis();

        EVERY_N_MILLISECONDS(20000) {
            checkTime();
        }

        EVERY_N_MILLISECONDS(500) {
            if (ledsOn) {
                client.publish("undermastervanity/OnOffState", "ON");
            } else {
                client.publish("undermastervanity/OnOffState", "OFF");
            }
        }
    }
}

void onConnectionEstablished() {
    client.subscribe("undermastervanity/OnOff", [](const String& payload) {
        TelnetStream.print("onoff payload is: ");
        TelnetStream.println(payload);
        if (payload == "OFF") {
           remoteOff = true;
        } else {
            if (!ledsOn) {
                remoteOn = true;
                ledsOntime = millis();
            }
        }
    });

    client.subscribe("undermastervanity/brightness", [](const String& payload) {
        TelnetStream.print("brightness payload is: ");
        TelnetStream.println(payload);
        brightness = map(payload.toInt(), 3, 255, 0, 255);
        FastLED.setBrightness(brightness);
    });

    client.subscribe("undermastervanity/hue", [](const String& payload) {
        TelnetStream.print("hue payload is: ");
        TelnetStream.println(payload);
        hue = payload.toInt();
    });

    client.subscribe("undermastervanity/saturation", [](const String& payload) {
        TelnetStream.print("saturation payload is: ");
        TelnetStream.println(payload);
        saturation = payload.toInt();
    });
}

void checkTime() {
    const char* ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = -18000;
    const int daylightOffset_sec = 3600;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    char timeHour[3];
    strftime(timeHour, 3, "%H", &timeinfo);

    int hour = atoi(timeHour);
    TelnetStream.print("Current hour: ");
    TelnetStream.println(hour);

    if (hour >= 22 || hour <= 6) {
        brightness = 50;
    } else {
        brightness = 200;
    }
}