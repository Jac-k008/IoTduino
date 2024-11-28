#define pump 8

int readsoil, readpH;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(pump, OUTPUT);
}


void pH(){
  readpH = analogRead(A1);
  Serial.print("pH Tanah : ");
  // Mengkonversi nilai analog menjadi tegangan
  float voltage = readpH * (5.0 / 1023.0);
  // Mengkonversi tegangan menjadi nilai pH
  // Asumsikan rentang tegangan sensor 0V - 5V berkorelasi dengan pH 0 - 14
  float pHValue = (voltage * 10.0) / 5.0;
  Serial.println(pHValue);
  if (pHValue >= 6.5 && pHValue <= 7.8){
    Serial.println("pH Tanah Normal");
    delay(1000);
  }
  else if (pHValue < 6.5 ){
    Serial.println("pH Tanah Asam");
  }
  else{
    Serial.println("pH Tanah Basa");
  }
}

void soil(){
  readsoil = analogRead(A0);
  Serial.print("Kelembaban Tanah : ");
  // Mengkonversi nilai analog menjadi tegangan
  float voltage1 = readsoil * (5.0 / 1023.0);
  // Mengkonversi tegangan menjadi nilai pH
  // Asumsikan rentang tegangan sensor 0V - 5V berkorelasi dengan pH 0 - 100
  float soilValue = (voltage1 * 100.0) / 5.0;
  Serial.print(soilValue);
  Serial.println("%");
  if (soilValue < 40){
    Serial.println("Tanah Kering");
    digitalWrite(pump, HIGH);
    delay(1000);
  }
  else if (soilValue >= 40 && soilValue <= 70){
    Serial.println("Tanah Normal");
    digitalWrite(pump, LOW);
    delay(1000);
  }
  else{
    Serial.println("Tanah Basah");
    digitalWrite(pump, LOW);
    delay(1000);
  }
}
void loop() {
  soil();
  pH();
  delay(1000);
}
