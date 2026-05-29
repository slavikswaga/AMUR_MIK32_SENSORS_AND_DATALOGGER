#include <Wire.h>
#include <SPI.h>
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

const float air_value = 561.0;
const float water_value = 293.0;
const float moisture_0 = 0.0;
const float moisture_100 = 100.0;

Adafruit_BME280 bme;
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor *bme_pressure = bme.getPressureSensor();  
Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();
PCA9634 testModule(0x1C);

void setup() {
  Serial.begin(9600);
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
  for (int channel = 0; channel < testModule.channelCount(); channel++) {
    testModule.setLedDriverMode(channel, PCA9634_LEDPWM);  // установка режима ШИМ (0-255)
  }
  lm75_sensor.shutdown(false);
  //L75 reading
  int lux = lightMeter.readLightLevel();
  lcd.home();
  lcd.clear();
  lcd.print("1 BH1750: ");
  lcd.print(lux);
  lcd.print("lx");

  //A6 reading
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  lcd.setCursor(0, 1);
  lcd.print("2 MPU60: ");
  lcd.print(temp.temperature);
  lcd.print("*C");
  
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
    lcd.print("Out Range");
  }
  //SND504 reading
  float adc1 = mcp3221.getVoltage();
  lcd.setCursor(0, 1);
  lcd.print("4 INMP: ");
  lcd.print(20*log10(adc1));
  lcd.print("dB");
  delay(1000);
  //LM75A reading

  lcd.home();
  lcd.clear();
  lcd.print("5 LM75A:");
  lcd.print(lm75_sensor.temp());
  lcd.print(" *C");
  lm75_sensor.shutdown(true);
  //THP80 reading
  sensors_event_t temp_event, pressure_event, humidity_event;
  bme_temp->getEvent(&temp_event);
  bme_pressure->getEvent(&pressure_event);
  bme_humidity->getEvent(&humidity_event);
  lcd.setCursor(0, 1);
  lcd.print("6 BME280: ");
  lcd.print(humidity_event.relative_humidity);
  lcd.print("%");

  delay(1000);
  
  //WT1 reading
  float adc0 = mcp3021.readADC();
  float h = map(adc0, air_value, water_value, moisture_0, moisture_100);
  if(h < 0){
    h = 0;
  }
  lcd.home();
  lcd.clear();
  lcd.print("7 WT1: ");
  lcd.print(String(h, 1) + " %");
  //FR403 reading

  delay(20);
  Fire.get_ir_and_vis();
  lcd.setCursor(0, 1);
  lcd.print("8 TSL2540:");
  int uwu = Fire.ir_data;
  lcd.print(uwu);
  lcd.print("UV");

  RGB(30, 0, 0);
  delay(1000);
  RGB(0, 0, 10);
}

void RGB(byte r, byte g, byte b) {
  testModule.write1(3, r);
  testModule.write1(2, g);
  testModule.write1(5, b);
} 