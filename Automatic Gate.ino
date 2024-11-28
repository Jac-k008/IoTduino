#include <Keypad.h>
#include <Stepper.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>

#define WIFI_AP ""
#define WIFI_PASSWORD ""
#define TOKEN "" //change it with the authentication token on the thingsboard
#define TB_SERVER "thingsboard.cloud"

constexpr uint16_t MAX_MESSAGE_SIZE = 128U;

WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard device1(mqttClient, MAX_MESSAGE_SIZE);

// Konfigurasi Keypad
const byte ROWS = 4; // empat baris
const byte COLS = 4; // empat kolom
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {25, 33, 32, 12}; // pin untuk baris
byte colPins[COLS] = {13, 14, 27, 26}; // pin untuk kolom
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Konfigurasi Motor Stepper dan Sensor Ultrasonik
#define PIN_TRIG 16 // pin trig ultrasonik
#define PIN_ECHO 17 // pin echo ultrasonik
const int stepperRevolution = 200; // jumlah langkah per revolusi motor stepper
const int stepperSpeed = 100; // kecepatan motor stepper
const int stepPin1 = 18; 
const int stepPin2 = 19; 
const int stepPin3 = 21; 
const int stepPin4 = 22; 
Stepper myStepper(stepperRevolution, stepPin1, stepPin2, stepPin3, stepPin4);
int totalSteps = 0;

// Setup kata sandi
const String password1 = "1122"; //User1
const String password2 = "5656"; //User2
const String password3 = "123A"; //User3
const String password4 = "BCA1"; //User4
String inputPassword = "";

bool passwordVerified = false; 
String CurrentUser = "";

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


void ultrasonik() {
  // Memulai pengukuran baru
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Membaca hasil
  long duration = pulseIn(PIN_ECHO, HIGH);
  int jarak = duration / 58; // konversi durasi ke jarak dalam cm
  Serial.print("Distance in CM: ");
  Serial.println(jarak); // membaca nilai jarak pada serial 

  if (jarak < 20) {
    // Menggerakkan motor stepper
    myStepper.step(stepperRevolution);
    totalSteps += stepperRevolution;
  } else {
    delay(1000);
    myStepper.step(-totalSteps); // Mengembalikan stepper ke posisi awal
    totalSteps = 0; // Reset totalSteps
    passwordVerified = false; // Set password kembali ke false setelah proses selesai
  }
  delay(1000);
}

void Password() {
  char key = keypad.getKey();
  
  if (key) {
    Serial.println(key);
    if (key == '#') { // tombol # digunakan untuk mengkonfirmasi kata sandi
      if (inputPassword == password1) {
        Serial.println("Selamat Datang Rafly");
        CurrentUser = "Rafly";
        passwordVerified = true; 
      } 
      else if (inputPassword == password2){ 
        Serial.println("Selamat Datang Teguh");
        CurrentUser = "Teguh";
        passwordVerified = true;
      } 
      else if (inputPassword == password3){ 
        Serial.println("Selamat Datang Aul");
        CurrentUser = "Aul";
        passwordVerified = true;
      }
      else if (inputPassword == password4){ 
        Serial.println("Selamat Datang Ryan");
        CurrentUser = "Ryan";
        passwordVerified = true;
      }  
      else {
        Serial.println("Kata sandi salah. Access Denied !!!");
        inputPassword = "";
      }
    } else if (key == '*') { 
      inputPassword = "";
      Serial.println("Input Password !!!");
    } else {
      inputPassword += key; // tambahkan input ke kata sandi
    }
  }
  if (passwordVerified) {
    ultrasonik(); // Panggil fungsi checkDistance jika kata sandi terverifikasi
    KirimDataKeCloud();
  }
}

void connectToThingsBoard() {
  if (!device1.connected()) {
    Serial.println("Connecting to ThingsBoard server");
    
    if (!device1.connect(TB_SERVER, TOKEN)) {
      Serial.println("Failed to connect to ThingsBoard");
    } else {
      Serial.println("Connected to ThingsBoard");
    }
  }
}

void KirimDataKeCloud() {
  if (device1.connected() && passwordVerified) {
    String jsonPayload = "{\"user\":\"" + CurrentUser + "\"}";
    if (device1.sendTelemetryJson(jsonPayload.c_str())) {
      Serial.print("Sent to ThingsBoard: User - ");
      Serial.println(CurrentUser);
      //passwordVerified = false;
      //CurrentUser = "";
    } else {
      Serial.println("Failed to send data to ThingsBoard");
    }
  }
  device1.loop();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  myStepper.setSpeed(stepperSpeed); // set kecepatan motor stepper
  ConnectToWifi();
  connectToThingsBoard();
}

void loop (){
  Password();
}
