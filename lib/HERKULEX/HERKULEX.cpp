#include <Arduino.h>
#include "HardwareSerial.h"
#include <HerkulexServo.h>

#define PIN_SW_RX PB7
#define PIN_SW_TX PB6

HardwareSerial Serial1(USART1); // Serial1 sur UART1 (celui que le Herkulex utilise)
HerkulexServoBus herkulex_bus(Serial1);
HerkulexServo my_servo(herkulex_bus, HERKULEX_BROADCAST_ID);
unsigned long last_update = 0;
unsigned long now = 0;
bool toggle = false;
int pos, pos_angle;

void init_serial_1_for_herkulex()
{
  Serial1.setRx(PIN_SW_RX);
  Serial1.setTx(PIN_SW_TX);
  Serial1.begin(115200);
  my_servo.setTorqueOn(); // turn power on
}

void test_herkulex()
{
  herkulex_bus.update();
  now = millis();

  // pos = my_servo.getPosition();
  // pos_angle = (pos-512)*0.325;
  // Serial.printf("pos : %4d | %4d °\n",pos, pos_angle);

  if ((now - last_update) > 5000)
  {
    // called every 1000 ms
    if (toggle)
    {
      // move to -90° over a duration of 560ms, set LED to green
      // 512 - 90°/0.325 = 235
      my_servo.setPosition(512 - 0 / 0.325, 50, HerkulexLed::Green);
      // my_servo_2.setPosition(512-10/0.325, 50, HerkulexLed::Blue);
      // my_servo_3.setPosition(512+30/0.325, 50, HerkulexLed::Yellow);
    }
    else
    {
      // move to +90° over a duration of 560ms, set LED to blue
      // 512 + 90°/0.325 = 789
      my_servo.setPosition(512 + 45 / 0.325, 50, HerkulexLed::Blue);
      my_servo.setTorqueOn();
      // my_servo_2.setPosition(512+10/0.325, 50, HerkulexLed::Blue);
      // my_servo_3.setPosition(512-30/0.325, 50, HerkulexLed::Yellow);
    }
    last_update = now;
    toggle = !toggle;
  }
}

int detect_id(bool activate)
{
  if (activate)
  {
    herkulex_bus.update();

    uint8_t servos_found = 0;

    for (uint8_t id = 0; id <= 0xFD; id++)
    {
      HerkulexPacket resp;
      bool success = herkulex_bus.sendPacketAndReadResponse(resp, id, HerkulexCommand::Stat);

      if (success)
      {
        servos_found++;

        if (id <= 0x0F)
        {
          Serial.print("0");
        }

        Serial.print(id, HEX);
      }
      else
      {
        Serial.print("--");
      }

      if (((id + 1) % 0x0F) == 0)
      {
        Serial.println();
      }
      else
      {
        Serial.print(" ");
      }
    }

    Serial.println();
    Serial.println("Done!");
    Serial.print("Found ");
    Serial.print(servos_found);
    Serial.println(" servos.");

    return 0xFD;
  }
  else
  {
    return 0;
  }
}

