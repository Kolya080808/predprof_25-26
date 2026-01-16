#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10);
const uint8_t buttonPin = 2;

const byte addressTx[] = "CMD01";
const byte addressRx[] = "DATA1";

struct CommandPacket {
  uint8_t command;
};

struct TelemetryPacket {
  uint8_t id;
  int8_t pitch;
  int8_t yaw;
  uint8_t mode;
};

CommandPacket cmd;
TelemetryPacket telemetry;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  
  radio.openWritingPipe(addressTx);
  radio.openReadingPipe(1, addressRx);
  
  radio.startListening();
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    delay(50);
    if (digitalRead(buttonPin) == LOW) {
      Serial.println("Button pressed. Sending START.");
      
      radio.stopListening();
      cmd.command = 1;
      radio.write(&cmd, sizeof(cmd));
      radio.startListening();
      
      delay(1000);
    }
  }
  
  if (radio.available()) {
    radio.read(&telemetry, sizeof(telemetry));
    
    Serial.print("RECV | ID: "); Serial.print(telemetry.id);
    Serial.print(" | Mode: "); Serial.print(telemetry.mode);
    Serial.print(" | Pitch: "); Serial.print(telemetry.pitch);
    Serial.print(" | Yaw: "); Serial.println(telemetry.yaw);
  }
}
