#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10);   // D9, D10

const byte address[6] = "RADIO"; // если надо поменяем, я пока так сделал, это канал, так что и на приемнике тоже поменять придется

const uint8_t DEVICE_ID = 1; // тоже если надо поменяем, это вообще не важно

/*
Каждое передаваемое сообщение должно содержать:
● Уникальный идентификатор устройства;
● Текущие углы наклона и поворота;
● Состояние режима работы.

Последовательно пройти три режима:
● Горизонтальный скан: при фиксированном угле 0° по горизонтали — изменение угла по вертикали от -40° до +40°;
● Вертикальный скан: при фиксированном угле 0° по вертикали — изменение по горизонтали от -40° до +40°;
● Диагональный скан: от позиции (-40°, -40°) до (40°, 40°);
● Диагональный скан: от позиции (-40°, 40°) до (40°, -40°).

● Возврат к начальному состоянию и ожидание нового сигнала.
*/

struct Packet {
  uint8_t id;
  int8_t pitch;
  int8_t yaw;
  uint8_t mode;
};

Packet packet;

const int DELAY_TIME = 3000; // 3 секунды по ТЗ

/* ----------- РЕЖИМЫ ----------- */
enum Mode {
  MODE_WAIT  = 0,
  MODE_HOR   = 1,
  MODE_VER   = 2,
  MODE_DIAG1 = 3,
  MODE_DIAG2 = 4
};

/* ----------- ОТПРАВКА ПАКЕТА ----------- */
void sendPacket(int pitch, int yaw, uint8_t mode) {
  packet.id = DEVICE_ID;
  packet.pitch = pitch;
  packet.yaw = yaw;
  packet.mode = mode;

  radio.write(&packet, sizeof(packet));

  Serial.print("Sent: ID=");
  Serial.print(packet.id);
  Serial.print(" pitch=");
  Serial.print(packet.pitch);
  Serial.print(" yaw=");
  Serial.print(packet.yaw);
  Serial.print(" mode=");
  Serial.println(packet.mode);
}

/* ----------- РЕЖИМ ОЖИДАНИЯ ----------- */
void waitModeNGoBack() {
  sendPacket(0, 0, MODE_WAIT);
}

/* ----------- ГОРИЗОНТАЛЬНЫЙ СКАН ----------- */
void horizontalScan() {
  sendPacket(0, -40, MODE_HOR); // начальная точка
  sendPacket(0,  40, MODE_HOR); // конечная точка
}

/* ----------- ВЕРТИКАЛЬНЫЙ СКАН ----------- */
void verticalScan() {
  sendPacket(-40, 0, MODE_VER);
  sendPacket( 40, 0, MODE_VER);
}

/* ----------- ДИАГОНАЛЬ 1 ----------- */
void diagonalScan1() {
  sendPacket(-40, -40, MODE_DIAG1);
  sendPacket( 40,  40, MODE_DIAG1);
}

/* ----------- ДИАГОНАЛЬ 2 ----------- */
void diagonalScan2() {
  sendPacket(-40,  40, MODE_DIAG2);
  sendPacket( 40, -40, MODE_DIAG2);
}

void setup() {
  Serial.begin(9600); // логи

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("NRF24 transmitter started.");
  Serial.println("All modules initialized.");
}

void loop() {

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

