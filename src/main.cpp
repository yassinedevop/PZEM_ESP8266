#include <SoftwareSerial.h>
#include <ModbusMaster.h>

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>




// Replace with your network credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Replace with your Firebase project's configuration
#define FIREBASE_HOST "your_project_ID.firebaseio.com"
#define FIREBASE_AUTH "your_service_account_key"
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

FirebaseData firebaseData;


// Create a SoftwareSerial object to communicate with the PZEM-004t
SoftwareSerial pzemSerial(5, 4);

// Create a ModbusMaster object to handle the Modbus communication
ModbusMaster node;




void loop() {
  // Send data to Firestore
  Firebase.setFloat(firebaseData, "collection/document/field", 123.45);

  if (Firebase.failed()) {
    Serial.println("Failed to send data to Firestore");
    Serial.println(Firebase.error());
  } else {
    Serial.println("Sent data to Firestore");
  }

  // Wait for 5 seconds
  delay(5000);
}


void setup()
{
  // Set up the serial communication
  Serial.begin(9600);
  pzemSerial.begin(9600);

  // Set up the Modbus communication
  node.begin(1, pzemSerial); // id = 1, Serial = pzemSerial

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Serial.println("Initialized Firebase");


}

void loop()
{
  // Read the voltage from the PZEM-004t
  uint16_t voltage;
  int8_t result = node.readInputRegisters(0x0000, 1);
  if (result == node.ku8MBSuccess)
  {
    voltage = node.getResponseBuffer(0);
    Serial.print("Voltage: ");
    Serial.println(voltage);
  }
  else
  {
    Serial.print("Error reading voltage: ");
    Serial.println(result);
  }

  // Read the current from the PZEM-004t
  uint16_t currentMSB, currentLSB;
  result = node.readInputRegisters(0x0001, 1);
  if (result == node.ku8MBSuccess)
  {
    currentLSB = node.getResponseBuffer(0);
    Serial.print("CurrentMSB: ");
    Serial.println(currentMSB);
    Serial.print("CurrentLSB: ");
    Serial.println(currentLSB);
  }
  else
  {
    Serial.print("Error reading current: ");
    Serial.println(result);
  }
  // now we will send this data to firebase



}
