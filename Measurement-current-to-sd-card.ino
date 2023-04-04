#include <TcBUTTON.h>
#include <Adafruit_INA219.h>
#include <TM1637Display.h>
#include <DS3231.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
DS3231 myRTC;
bool century = false;
bool h12Flag;
bool pmFlag;

// Create a button object esc, up, down, enter
#define esc A0
#define up A1
#define down A2
#define enter A3

void EscPressed(void);
void UpPressed(void);
void DownPressed(void);
void EnterPressed(void);

TcBUTTON escButton(esc, EscPressed, NULL);
TcBUTTON upButton(up, UpPressed, NULL);
TcBUTTON downButton(down, DownPressed, NULL);
TcBUTTON enterButton(enter, EnterPressed, NULL);


#define SD_CS 10
#define SD_MOSI 11
#define SD_MISO 12
#define SD_SCK 13


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
  SEG_A | SEG_F | SEG_G | SEG_C | SEG_D           // S.
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

int menu = 0;
uint8_t subMenu = 3;
bool isMenu = false;

unsigned long lastTime = 0;
unsigned long last_time_ms = 0;
unsigned long last_time_mss = 0;
// Time _time;
uint8_t date = 0;
uint8_t month = 0;
uint16_t year = 0;
uint16_t hour = 0;
uint16_t minute = 0;
uint16_t second = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  ina219_A.begin();
  Wire.begin();
  display.setBrightness(5);
  display.setSegments(SEG_WAIT);


  if (!SD.begin(SD_CS)) {
    Serial.println("SD card not found");
    while (1) {
      display.setBrightness(5, true);
      display.setSegments(SEG_WAIT);
      delay(500);
      display.setBrightness(0, false);
      display.setSegments(SEG_WAIT);
      delay(500);
    }
  }
  // Serial Plotter
  // Serial.print("Bus_Voltage:"); Serial.print(voltage); Serial.println(",");
  // Serial.print("Shunt_Voltage:"); Serial.print(shuntvoltage); Serial.println(",");
  // Serial.print("Load_Voltage:"); Serial.print(loadvoltage); Serial.println(",");
  // Serial.print("Current:"); Serial.print(current); Serial.println(",");
  // Serial.print("Power:"); Serial.print(power); Serial.println(",");
  // Serial.println(" ");
  // delay(300);
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
  myTime();
  menuDisplay();
}
void myTime(void) {
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 1000) {


    lastTime = currentTime;
  } else if (currentTime < 1000) {
    lastTime = currentTime;
  }
}
void EscPressed(void) {
  Serial.println("Esc pressed");
  if (subMenu > 0 && isMenu) {
    subMenu = 0;
  } else {
    isMenu = false;
  }
}

void UpPressed(void) {
  Serial.println("Up pressed");
  if (isMenu) {
    if (menu == 0) {
      switch (subMenu) {
        case 2:
          date++;
          if (date > 31) {
            date = 1;
          } else if (date <= 0) {
            date = 31;
          }
          break;
        case 3:
          month++;
          if (month > 12) {
            month = 1;
          } else if (month <= 0) {
            month = 12;
          }
          break;
        case 4:
          year++;
          if (year > 99) {
            year = 0;
          } else if (year <= 0) {
            year = 99;
          }
          break;
        default:
          menu--;
          if (menu < 0) {
            menu = 1;
          } else if (menu > 1) {
            menu = 0;
          }
          break;
      }
    } else if (menu == 1) {
      switch (subMenu) {
        case 2:
          hour++;
          if (hour > 23) {
            hour = 0;
          } else if (hour <= 0) {
            hour = 23;
          }
          break;
        case 3:
          minute++;
          if (minute > 59) {
            minute = 0;
          } else if (minute <= 0) {
            minute = 59;
          }
          break;
        case 4:
          second++;
          if (second > 59) {
            second = 0;
          } else if (second <= 0) {
            second = 59;
          }
          break;
        default:
          menu--;
          if (menu < 0) {
            menu = 1;
          } else if (menu > 1) {
            menu = 0;
          }
          break;
      }
    } else {
      menu--;
      if (menu < 0) {
        menu = 1;
      } else if (menu > 1) {
        menu = 0;
      }
    }
    Serial.print("Menu Up: ");
    Serial.println(menu);
  }
}

