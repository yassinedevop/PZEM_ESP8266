#include "main.h"

void setup()
{
    Serial.begin(9600);
    lcd.init(); // initialize the lcd
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    pinMode(15, OUTPUT);
    pinMode(btn1, INPUT);
    pinMode(btn2, INPUT);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lcd.println("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        lcd.print(".");
        delay(300);
    }
    lcd.clear();
    lcd.println("Connected with IP: ");
    lcd.println(WiFi.localIP());
    Serial.println();
    timeClient.begin();
    delay(1000);
    lcd.clear();
}

int waitTime = 0;

void loop()
{
    if (millis() - waitTime > 2000)
    {
        PZEMUpdate();
        readSwitchStateFromFirebase();
        if (voltage != NAN && current != NAN && power != NAN && energy != NAN && frequency != NAN && pf != NAN)
            if (switchState != NAN)
            {
                sendDataToFirebase();
                if (switchState == 1)
                {
                    digitalWrite(15, HIGH);
                    Serial.println("Switch is on");
                }
                else
                {
                    digitalWrite(15, LOW);
                    Serial.println("Switch is off");
                }
            }

        waitTime = millis();
        count++;
        lcd.clear();
    }
    ShowData();
    checkButton();
    clampCount();
}
