#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#define BLYNK_PRINT Serial

char auth[] = ""; //Token Blynk
char ssid[] = ""; //Wi-fi name
char pass[] = ""; //Wi-fi password

#define PULSE_PIN D2 //gpio4
#define FLOW_CALIBRATION 7 
#define VPIN_TOTAL_LITERS V6
#define VPIN_COUNTING V10
#define VPIN_RESET V4

BlynkTimer timer;
WidgetLCD lcd1(V1);
unsigned int harga ;
int trig = D1;
int echo = D3;
int pompa = D4;
float durasi, jarak;
volatile long pulseCount;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;
float totalLitres;
float totalLitresold;

BLYNK_CONNECTED() { 
Blynk.syncVirtual(VPIN_TOTAL_LITERS); 
}

BLYNK_WRITE(VPIN_TOTAL_LITERS)
{
 totalLitresold = param.asFloat();
}
 
BLYNK_WRITE(VPIN_RESET) { // mereset data water flow sensor dengan tombol push button pada pin V4
  int resetdata = param.asInt();
  if (resetdata == 0) {
  Serial.println("Clearing Data");
  Blynk.virtualWrite(VPIN_TOTAL_LITERS, 0);
  Blynk.virtualWrite(V11, 0);
  flowRate = 0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  totalLitres = 0;
  }
}

BLYNK_WRITE (VPIN_COUNTING) {
  switch (param.asInt())
 {
  case 1 : //Perhitungan Harga Air Kelompok II
 {
 if (totalLitres <= 1) {
 harga = totalLitres*1050;
 Blynk.virtualWrite(V11,"Rp. ", harga);
 }
 if (totalLitres > 1) {
 harga = totalLitres*1050;
 Blynk.virtualWrite(V11,"Rp. ", harga);
 }
 if (totalLitres > 2) {
 harga = totalLitres*1575;
 Blynk.virtualWrite(V11,"Rp. ", harga);
 }
 break;
 }
 case 2 : //Perhitungan Harga Air Kelompok III A
 {
 if (totalLitres <= 1) {
 harga = totalLitres*3550;
 Blynk.virtualWrite(V11,"Rp. ", harga);
 }
 if (totalLitres > 1) {
 harga = totalLitres*4700;
 Blynk.virtualWrite(V11,"Rp. ", harga);
 }
 if (totalLitres > 2) {
 harga = totalLitres*5500;
 Blynk.virtualWrite(V11,"Rp. ", harga);
 }
 break;
 }
 case 3 : //Perhitungan Harga Air Kelompok III B
 {
 if (totalLitres <= 1) {
 harga = totalLitres*4500;
 Blynk.virtualWrite(V11,"Rp. ", harga);
 }
 if (totalLitres > 1) {
 harga = totalLitres*6000;
 Blynk.virtualWrite(V11,"Rp. ", harga);
 }
 if (totalLitres > 2) {
 harga = totalLitres*7450;
 Blynk.virtualWrite(V11,"Rp. ", harga);
 } 
 break ;
 }
 case 4 : //Perhitungan Harga Air Kelompok IV
 {
 if (totalLitres <= 1) {
 harga = totalLitres*6825;
 Blynk.virtualWrite(V11,"Rp. ", harga);
 }
 if (totalLitres > 1) {
 harga = totalLitres*8150;
 Blynk.virtualWrite(V11,"Rp. ", harga);
 }
 if (totalLitres > 2) {
 harga = totalLitres*9800;
  Blynk.virtualWrite(V11,"Rp. ", harga);
 }
 break;
 }
 }
}

void IRAM_ATTR pulseCounter()
{
 pulseCount++;
}

void flow ()
{
 if ((millis() - oldTime) > 1000) // Only process counters once per second
 {
 detachInterrupt(PULSE_PIN);
 flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / FLOW_CALIBRATION;
 oldTime = millis();
 flowMilliLitres = (flowRate / 60) * 1000;
 totalMilliLitres += flowMilliLitres;
 totalLitres = totalLitresold + totalMilliLitres * 0.001;
 unsigned int frac;// Cetak laju aliran untuk detik ini dalam satuan liter / menit
 Serial.print("flowrate : ");
 Serial.print(int(flowRate)); // Cetak bagian integer dari variabel
 Serial.print("."); // Print the decimal point
 frac = (flowRate - int(flowRate)) * 10; // menentukan bagian pecahannya. Pengali 10 memberi kita 1 tempat desimal.
 Serial.print(frac, DEC) ; // mencetak bagian pecahan dari variabel
 Serial.print(" L/min ");
 Serial.print(" Current Liquid Flowing : "); // Cetak jumlah liter yang dialirkan pada detik ini.
 Serial.print(flowMilliLitres);
 Serial.print(" mL/Sec ");
 Serial.print(" Output Liquid Quantity: "); // Cetak total kumulatif liter yang dialirkan sejak memulai.
 Serial.print(totalLitres);
 Serial.println("L");
 pulseCount = 0; // Reset the pulse counter so we can start incrementing again
 attachInterrupt(PULSE_PIN, pulseCounter, FALLING);
 }
}

void Ultrasonik() 
{
 lcd1.clear();
 lcd1.print(0, 0, "Jarak : ");
 digitalWrite(trig, LOW);
 delayMicroseconds(2);
 digitalWrite(trig, HIGH);
 delayMicroseconds(10);
 digitalWrite(trig, LOW);
 durasi = pulseIn(echo, HIGH); // menerima suara ultrasonic
 jarak = durasi*0.034/2 ; // mengubah durasi menjadi jarak (cm)
 Serial.print(jarak);
 Serial.println(" Cm");
 lcd1.print(8, 0, jarak);
 lcd1.print(12, 0, "Cm");
 Blynk.virtualWrite(V5, jarak);
 delay(200);
 
 if (jarak >= 12){
 digitalWrite (pompa, LOW);
 }
 if (jarak < 5) {
 digitalWrite (pompa, HIGH);
 }
 Blynk.virtualWrite(V5, jarak);
 delay(800);
 }

void sendtoBlynk() // In this function we are sending values to blynk server
{
 Blynk.virtualWrite(VPIN_TOTAL_LITERS, totalLitres); 
}

void setup() {
 pinMode(trig, OUTPUT);
 pinMode(echo, INPUT);
 pinMode(pompa, OUTPUT);
 Serial.begin(9600);
 pulseCount = 0;
 flowRate = 0.0;
 flowMilliLitres = 0;
 totalMilliLitres = 0;
 oldTime = 0;
 totalLitresold = 0;
 pinMode(PULSE_PIN, INPUT); // Inisialisasi variabel "PULSE_PIN" sebagai INPUT (pin D5)
 attachInterrupt(PULSE_PIN, pulseCounter, FALLING);
 timer.setInterval(1000L, sendtoBlynk); // send values blynk server every 1 sec
 Blynk.begin(auth, ssid, pass);
}

void loop ()
{
 flow();
 Ultrasonik();
 Blynk.run();
 timer.run();
}


