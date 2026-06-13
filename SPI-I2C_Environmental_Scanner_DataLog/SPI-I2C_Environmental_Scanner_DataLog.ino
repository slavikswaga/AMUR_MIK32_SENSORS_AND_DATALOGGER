#include <SPI.h>
#include <stdint.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <PCA9634.h>
#include "mcp3021.h"
#include <LM75.h>
#include "MCP3221.h"
#include "Adafruit_VL53L0X.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <MGS_FR403.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>      // подключаем библиотеку дисплея
LiquidCrystal_I2C lcd(0x20, 16, 2); // адрес, столбцов, строк

MGS_FR403 Fire;
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
MCP3221 mcp3221(0x4E);
Adafruit_MPU6050 mpu;
MCP3021 mcp3021;
byte ADDR_mcp3021 = 0x4B - 0x48;
BH1750 lightMeter;
LM75 lm75_sensor;

const int chipSelectPin = 7; // CS pin

const float air_value = 561.0;
const float water_value = 293.0;
const float moisture_0 = 0.0;
const float moisture_100 = 100.0;

/*OUR SENSORS NAMES*/
const String mgs_L75 = "0BH1750"; // 1 in memory
const String mgs_A6 = "MPU6050"; // 2 in memory
const String mgs_D20 = "VL53L0X"; // 3 in memory
const String mgs_SND504 = "INMP504"; // 4 in memory
const String LM75 = "00LM75A"; // 5 in memory
const String mgs_THP80 = "0BME280"; // 6 in memory
const String mgs_WT1 = "0000WT1"; // 7 in memory
const String mgs_FR403 = "TSL2540"; // 8 in memory

enum SENSORS{
  MGS_L75 = 1,
  MGS_A6 = 2,
  MGS_D20 = 3,
  MGS_SND504 = 4,
  nLM75 = 5,
  MGS_THP80 = 6,
  MGS_WT1 = 7,
  MGS_FR403 = 8,

};

Adafruit_BME280 bme;
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor *bme_pressure = bme.getPressureSensor();  
Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();
PCA9634 testModule(0x1C);

SENSORS hashString(const String& str){
  if(str == "0BH1750") return SENSORS::MGS_L75;
  if(str == "MPU6050") return SENSORS::MGS_A6;
  if(str == "VL53L0X") return SENSORS::MGS_D20;
  if(str == "INMP504") return SENSORS::MGS_SND504;
  if(str == "00LM75A") return SENSORS::nLM75;
  if(str == "0BME280") return SENSORS::MGS_THP80;
  if(str == "0000WT1") return SENSORS::MGS_WT1;
  if(str == "TSL2540") return SENSORS::MGS_FR403;
}

word ADDR = 0x0000;
byte count_curr_data = 1;

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
  Serial.print(readEEPROM (ADDR));
  ADDR += 1;
  count_curr_data += 1;
}

void write_DATE_and_NAME(const String& name){
  if (Serial.available() > 0) {
    String receivedData = Serial.readString();

    writeEEPROM(ADDR, count_curr_data);
    Serial.print(readEEPROM(ADDR));
    Serial.print(" ");
    ADDR++;
    
    if(count_curr_data >= 100){  //100 значений
      ADDR = 0;
      count_curr_data = 1;
    }

    Parsing_and_Write_String_from_DATE(receivedData);
    writeEEPROM(ADDR, hashString(name));

    Serial.print(" ");
    Serial.print(readEEPROM(ADDR));
    Serial.print(" ");
    Serial.print(name);
    //Serial.print(String(name[0]) + String(name[1]) + String(name[2]) + String(name[3]) + String(name[4]) + String(name[5]) + String(name[6]));
    //Serial.println("");
    Serial.print(" ");  
    ADDR += 1;
  }
}

uint32_t uint32_t_to_uint8_t_array_write_read_memory(const uint32_t& data){
  uint8_t arr[4];
  memcpy(arr, &data, sizeof(uint32_t));
  for(int i = 0; i < (sizeof(uint32_t) / sizeof(uint8_t)); i++){
    writeEEPROM(ADDR, arr[i]);
    ADDR++;
  }
  uint32_t reading_from_mem_data = *((uint32_t*)arr);
  return reading_from_mem_data;
}

