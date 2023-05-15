#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PZEM004Tv30.h>
#include <ArduinoJson.h>
#define WIFI_SSID "Hp"
#define WIFI_PASSWORD "12345678"

PZEM004Tv30 pzem(5, 4); // Software Serial pin 5 (RX) & 4 (TX)

float voltage, current, power, energy, frequency, pf;
int switchState = NAN;
const String url = "https://pzem004t-esp12f-default-rtdb.europe-west1.firebasedatabase.app/pzem.json";

void PZEMUpdate()
{
    voltage = pzem.voltage();
    if (voltage != NAN)
    {
        Serial.print("Voltage: ");
        Serial.print(voltage);
        Serial.println("V");
    }
    else
    {
        Serial.println("Error reading voltage");
    }
    current = pzem.current();
    if (current != NAN)
    {
        Serial.print("Current: ");
        Serial.print(current);
        Serial.println("A");
    }
    else
    {
        Serial.println("Error reading current");
    }
    power = pzem.power();
    if (current != NAN)
    {
        Serial.print("Power: ");
        Serial.print(power);
        Serial.println("W");
    }
    else
    {
        Serial.println("Error reading power");
    }
    energy = pzem.energy();
    if (current != NAN)
    {
        Serial.print("Energy: ");
        Serial.print(energy, 3);
        Serial.println("kWh");
    }
    else
    {
        Serial.println("Error reading energy");
    }
    frequency = pzem.frequency();
    if (current != NAN)
    {
        Serial.print("Frequency: ");
        Serial.print(frequency, 1);
        Serial.println("Hz");
    }
    else
    {
        Serial.println("Error reading frequency");
    }
    pf = pzem.pf();
    if (current != NAN)
    {
        Serial.print("PF: ");
        Serial.println(pf);
    }
    else
    {
        Serial.println("Error reading power factor");
    }
    Serial.println();
    delay(2000);
}
void readSwitchStateFromFirebase()
{
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0)
    {
        String response = http.getString();
        // parse json the response
        StaticJsonDocument<200> doc;
        auto err = deserializeJson(doc, response);
        if (err)
        {
            Serial.print("deserializeJson() failed with code ");
            Serial.println(err.c_str());
            return;
        }
        else
        {
            Serial.println("JSON parsed successfully!");
            digitalWrite(14, doc["switchState"]);
        }
        Serial.println(httpResponseCode);
        Serial.println(response);
    }
    else
    {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }
}

// now we will send the readings to firebase
void sendDataToFirebase()
{
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    String data = "{\"voltage\": " + String(voltage) + ", \"current\": " + String(current) + ", \"power\": " + String(power) + ", \"energy\": " + String(energy) + ", \"frequency\": " + String(frequency) + ", \"pf\": " + String(pf) + ", \"switchState\": " + String(switchState) + "}";
    int httpResponseCode = http.PUT(data);
    if (httpResponseCode > 0)
    {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
    }
    else
    {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}