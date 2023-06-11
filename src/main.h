#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PZEM004Tv30.h>
#include <ArduinoJson.h>
#include <time.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
// we will get Date/Time from NTP server

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
int count = 0;
bool stateBtn1 = false, stateBtn2 = false;
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display
int btn1 = 16, btn2 = 14;
#define WIFI_SSID "Hp"
#define WIFI_PASSWORD "12345678"

unsigned long timeStamp;

PZEM004Tv30 pzem(13, 12); // Software Serial pin 13 (RX) & 12 (TX)

double voltage, current, power, energy, frequency, pf;
int switchState = 0;
const String url = "https://pzem004t-esp12f-default-rtdb.europe-west1.firebasedatabase.app/pzem.json";
String convertEpochToDateTime(unsigned long epochTime);
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
        StaticJsonDocument<400> doc;
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
            switchState = doc["switchState"];
        }
        Serial.println(httpResponseCode);
        Serial.println(response);
    }
    else
    {
        Serial.print("Error on reading GET: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}

// now we will send the readings to firebase
void sendDataToFirebase()
{
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    if (timeClient.update())
    {
        timeStamp = timeClient.getEpochTime();
    }
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    String data = "{\"voltage\": " + String(voltage, 2) + ", \"current\": " + String(current, 2) + ", \"power\": " + String(power, 2) + ", \"energy\": " + String(energy, 2) + ", \"frequency\": " + String(frequency, 2) + ", \"pf\": " + String(pf, 2) + ", \"switchState\": " + String(switchState) + ", \"timeStamp\": " + String(timeStamp) + "}";

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
void clampCount()
{
    if (count > 3)
    {
        count = 0;
    }
    if (count < 0)
    {
        count = 3;
    }
}
void checkButton()
{
    if (digitalRead(btn1) && !stateBtn1)
    {
        count++;
        stateBtn1 = true;
        lcd.clear();
    }
    else if (digitalRead(btn2) && !stateBtn2)
    {
        count--;
        stateBtn2 = true;
        lcd.clear();
    }
    else if (!digitalRead(btn1))
    {
        stateBtn1 = false;
    }
    else if (!digitalRead(btn2))
    {
        stateBtn2 = false;
    }
}
void ShowData()
{
    if (count == 0)
    {
        lcd.setCursor(0, 0);
        lcd.print("Voltage:");
        lcd.print(String(voltage) + "V");
        lcd.setCursor(0, 1);
        lcd.print("Current:");
        lcd.print(String(current) + "A");
    }
    else if (count == 1)
    {

        lcd.setCursor(0, 0);
        lcd.print("frequency:");
        lcd.print(String(frequency) + "Hz");
        lcd.setCursor(0, 1);
        lcd.print("cosphi:");
        lcd.print(String(pf));
    }
    else if (count == 2)
    {

        lcd.setCursor(0, 0);
        lcd.print("energy:");
        lcd.print(String(energy) + "Wh");
        lcd.setCursor(0, 1);
        lcd.print("power:");
        lcd.print(String(power) + "W");
    }
    else if (count == 3)
    {
        lcd.clear();
        lcd.println(convertEpochToDateTime(timeStamp));
    }
}

String convertEpochToDateTime(unsigned long epochTime)
{
    static unsigned char month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    static unsigned char week_days[7] = {4, 5, 6, 0, 1, 2, 3};
    // Thu=4, Fri=5, Sat=6, Sun=0, Mon=1, Tue=2, Wed=3

    unsigned char
        ntp_hour,
        ntp_minute, ntp_second, ntp_week_day, ntp_date, ntp_month, leap_days, leap_year_ind;

    unsigned short temp_days;

    unsigned int
        ntp_year,
        days_since_epoch, day_of_year;

    // Add or substract time zone here.
    epochTime += 3600; // GMT +1 = +3600 seconds

    ntp_second = epochTime % 60;
    epochTime /= 60;
    ntp_minute = epochTime % 60;
    epochTime /= 60;
    ntp_hour = epochTime % 24;
    epochTime /= 24;

    days_since_epoch = epochTime;                   // number of days since epoch
    ntp_week_day = week_days[days_since_epoch % 7]; // Calculating WeekDay

    ntp_year = 1970 + (days_since_epoch / 365); // ball parking year, may not be accurate!

    int i;
    for (i = 1972; i < ntp_year; i += 4) // Calculating number of leap days since epoch/1970
        if (((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0))
            leap_days++;

    ntp_year = 1970 + ((days_since_epoch - leap_days) / 365); // Calculating accurate current year by (days_since_epoch - extra leap days)
    day_of_year = ((days_since_epoch - leap_days) % 365) + 1;

    if (((ntp_year % 4 == 0) && (ntp_year % 100 != 0)) || (ntp_year % 400 == 0))
    {
        month_days[1] = 29; // February = 29 days for leap years
        leap_year_ind = 1;  // if current year is leap, set indicator to 1
    }
    else
        month_days[1] = 28; // February = 28 days for non-leap years

    temp_days = 0;

    for (ntp_month = 0; ntp_month <= 11; ntp_month++) // calculating current Month
    {
        if (day_of_year <= temp_days)
            break;
        temp_days = temp_days + month_days[ntp_month];
    }

    temp_days = temp_days - month_days[ntp_month - 1]; // calculating current Date
    ntp_date = day_of_year - temp_days;

    // -------------------- Printing Results -------------------------------------

    switch (ntp_week_day)
    {

    case 0:
        printf("\nSunday");
        break;
    case 1:
        printf("\nMonday");
        break;
    case 2:
        printf("\nTuesday");
        break;
    case 3:
        printf("\nWednesday");
        break;
    case 4:
        printf("\nThursday");
        break;
    case 5:
        printf("\nFriday");
        break;
    case 6:
        printf("\nSaturday");
        break;
    default:
        break;
    }
    printf(", ");

    switch (ntp_month)
    {

    case 1:
        printf("January");
        break;
    case 2:
        printf("February");
        break;
    case 3:
        printf("March");
        break;
    case 4:
        printf("April");
        break;
    case 5:
        printf("May");
        break;
    case 6:
        printf("June");
        break;
    case 7:
        printf("July");
        break;
    case 8:
        printf("August");
        break;
    case 9:
        printf("September");
        break;
    case 10:
        printf("October");
        break;
    case 11:
        printf("November");
        break;
    case 12:
        printf("December");
    default:
        break;
    }
    return String(ntp_date) + "/" + String(ntp_month) + "/" + String(ntp_year) + " " + String(ntp_hour) + ":" + String(ntp_minute) + ":" + String(ntp_second);
}
