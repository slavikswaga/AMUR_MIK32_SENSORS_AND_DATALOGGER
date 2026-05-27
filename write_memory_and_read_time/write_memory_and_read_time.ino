#include <SPI.h>
#include <stdint.h>

const int chipSelectPin = 7; // CS pin

/*OUR SENSORS NAMES*/
const String mgs_L75 = "0BH1750"; // 1 in memory
const String mgs_A6 = "MPU6050"; // 2 in memory
const String mgs_D20 = "VL53L0X"; // 3 in memory
const String mgs_SND504 = "INMP504"; // 4 in memory
const String mgs_CO30 = "00SGP30"; // 5 in memory
const String mgs_THP80 = "0BME280"; // 6 in memory
const String mgs_WT1 = "0000WT1"; // 7 in memory
const String mgs_FR403 = "TSL2540"; // 8 in memory

enum SENSORS{
  MGS_L75 = 1,
  MGS_A6 = 2,
  MGS_D20 = 3,
  MGS_SND504 = 4,
  MGS_CO30 = 5,
  MGS_THP80 = 6,
  MGS_WT1 = 7,
  MGS_FR403 = 8,

};

SENSORS hashString(const String& str){
  if(str == "0BH1750") return SENSORS::MGS_L75;
  if(str == "MPU6050") return SENSORS::MGS_A6;
  if(str == "VL53L0X") return SENSORS::MGS_D20;
  if(str == "INMP504") return SENSORS::MGS_SND504;
  if(str == "00SGP30") return SENSORS::MGS_CO30;
  if(str == "0BME280") return SENSORS::MGS_THP80;
  if(str == "0000WT1") return SENSORS::MGS_WT1;
  if(str == "TSL2540") return SENSORS::MGS_FR403;
}

word ADDR = 0x0000;
byte count_curr_data = 0;

void writeEEPROM(word address, byte data) {
  writeEnable();
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer(0x02);
  SPI.transfer((address >> 8) & 0xFF);
  SPI.transfer(address & 0xFF);
  SPI.transfer(data);
  digitalWrite(chipSelectPin, HIGH);
  delay(5);
}

byte readEEPROM(word address) {
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer(0x03);
  SPI.transfer((address >> 8) & 0xFF);
  SPI.transfer(address & 0xFF);
  byte data = SPI.transfer(0xFF);
  digitalWrite(chipSelectPin, HIGH);
  return data;
}

void writeEnable() {
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer(0x06);
  digitalWrite(chipSelectPin, HIGH);
}