void DownPressed(void) {
  Serial.println("Down pressed");
  if (isMenu) {
    if (menu == 0) {
      switch (subMenu) {
        case 2:
          date--;
          if (date > 31) {
            date = 1;
          } else if (date <= 0) {
            date = 31;
          }

          break;
        case 3:
          month--;
          if (month > 12) {
            month = 1;
          } else if (month <= 0) {
            month = 12;
          }
          break;
        case 4:
          year--;
          if (year > 99) {
            year = 0;
          } else if (year <= 0) {
            year = 99;
          }
          break;
        default:
          menu++;
          if (menu < 0) {
            menu = 1;
          } else if (menu > 1) {
            menu = 0;
          }
          break;
      }
    } else if (menu == 1) {
      switch (subMenu) {
        case 2:
          hour--;
          if (hour > 23) {
            hour = 0;
          } else if (hour <= 0) {
            hour = 23;
          }
          break;
        case 3:
          minute--;
          if (minute > 59) {
            minute = 0;
          } else if (minute <= 0) {
            minute = 59;
          }
          break;
        case 4:
          second--;
          if (second > 59) {
            second = 0;
          } else if (second <= 0) {
            second = 59;
          }
          break;
        default:
          menu++;
          if (menu < 0) {
            menu = 1;
          } else if (menu > 1) {
            menu = 0;
          }
          break;
      }
    } else {
      menu++;
      if (menu < 0) {
        menu = 1;
      } else if (menu > 1) {
        menu = 0;
      }
    }
  }
}

void EnterPressed(void) {
  Serial.println("Enter pressed");
  if (!isMenu) {
    display.setBrightness(5);
    isMenu = true;
    menu = 0;
    subMenu = 0;
  } else {
    subMenu++;
    display.setBrightness(5);
  }
}

void menuDisplay(void) {
  if (isMenu) {
    switch (menu) {
      case 0:
        // Display show F001
        switch (subMenu) {
          case 0:
            display.showNumberHexEx(0xF000);
            break;
          case 1:
            date = myRTC.getDate();
            month = myRTC.getMonth(century);
            year = myRTC.getYear();
            Serial.print("Date: ");
            Serial.print(date);
            Serial.print("/");
            Serial.print(month);
            Serial.print("/");
            Serial.println(year);

            subMenu = 2;
            break;
          case 2:
            // Display Date
            setBrightnessDisplay();
            display.showNumberDec(date, false, 4, 0);
            break;
          case 3:
            // Display Month
            setBrightnessDisplay();
            display.showNumberDec(month, false, 4, 0);
            break;
          case 4:
            // Display Year
            setBrightnessDisplay();
            display.showNumberDec(year, false, 4, 0);
            break;
          case 5:
            // Save
            myRTC.setDate(date);
            myRTC.setMonth(month);
            myRTC.setYear(year);
            subMenu = 0;
            break;
        }
        break;
      case 1:
        
        switch(subMenu){
          case 0:
           display.showNumberHexEx(0xF001);
          break;
          case 1:
            hour = myRTC.getHour(h12Flag,pmFlag);
            minute = myRTC.getMinute();
            second = myRTC.getSecond();
            Serial.print("Time: ");
            Serial.print(hour);
            Serial.print(":");
            Serial.print(minute);
            Serial.print(":");
            Serial.println(second);
            subMenu = 2;
          break;
          case 2:
            // Display Hour
            setBrightnessDisplay();
            display.showNumberDec(hour, false, 4, 0);
            break;
          case 3:
            // Display Minute
            setBrightnessDisplay();
            display.showNumberDec(minute, false, 4, 0);
            break;
          case 4:
            // Display Second
            setBrightnessDisplay();
            display.showNumberDec(second, false, 4, 0);
            break;
          case 5:
            // Save
            myRTC.setHour(hour);
            myRTC.setMinute(minute);
            myRTC.setSecond(second);
            subMenu = 0;
            break;
        }
        break;
    }
  } else {
    display.setSegments(SEG_WAIT);
  }
}
bool isBlink = false;
void setBrightnessDisplay() {
  if (millis() - last_time_ms >= 500) {
    isBlink = !isBlink;
    if (isBlink) {
      display.setBrightness(0, true);
    } else {
      display.setBrightness(5, true);
    }
    last_time_ms = millis();
  } else if (millis() < 1000) {
    last_time_ms = millis();
  }
}