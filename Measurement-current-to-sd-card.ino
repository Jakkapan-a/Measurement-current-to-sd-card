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
void EscReleased(void);
void UpPressed(void);
void UpReleased(void);
void DownPressed(void);
void DownReleased(void);
void EnterPressed(void);
void EnterReleased(void);

TcBUTTON escButton(esc, EscPressed, EscReleased);
TcBUTTON upButton(up, UpPressed, UpReleased);
TcBUTTON downButton(down, DownPressed, DownReleased);
TcBUTTON enterButton(enter, EnterPressed, EnterReleased);

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
const uint8_t SEG_WAIT_[] = {
  SEG_D,  // -
  SEG_D,  // -
  SEG_D,  // -
  SEG_D,  // -
};
// TM1637 Display
#define CLK 5
#define DIO 6
TM1637Display display(CLK, DIO);

int menu = 0;
uint8_t subMenu = 3;
bool isMenu = false;

uint8_t IsNumButton = 0;
int CountButtonPressed = 0;
uint8_t CountButtonPressedMax = 3;

uint8_t CountDownSleep = 0;
uint8_t CountDownSleepMax = 50;


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


float current = 0;
float voltage = 0;
float power = 0;
float shuntvoltage = 0;
float loadvoltage = 0;
String filename = "Data.csv";

void setup() {
   Serial.begin(9600);
  while (!Serial); // Wait for serial monitor to open
  Serial.println("Start");

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

  Serial.println("\nI2C Scanner");
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println(" !");

      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("done\n");
  }
  
  if (!ina219_A.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
       display.setBrightness(5, true);
      display.setSegments(SEG_WAIT_);
      delay(500);
      display.setBrightness(0, false);
      display.setSegments(SEG_WAIT_);
      delay(500);
     }
  }

}

void loop() {
  escButton.update();
  upButton.update();
  downButton.update();
  enterButton.update();

  myTime();
  menuDisplay();
}
void myTime(void) {
  // Get time from RTC
  unsigned long currentTime = millis();
  //--------------------------------------------------------------------------------
  // This function is called every 1 second
  if (currentTime - lastTime >= 1000) {
    if (CountDownSleep > 0) {
      CountDownSleep--;
    }

    if (CountDownSleep == 1) {
      isMenu = false;
    }

    if (IsNumButton != 0) {
      CountButtonPressed++;
    }
    lastTime = currentTime;
  } else if (currentTime < 1000) {
    lastTime = currentTime;
  }
  //--------------------------------------------------------------------------------
  // Is button pressed for 5 seconds
  if (IsNumButton != 0 && CountButtonPressed >= CountButtonPressedMax) {
    switch (IsNumButton) {
      case 1:
        EscPressed();
        break;
      case 2:
        UpPressed();
        break;
      case 3:
        DownPressed();
        break;
      case 4:
        EnterPressed();
        break;
    }
  }
  //--------------------------------------------------------------------------------
  // Auto save data to SD card every 100 ms
  if (currentTime - last_time_mss >= 100) {
    if (!isMenu) {

      SaveData();
    }
    last_time_mss = currentTime;
  } else if (currentTime < 1000) {
    last_time_mss = currentTime;
  }
}

void SaveData() {
  current = ina219_A.getCurrent_mA();
  voltage = ina219_A.getBusVoltage_V();
  power = ina219_A.getPower_mW();
  shuntvoltage = ina219_A.getShuntVoltage_mV();
  loadvoltage = voltage + (shuntvoltage / 1000);
  display.showNumberDec(current, false, 4, 0);


  year = myRTC.getYear();
  month = myRTC.getMonth(century);
  date = myRTC.getDate();
  hour = myRTC.getHour(h12Flag, pmFlag);
  minute = myRTC.getMinute();
  second = myRTC.getSecond();

  filename = "F" + String(year) + String(month) + String(date) + String(hour) + ".csv";

  // // Check if file exists
  if (!SD.exists(filename)) {
    // Create file
    File dataFile = SD.open(filename, FILE_WRITE);
    dataFile.println("Date, Time,Current_mA,Voltage"); //,Power,Shunt Voltage,Load Voltage");
    dataFile.close();
  }

  // // Open file
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.print(String(date) + "/" + String(month) + "/" + String(year) + ",");
    dataFile.print(String(hour) + ":" + String(minute) + ":" + String(second) + ",");
    dataFile.print(String(current) + ",");
    dataFile.println(String(voltage) + ",");
    // dataFile.print(String(power) + ",");
    // dataFile.print(String(shuntvoltage) + ",");
    // dataFile.println(String(loadvoltage));
    dataFile.close();
    // Serial.println("Data saved");
    // Serial print data
    Serial.print(String(date) + "/" + String(month) + "/" + String(year) + ",");
    Serial.print(String(hour) + ":" + String(minute) + ":" + String(second) + ",");
    Serial.print(String(current) + ",");
    Serial.println(String(voltage) + ",");
    // Serial.print(String(power) + ",");
    // Serial.print(String(shuntvoltage) + ",");
    // Serial.println(String(loadvoltage));
  } else {
    while (!SD.begin(SD_CS)) {
      display.setBrightness(5, true);
      display.setSegments(SEG_WAIT);
      delay(200);
      display.setBrightness(0, false);
      display.setSegments(SEG_WAIT);
      delay(200);
    }
    display.setBrightness(5, true);
  }
}
void EscPressed(void) {
  Serial.println("Esc pressed");
  if (subMenu > 0 && isMenu) {
    subMenu = 0;
  } else {
    isMenu = false;
  }
  if (IsNumButton == 0) {
    IsNumButton = 1;
    CountButtonPressed = 0;
  }
  CountDownSleep = CountDownSleepMax;
}

void EscReleased(void) {
  IsNumButton = 0;
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

  if (IsNumButton == 0) {
    IsNumButton = 2;
    CountButtonPressed = 0;
  }
  CountDownSleep = CountDownSleepMax;
}

void UpReleased(void) {
  IsNumButton = 0;
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
  if (IsNumButton == 0) {
    IsNumButton = 3;
    CountButtonPressed = 0;
  }
  CountDownSleep = CountDownSleepMax;
}

void DownReleased(void) {
  IsNumButton = 0;
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
  if (IsNumButton == 0) {
    IsNumButton = 4;
    CountButtonPressed = 0;
  }
  CountDownSleep = CountDownSleepMax;
}

void EnterReleased(void) {
  IsNumButton = 0;
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

        switch (subMenu) {
          case 0:
            display.showNumberHexEx(0xF001);
            break;
          case 1:
            hour = myRTC.getHour(h12Flag, pmFlag);
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
    // display.setSegments(SEG_WAIT);
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