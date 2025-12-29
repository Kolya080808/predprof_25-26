#include <SPI.h>
#include <RF24.h>
#include <Servo.h>

const uint8_t laserPin = 2;

const uint8_t startAngle = 90; 

RF24 radio(9, 10);   // D9, D10
const byte address[] = "RADIO";


struct Packet {
  uint8_t id;
  int8_t pitch;
  int8_t yaw;
  uint8_t mode;
};

struct AckPacket {
  uint8_t id;
  uint8_t status; // 1 = принято
};

Packet packet;
AckPacket ack;

Servo servo1;
Servo servo2;

uint8_t servoAngles(int8_t angle) { // сервопривод не умеет поворачиваться на отрицательные углы, или тем более работать в системе координат, так что рассчет будет математическим
  return startAngle + angle;
}

bool inFOV(int8_t angle) {
  if (angle <= 40 && angle >= -40) {
    return true;
  } else {
    return false;
  }
}

void setup() {
  Serial.begin(9600);

  servo1.attach(3);
  servo2.attach(5);
  servo1.write(startAngle);
  servo2.write(startAngle);

  Serial.println("Servo started.");
  
  pinMode(laserPin,OUTPUT);
  digitalWrite(laserPin,HIGH);

  Serial.println("Laser started.");
  
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.enableAckPayload();
  radio.openReadingPipe(0, address);
  radio.startListening();

  Serial.println("NRF24 receiver started.");
  Serial.println("All modules initialized.");
  Serial.println("Waiting for packets...");
}

void loop() {
  if (radio.available()) {
    radio.read(&packet, sizeof(packet));

    Serial.print("Received: ID=");
    Serial.print(packet.id);
    Serial.print(" pitch=");
    Serial.print(packet.pitch);
    Serial.print(" yaw=");
    Serial.print(packet.yaw);
    Serial.print(" mode=");
    Serial.println(packet.mode);

    // формирование подтверждения
    ack.id = packet.id;
    ack.status = 1;

    radio.writeAckPayload(0, &ack, sizeof(ack));
    if (inFOV(packet.pitch) && inFOV(packet.yaw)) {
      servo1.write(servoAngles(packet.yaw));
      servo2.write(servoAngles(packet.pitch));
    } else {
      Serial.println("ANGLES NOT IN FOV!");
    }
  }
}
