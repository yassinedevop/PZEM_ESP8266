#include "main.h"

void setup()
{

    Serial.begin(9600);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
}

void loop()
{
    PZEMUpdate();
    readSwitchStateFromFirebase();
    if (voltage != NAN && current != NAN && power != NAN && energy != NAN && frequency != NAN && pf != NAN)
        if (switchState != NAN)
        {
            sendDataToFirebase();
            if (switchState == 1)
            {
                digitalWrite(2, HIGH);
                Serial.println("Switch is on");
            }
            else
            {
                digitalWrite(2, LOW);
                Serial.println("Switch is off");
            }
        }
    delay(1000);
}