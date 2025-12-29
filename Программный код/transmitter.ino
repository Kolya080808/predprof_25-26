#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10);   // D9, D10

const byte address[] = "RADIO"; 
const uint8_t DEVICE_ID = 1;

const uint8_t buttonPin = 2;

/* ---------- ПАКЕТЫ ---------- */
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

const unsigned long DELAY_TIME = 3000; // 3 секунды

/* ---------- РЕЖИМЫ ---------- */
enum Mode {
  MODE_WAIT  = 0,
  MODE_HOR   = 1,
  MODE_VER   = 2,
  MODE_DIAG1 = 3,
  MODE_DIAG2 = 4
};

/* ---------- СОСТОЯНИЕ СИСТЕМЫ ---------- */
bool systemEnabled = false;
bool lastButtonState = HIGH;

/* ---------- ОТПРАВКА С ACK ---------- */
void sendPacket(int8_t pitch, int8_t yaw, uint8_t mode) {
  packet.id = DEVICE_ID;
  packet.pitch = pitch;
  packet.yaw = yaw;
  packet.mode = mode;

  bool ackReceived = false;

  while (!ackReceived && systemEnabled) {
    bool success = radio.write(&packet, sizeof(packet));

    Serial.print("Sent: ID=");
    Serial.print(packet.id);
    Serial.print(" pitch=");
    Serial.print(packet.pitch);
    Serial.print(" yaw=");
    Serial.print(packet.yaw);
    Serial.print(" mode=");
    Serial.println(packet.mode);

    if (success && radio.isAckPayloadAvailable()) {
      radio.read(&ack, sizeof(ack));
      Serial.print("ACK received from ID=");
      Serial.print(ack.id);
      Serial.print(" status=");
      Serial.println(ack.status);
      ackReceived = true;
    } else {
      Serial.println("No ACK received, retrying...");
      delay(100);
    }
  }
}

/* ---------- ДВИЖЕНИЯ ---------- */
void waitModeNGoBack() {
  sendPacket(0, 0, MODE_WAIT);
}

void horizontalScan() {
  sendPacket(0, -40, MODE_HOR);
  sendPacket(0,  40, MODE_HOR);
}

void verticalScan() {
  sendPacket(-40, 0, MODE_VER);
  sendPacket( 40, 0, MODE_VER);
}

void diagonalScan1() {
  sendPacket(-40, -40, MODE_DIAG1);
  sendPacket( 40,  40, MODE_DIAG1);
}

void diagonalScan2() {
  sendPacket(-40,  40, MODE_DIAG2);
  sendPacket( 40, -40, MODE_DIAG2);
}

/* ---------- SETUP ---------- */
void setup() {
  Serial.begin(9600);

  pinMode(buttonPin, INPUT_PULLUP);

  Serial.println("Button started.");
  
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.enableAckPayload();
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("NRF24 transmitter started.");
  Serial.println("All modules initialized.");
  Serial.println("Waiting for button...");
}

/* ---------- LOOP ---------- */
void loop() {

  bool buttonState = digitalRead(buttonPin);

  if (lastButtonState == HIGH && buttonState == LOW) {
    systemEnabled = !systemEnabled;
    Serial.println(systemEnabled ? "SYSTEM ENABLED" : "SYSTEM DISABLED");
    delay(200);
  }
  lastButtonState = buttonState;


  if (!systemEnabled) return;

  waitModeNGoBack();
  delay(DELAY_TIME);

  horizontalScan();
  delay(DELAY_TIME);

  verticalScan();
  delay(DELAY_TIME);

  diagonalScan1();
  delay(DELAY_TIME);

  diagonalScan2();
  delay(DELAY_TIME);

  waitModeNGoBack();
  delay(DELAY_TIME);
}
