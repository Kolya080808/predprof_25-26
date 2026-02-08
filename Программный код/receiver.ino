#include <SPI.h>
#include <RF24.h>
#include <Servo.h>

const uint8_t laserPin = 2;
const uint8_t servoPinH = 3;
const uint8_t servoPinV = 5;
const uint8_t startAngle = 90;

RF24 radio(9, 10);
const byte addressRx[] = "CMD01";
const byte addressTx[] = "DATA1";

char messange[8];

struct TelemetryPacket {
  char id[4];
  char mode[2];
  char pitch[4];
  char yaw[4];
};

TelemetryPacket telemetry;
Servo servoYaw, servoPitch;

const uint8_t DEVICE_ID = 101;
const int STEP_DELAY = 3000;

bool isRunning = false;

enum Mode {
  MODE_IDLE   = 0,
  MODE_HOR    = 1,
  MODE_VER    = 2,
  MODE_DIAG1  = 3,
  MODE_DIAG2  = 4,
};

uint8_t getServoAngle(int8_t angle) {
  return startAngle + angle;
}
void setPosition(int8_t pitch, int8_t yaw) {
  servoPitch.write(getServoAngle(pitch));
  servoYaw.write(getServoAngle(yaw));
}
void sendTelemetry(int8_t pitch, int8_t yaw, uint8_t mode) {
  itoa(DEVICE_ID,telemetry.id,10);
  itoa(pitch,telemetry.pitch,10);
  itoa(yaw,telemetry.yaw,10);
  itoa(mode,telemetry.mode,10);
  radio.stopListening();
  radio.write(&telemetry, sizeof(telemetry));
  radio.startListening();
  Serial.print("Sent: M="); Serial.print(mode);
  Serial.print(" P="); Serial.print(pitch);
  Serial.print(" Y="); Serial.println(yaw);
}

void performStep(int8_t pitch, int8_t yaw, uint8_t mode) {
  setPosition(pitch, yaw);
  sendTelemetry(pitch, yaw, mode);
  delay(STEP_DELAY);
}

void runScanSequence() {
  for (int8_t p = -40; p <= 40; p += 10) {
    performStep(p, 0, MODE_HOR);
  }
  for (int8_t y = -40; y <= 40; y += 10) {
    performStep(0, y, MODE_VER);
  }
  for (int8_t i = -40; i <= 40; i += 10) {
    performStep(i, i, MODE_DIAG1);
  }
  for (int8_t i = -40; i <= 40; i += 10) {
    performStep(i, -i, MODE_DIAG2);
  }
  setPosition(0, 0);
  sendTelemetry(0, 0, MODE_IDLE);
}

void setup() {
  Serial.begin(9600);
  pinMode(laserPin, OUTPUT);
  digitalWrite(laserPin, HIGH);
  servoYaw.attach(servoPinH);
  servoPitch.attach(servoPinV);
  setPosition(0, 0);
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.openReadingPipe(1, addressRx);
  radio.openWritingPipe(addressTx);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    radio.read(&messange, sizeof(messange));
    if (!isRunning && String(messange) == "start") {
      isRunning = true;
      runScanSequence();
      isRunning = false;
    }
  }
}
