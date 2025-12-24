#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10);   // D9, D10
const byte address[6] = "RADIO";


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

void setup() {
  Serial.begin(9600);

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
  }
}