void Parsing_and_Write_String_from_DATE(const String& Data){
  //dd.mm.yyyy hh:mm:ss FORMAT DATA
  /*write in memory - 1 byte count + 1 byte date + 1 byte month + 2 byte year + 1 byte hour + 1 byte minute + 1 byte second*/
  writeEEPROM(ADDR, count_curr_data);
  Serial.print(readEEPROM(ADDR));
  Serial.print(" ");
  if(readEEPROM(ADDR) > 100){
    ADDR = 0;
    count_curr_data = 0;
  }
  ADDR += 1;
  enum{
    DATA_INDEX_DAYS_TENS = 0,
    DATA_INDEX_DAYS_UNITS = 1,
    DATA_INDEX_MONTHS_TENS = 3,
    DATA_INDEX_MONTHS_UNITS = 4,
    DATA_INDEX_YEARS_THOUSANDS = 6,
    DATA_INDEX_YEARS_HUNDREDS = 7,
    DATA_INDEX_YEARS_TENS = 8,
    DATA_INDEX_YEARS_UNITS = 9,
    DATA_INDEX_HOURS_TENS = 11,
    DATA_INDEX_HOURS_UNITS = 12,
    DATA_INDEX_MINUTES_TENS = 14,
    DATA_INDEX_MINUTES_UNITS = 15,
    DATA_INDEX_SECONDS_TENS = 17,
    DATA_INDEX_SECONDS_UNITS = 18,
  };
  uint8_t date = (Data[DATA_INDEX_DAYS_TENS] - '0') * 10 + (Data[DATA_INDEX_DAYS_UNITS] - '0');
  uint8_t month = (Data[DATA_INDEX_MONTHS_TENS] - '0') * 10 + (Data[DATA_INDEX_MONTHS_UNITS] - '0');
  uint8_t year_part1 = (Data[DATA_INDEX_YEARS_THOUSANDS] - '0') * 10 + (Data[DATA_INDEX_YEARS_HUNDREDS] - '0');
  uint8_t year_part2 = (Data[DATA_INDEX_YEARS_TENS] - '0') * 10 + (Data[DATA_INDEX_YEARS_UNITS] - '0');
  uint8_t hour = (Data[DATA_INDEX_HOURS_TENS] - '0') * 10 + (Data[DATA_INDEX_HOURS_UNITS] - '0');
  uint8_t minutes = (Data[DATA_INDEX_MINUTES_TENS] - '0') * 10 + (Data[DATA_INDEX_MINUTES_UNITS] - '0');
  uint8_t second = (Data[DATA_INDEX_SECONDS_TENS] - '0') * 10 + (Data[DATA_INDEX_SECONDS_UNITS] - '0');
  writeEEPROM(ADDR, date);
  if((readEEPROM(ADDR) / 10) == 0) Serial.print("0");
  Serial.print(readEEPROM(ADDR));
  Serial.print('.');
  ADDR += 1;

  writeEEPROM(ADDR, month);
  if((readEEPROM(ADDR) / 10) == 0) Serial.print("0");
  Serial.print(readEEPROM(ADDR));
  Serial.print('.');
  ADDR += 1;

  writeEEPROM(ADDR, year_part1);
  Serial.print(readEEPROM(ADDR));
  ADDR += 1;
  writeEEPROM(ADDR, year_part2);
  Serial.print(readEEPROM(ADDR));
  ADDR += 1;
  Serial.print(' ');

  writeEEPROM(ADDR, hour);
  if((readEEPROM(ADDR) / 10) == 0) Serial.print("0");
  Serial.print(readEEPROM(ADDR));
  ADDR += 1;
  Serial.print(':');

  writeEEPROM(ADDR, minutes);
  if((readEEPROM(ADDR) / 10) == 0) Serial.print("0");
  Serial.print(readEEPROM(ADDR));
  ADDR += 1;
  Serial.print(':');

  writeEEPROM(ADDR, second);
  if((readEEPROM(ADDR) / 10) == 0) Serial.print("0");
  Serial.print(readEEPROM(ADDR));
  ADDR += 1;
  count_curr_data += 1;
}

void write_DATE_and_NAME(const String& name){
  if (Serial.available() > 0) {
    String receivedData = Serial.readString();
    Parsing_and_Write_String_from_DATE(receivedData);
    writeEEPROM(ADDR, hashString(name));

    Serial.print(" ");
    Serial.print(readEEPROM(ADDR));
    Serial.print(" ");
    Serial.print(name);
    //Serial.print(String(name[0]) + String(name[1]) + String(name[2]) + String(name[3]) + String(name[4]) + String(name[5]) + String(name[6]));
    Serial.println("");
    ADDR += 1;
  }
}
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(10);
  pinMode(chipSelectPin, OUTPUT);
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(chipSelectPin, HIGH);
  ADDR = 0x0000;
  count_curr_data = 0;
  Serial.println("ELBEAR ACE UNO подключен");
}

void loop() {
  write_DATE_and_NAME(mgs_L75);
  delay(1000);
  write_DATE_and_NAME(mgs_A6);
  delay(1000);
  write_DATE_and_NAME(mgs_D20);
  delay(1000);
  write_DATE_and_NAME(mgs_SND504);
  delay(1000);
  write_DATE_and_NAME(mgs_CO30);
  delay(1000);
  write_DATE_and_NAME(mgs_THP80);
  delay(1000);
  write_DATE_and_NAME(mgs_WT1);
  delay(1000);
  write_DATE_and_NAME(mgs_FR403);
  delay(1000);
}