float float_to_uint8_t_array_write_read_memory(const float& data){
  uint8_t arr[4];
  memcpy(arr, &data, sizeof(float));
  for(int i = 0; i < (sizeof(float) / sizeof(uint8_t)); i++){
    writeEEPROM(ADDR, arr[i]);
    ADDR++;
  }
  float reading_from_mem_data = *((float*)arr);
  return reading_from_mem_data;
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
  //Serial.println("ELBEAR ACE UNO подключен"); 
  lcd.init();
  lcd.backlight();
  lcd.print("ENVIRONMENTAL");
  lcd.setCursor(0, 1);
  lcd.print("   MONITORING");
  delay(1000);
  lcd.clear();
  lcd.home();
  lcd.print("VYACHESLAV");
  lcd.setCursor(0, 1);
  lcd.print("KUSTOV 8V51T");
  Wire.begin();
  //MGS-L75 init
  lightMeter.begin();
  //MGS-FR403 Init
  Fire.begin();
  //MGS-А6 init
  if (!mpu.begin(0x69)) {
    Serial.println("Failed to find MPU6050 chip");
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  //MGS-D20 init
  if (!lox.begin()) {
    Serial.println(F("Sensor MGS-D20 not found :("));
  }
  // MGS-WT1 init
  mcp3021.begin(ADDR_mcp3021);
  //MGS-THP80 init
  if (!bme.begin()) {
    Serial.println(F("Sensor MGS-THP80 not found :("));
  }
  //RGB3 init
  testModule.begin();
  for (int channel = 0; channel < testModule.channelCount(); channel++) {
    testModule.setLedDriverMode(channel, PCA9634_LEDOFF);  // выключить все светодиоды в режиме 0/1
  }
}

int counter = 0;

void loop() {
  for (int channel = 0; channel < testModule.channelCount(); channel++) { // Настройка для RGB3
    testModule.setLedDriverMode(channel, PCA9634_LEDPWM);  // установка режима ШИМ (0-255)
  }
  lm75_sensor.shutdown(false); // Настройка для LM75
  delay(1000);
  //L75 reading
  uint32_t lux = lightMeter.readLightLevel();
  lcd.home();
  lcd.clear();
  lcd.print("1 BH1750:");
  lcd.print(lux);
  lcd.print(" lx");
  write_DATE_and_NAME(mgs_L75);
  Serial.print(uint32_t_to_uint8_t_array_write_read_memory(lux));
  Serial.println("");
  delay(1000);

  //A6 reading
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  lcd.setCursor(0, 1);
  lcd.print("2 MPU60: ");
  lcd.print(temp.temperature);
  lcd.print("*C");
  write_DATE_and_NAME(mgs_A6);
  Serial.print(float_to_uint8_t_array_write_read_memory(temp.temperature));
  Serial.println("");
  delay(1000);

  //D20 reading
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);  // pass in 'true' to get debug data printout!
  lcd.home();
  lcd.clear();
  lcd.print("3 VL53L0X:");
  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    lcd.print(measure.RangeMilliMeter);
    lcd.print("mm");
  } else {
    lcd.print("OutOfRange");
  }
  write_DATE_and_NAME(mgs_D20);
  Serial.print(uint32_t_to_uint8_t_array_write_read_memory(measure.RangeMilliMeter));
  Serial.println("");
  delay(1000);

  //SND504 reading
  float adc1 = mcp3221.getVoltage();
  lcd.setCursor(0, 1);
  lcd.print("4 INMP: ");
  lcd.print(20*log10(adc1));
  lcd.print("dB");
  write_DATE_and_NAME(mgs_SND504);
  Serial.print(float_to_uint8_t_array_write_read_memory(20*log10(adc1)));
  Serial.println("");
  delay(1000);

  //LM75
  lcd.home();
  lcd.clear();
  lcd.print("5 LM75A:");
  lcd.print(lm75_sensor.temp());
  lcd.print(" *C");
  write_DATE_and_NAME(LM75);
  Serial.print(float_to_uint8_t_array_write_read_memory(lm75_sensor.temp()));
  Serial.println("");
  lm75_sensor.shutdown(true);
  delay(1000);

  //THP80 reading
  sensors_event_t temp_event, pressure_event, humidity_event;
  bme_temp->getEvent(&temp_event);
  bme_pressure->getEvent(&pressure_event);
  bme_humidity->getEvent(&humidity_event);
  lcd.setCursor(0, 1);
  lcd.print("6 BME280: ");
  lcd.print(humidity_event.relative_humidity);
  lcd.print("%");
  write_DATE_and_NAME(mgs_THP80);
  Serial.print(float_to_uint8_t_array_write_read_memory(humidity_event.relative_humidity));
  Serial.println("");
  delay(1000);

  float adc0 = mcp3021.readADC();
  float h = map(adc0, air_value, water_value, moisture_0, moisture_100);
  if(h < 0) h = 0;
  lcd.home();
  lcd.clear();
  lcd.print("7 WT1: ");
  lcd.print(String(h, 1) + " %");
  write_DATE_and_NAME(mgs_WT1);
  Serial.print(String(float_to_uint8_t_array_write_read_memory(h), 1));
  Serial.println("");
  delay(1000);

  //FR403 reading

  delay(20);
  Fire.get_ir_and_vis();
  lcd.setCursor(0, 1);
  lcd.print("8 TSL2540:");
  uint32_t uwu = Fire.ir_data;
  lcd.print(uwu);
  lcd.print("UV");
  write_DATE_and_NAME(mgs_FR403);
  Serial.print(uint32_t_to_uint8_t_array_write_read_memory(uwu));
  Serial.println("");

  RGB(30, 0, 0);
  delay(1000);
  RGB(0, 0, 10);
}

void RGB(byte r, byte g, byte b) {
  testModule.write1(3, r);
  testModule.write1(2, g);
  testModule.write1(5, b);
} 