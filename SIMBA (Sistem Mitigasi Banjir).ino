#include <WiFi.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>

#define WIFI_AP ""
#define WIFI_PASSWORD ""
#define TOKEN ""  // Ganti dengan token akses dari ThingsBoard
#define TB_SERVER "thingsboard.cloud"

constexpr uint16_t MAX_MESSAGE_SIZE = 256U;

WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

// Pin Definitions
#define PIN_TRIG 16 // pin trig ultrasonik
#define PIN_ECHO 17 // pin echo ultrasonik
#define ONE_WIRE_BUS 4
#define Flow_PIN 33
#define Rain_PIN 35

float readrain, readflow;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
unsigned long startTime = 0;
//unsigned long endTime = 0;
unsigned long rainDuration = 0; // Durasi hujan dalam detik
bool isRaining = false;
float ketinggianAir = 0.0;


void ConnectToWifi()
{
  Serial.println("Connecting to WiFi...");
  int attempts = 0;
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect to WiFi.");
  } else {
    Serial.println("\nConnected to WiFi");
  }
}

void sendToThingsBoard(String key, float value) {
  if (tb.connected()) {
    tb.sendTelemetryData(key.c_str(), value);
    //Serial.print("Sent ");
    //Serial.print(key);
    //Serial.print(": ");
    //Serial.println(value);
  } else {
    Serial.println("Failed to send data to ThingsBoard - not connected");
  }
}

void sendToThingsBoard(String key, String value) {
  if (tb.connected()) {
    tb.sendTelemetryData(key.c_str(), value.c_str());
    //Serial.print("Sent ");
    //Serial.print(key);
    //Serial.print(": ");
    //Serial.println(value);
  } else {
    Serial.println("Failed to send data to ThingsBoard - not connected");
  }
}

void Ultrasonik() {
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Read the result:
  int duration = pulseIn(PIN_ECHO, HIGH);
  ketinggianAir = duration / 58.0;
  Serial.print("Ketinggian Air : ");
  Serial.print(ketinggianAir);
  Serial.println(" CM");

  // Send data to ThingsBoard
  
}

void SensorHujan() {
  int readrain = analogRead(Rain_PIN);
  Serial.print("Nilai analog: ");
  Serial.println(readrain); // Cetak nilai analog untuk debugging
  
  if (readrain >= 512) { // Jika nilai 512 atau lebih, cuaca hujan
    if (!isRaining) {
      // Jika sebelumnya cerah dan sekarang mulai hujan, catat waktu mulai hujan
      startTime = millis();
      isRaining = true;
    } else {
      // Jika sedang hujan, hitung durasi hujan
      unsigned long currentTime = millis();
      unsigned long rainDuration = (currentTime - startTime)/1000; // Durasi dalam milidetik
      Serial.print("Durasi hujan saat ini: ");
      Serial.print(rainDuration);
      Serial.println(" detik");
      sendToThingsBoard("Timer", rainDuration);
      sendToThingsBoard("Ketinggian Air", ketinggianAir);
      sendToThingsBoard("Cuaca", "Hujan");
    }
  } else {
    isRaining = false; // Set status hujan menjadi false
    sendToThingsBoard("Timer", rainDuration);
    sendToThingsBoard("Ketinggian Air", ketinggianAir);
    sendToThingsBoard("Cuaca", "Cerah");
  }
}

void SensorSuhu() {
  sensors.requestTemperatures();
  
  // Check if the sensor is found
  if (!sensors.getAddress(tempDeviceAddress, 0)) {
    Serial.println("Unable to find address for Device 0");
    return;
  }

  // Fetch the temperature in Celsius
  float temperatureC = sensors.getTempC(tempDeviceAddress);

  // Check if the temperature is valid
  if (temperatureC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Could not read temperature data");
    return;
  }

  Serial.print("Suhu Lingkungan : ");
  Serial.print(temperatureC);
  Serial.println(" C");

  sendToThingsBoard("Suhu", temperatureC);
}

void SensorFlow() {
  readflow = analogRead(Flow_PIN);
  Serial.print("Aliran Air : ");
  // Hitung nilai dengan rentang 0 - 4095
  float flowRate = map(readflow, 0, 4095, 0, 60);
  Serial.print(flowRate);
  Serial.println(" L/min");
  // Send data to ThingsBoard
  sendToThingsBoard("Aliran Air", flowRate);
}

void connectToThingsBoard() {
  if (!tb.connected()) {
    Serial.println("Connecting to ThingsBoard server");
    
    if (!tb.connect(TB_SERVER, TOKEN)) {
      Serial.println("Failed to connect to ThingsBoard");
    } else {
      Serial.println("Connected to ThingsBoard");
    }
  }
}

void setup() {
  Serial.begin(115200);
  sensors.begin();
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(Rain_PIN, INPUT);
  pinMode(Flow_PIN, INPUT);

  ConnectToWifi();
  connectToThingsBoard();
}

void loop() {
  Ultrasonik();
  SensorSuhu();
  SensorHujan();
  SensorFlow();
  Serial.println("");
  tb.loop();
  delay(3000);  // Delay 3 seconds between sensor readings
}
