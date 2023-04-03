#include <TcBUTTON.h>
#include <Adafruit_INA219.h>
#include "TM1637Display.h"
// Create a button object esc, up, down, enter
#define esc A0
#define up A1
#define down A2
#define enter A3

void EscPressed(void);
void UpPressed(void);
void DownPressed(void);
void EnterPressed(void);

TcBUTTON escButton(esc,EscPressed,NULL);
TcBUTTON upButton(up,UpPressed,NULL);
TcBUTTON downButton(down,DownPressed,NULL);
TcBUTTON enterButton(enter,EnterPressed,NULL);

//*********************** INPUT Sensor ***********************//
/* 
 * Up to 4 boards may be connected. Addressing is as follows:
 * Board 0: Address = 0x40 Offset = binary 00000 (no jumpers required)
 * Board 1: Address = 0x41 Offset = binary 00001 (bridge A0 as in the photo above)
 * Board 2: Address = 0x44 Offset = binary 00100 (bridge A1)
 * Board 3: Address = 0x45 Offset = binary 00101 (bridge A0 & A1)
*/
Adafruit_INA219 ina219_A(0x40);

const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_C | SEG_E | SEG_G,                          // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G           // E
};

const uint8_t SEG_PASS[] = {
  SEG_A | SEG_B | SEG_G | SEG_E | SEG_F,          // P
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
  SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,          // S
  SEG_A | SEG_F | SEG_G | SEG_C | SEG_D   // S.
};

const uint8_t SEG_FAIL[] = {
  SEG_A | SEG_F | SEG_G | SEG_E,                  // F
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
  SEG_E | SEG_F,                                  // I
  SEG_D | SEG_E | SEG_F,                          // L

};

const uint8_t SEG_WAIT[] = {
  SEG_G,  // -
  SEG_G,  // -
  SEG_G,  // -
  SEG_G,  // -
};

// TM1637 Display
#define CLK 5
#define DIO 6
TM1637Display display(CLK, DIO);



void setup() { 
    Serial.begin(9600);
    Serial.println("Start");
    ina219_A.begin();

    display.setBrightness(5);
    display.setSegments(SEG_WAIT);
}

void loop() {
    escButton.update();
    upButton.update();
    downButton.update();
    enterButton.update();

    double current = ina219_A.getCurrent_mA();
    double voltage = ina219_A.getBusVoltage_V();
    double power = ina219_A.getPower_mW();
    double shuntvoltage = ina219_A.getShuntVoltage_mV();
    double loadvoltage = voltage + (shuntvoltage / 1000);
    // Serial Plotter
    // Serial.print("Bus_Voltage:"); Serial.print(voltage); Serial.println(",");
    // Serial.print("Shunt_Voltage:"); Serial.print(shuntvoltage); Serial.println(",");
    // Serial.print("Load_Voltage:"); Serial.print(loadvoltage); Serial.println(",");
    // Serial.print("Current:"); Serial.print(current); Serial.println(",");
    // Serial.print("Power:"); Serial.print(power); Serial.println(",");
    // Serial.println(" ");          
    // delay(300);
}

void EscPressed(void)
{
  Serial.println("Esc pressed");
}

void UpPressed(void)
{
  Serial.println("Up pressed");
}

void DownPressed(void)
{
  Serial.println("Down pressed");
     display.setSegments(SEG_FAIL);
}

void EnterPressed(void)
{
  Serial.println("Enter pressed");
     display.setSegments(SEG_WAIT);
}
