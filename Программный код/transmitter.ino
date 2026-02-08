#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10);
const int buttonPin = 2;

const byte addressTx[] = "CMD01";
const byte addressRx[] = "DATA1";

byte lastbuttonState = 0;
byte nowbuttonstate = 0;
int timetopress = 50;

char messange[] = "start";

struct TelemetryPacket {
  char id[4];
  char mode[2];
  char pitch[4];
  char yaw[4];
};

TelemetryPacket telemetry;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.openWritingPipe(addressTx);
  radio.openReadingPipe(1, addressRx);
  radio.startListening();
}

void loop() {
  nowbuttonstate = digitalRead(buttonPin);
  if(nowbuttonstate && nowbuttonstate != lastbuttonState ){
    if(millis() - timetopress > 50){
      timetopress = millis();
      Serial.println("Button pressed. Sending START.");
      radio.stopListening();
      bool ok = radio.write(&messange, sizeof(messange));
      if (!ok) Serial.println("Ошибка: команда не отправлена!");
      radio.startListening();
    }
  }
  lastbuttonState = nowbuttonstate;

  if (radio.available()) {
    TelemetryPacket newTel;
    radio.read(&newTel, sizeof(newTel));
    Serial.print("RECV | ID: "); Serial.print(newTel.id);
    Serial.print(" | Mode: "); Serial.print(newTel.mode);
    Serial.print(" | Pitch: "); Serial.print(newTel.pitch);
    Serial.print(" | Yaw: "); Serial.println(newTel.yaw);
    
  }
}
