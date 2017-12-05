#include <OneWire.h>
#include <TM1637Display.h>
#include <iarduino_Encoder_tmr.h>
#include <DallasTemperature.h>

//===========================================================================
// Подключение дисплеев
//===========================================================================
const byte DISPLAY_TIME_CLK = 14; 
const byte DISPLAY_TIME_DIO = 15;

const byte DISPLAY_CURRENT_TEMP_CLK = 16; 
const byte DISPLAY_CURRENT_TEMP_DIO = 17;

const byte DISPLAY_TARGET_TEMP_CLK = 18; 
const byte DISPLAY_TARGET_TEMP_DIO = 19;

const byte DISPLAY_BRIGHT = 0x0a;

//===========================================================================
// Подключение энкодеров
//===========================================================================
const byte ENCODER_TIME_CLK = 2;
const byte ENCODER_TIME_DT = 3;
const byte ENCODER_TIME_SW = 4;

const byte ENCODER_TEMP_CLK = 5;
const byte ENCODER_TEMP_DT = 6;
const byte ENCODER_TEMP_SW = 7;

//===========================================================================
// Подключение реле
//===========================================================================
const byte RELAY_1_PIN = 8;
const byte RELAY_2_PIN = 9;
const byte RELAY_3_PIN = 10;

//===========================================================================
// Подключение температурных датчиков
//===========================================================================
const byte ONE_WIRE_BUS = 2;
//const byte THERMOMETER_MILK_DATA = 2; 
///const byte THERMOMETER_WATER_DATA = 2;
OneWire oneWire(ONE_WIRE_BUS);

DeviceAddress milkThermometerAddress = { 0x28, 0xFF, 0xEF, 0x94, 0xA1, 0x16, 0x03, 0xEB }; //адрес датчика 28 FF EF 94 A1 16 3 EB Thermometer
DeviceAddress waterThermometerAddress = { 0x28, 0xFF, 0x65, 0x3E, 0xA0, 0x16, 0x03, 0x84 }; //28 FF 65 3E A0 16 3 84

DallasTemperature sensors(&oneWire);


iarduino_Encoder_tmr timeEncoder(ENCODER_TIME_CLK, ENCODER_TIME_DT);
iarduino_Encoder_tmr tempEncoder(ENCODER_TEMP_CLK, ENCODER_TEMP_DT);

TM1637Display timeDisplay(DISPLAY_TIME_CLK, DISPLAY_TIME_DIO);
TM1637Display curentTempDisplay(DISPLAY_CURRENT_TEMP_CLK, DISPLAY_CURRENT_TEMP_DIO);
TM1637Display targetTempDisplay(DISPLAY_TARGET_TEMP_CLK, DISPLAY_TARGET_TEMP_DIO);

//OneWire milkThermometer(THERMOMETER_MILK_DATA);
//OneWire waterThermometer(THERMOMETER_WATER_DATA);



float milkCurrentTemperature = 0.0;
float waterCurrentTemperature = 0.0;
float milkTargetTemperature = 0.0;
float milkTargetEncoderTemperature = 0.0;
bool isMilkTargetTempEdit = false;
unsigned long encodderEditTime = 0;

void initDisplays() 
{
    uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
    timeDisplay.setBrightness(DISPLAY_BRIGHT);
    timeDisplay.setSegments(data);

    curentTempDisplay.setBrightness(DISPLAY_BRIGHT);
    curentTempDisplay.setSegments(data);

    targetTempDisplay.setBrightness(DISPLAY_BRIGHT);
    targetTempDisplay.setSegments(data);
}

void initEncoders()
{
    timeEncoder.begin();  
    tempEncoder.begin();

    pinMode(ENCODER_TIME_SW, INPUT);
    pinMode(ENCODER_TEMP_SW, INPUT);
}

void initSensors() 
{
    sensors.begin();
    sensors.setResolution(milkThermometerAddress, 10);
    sensors.setResolution(waterThermometerAddress, 10);
}

void getTemperature() 
{
    sensors.requestTemperatures();
    milkCurrentTemperature = sensors.getTempC(milkThermometerAddress);
    waterCurrentTemperature = sensors.getTempC(waterThermometerAddress);
    Serial.print("Milk: ");
    Serial.print(milkCurrentTemperature);
    Serial.print("; Water: ");
    Serial.println(waterCurrentTemperature);
}

void printInfoOnDisplay()
{
    curentTempDisplay.showNumberDec((int)milkCurrentTemperature, false, 2, 0);
    curentTempDisplay.showNumberDec((int)waterCurrentTemperature, false, 2, 2);
}

void targetTemperatureEncoderRead() 
{
    int i = tempEncoder.read();
    if(i != 0)
    {
        encodderEditTime = millis();
        isMilkTargetTempEdit = true;
    }
    else 
    {
        
    }
}

void setup()
{
    Serial.begin(9600);
    initDisplays();
    initEncoders();
}

void loop(){
    getTemperature();
    printInfoOnDisplay();
    delay(1000);
}

//long currentValue = 0;
//long currentValue2 = 0;
//void loop(){
//    int i=enc.read();
//    int i2=enc2.read();
//    /*if(i){                                    //  Если энкодер зафиксировал поворот, то ...
//        if(i==encLEFT ){currentValue Serial.println("<");} //  Если энкодер зафиксировал поворот влево,  выводим символ <
//        if(i==encRIGHT){Serial.println(">");} //  Если энкодер зафиксировал поворот вправо, выводим символ >
//    }*/
//    if(i){
//        currentValue += i;
//        Serial.print("E1");
//        Serial.println(currentValue);
//        if (currentValue < 0) {
//            currentValue = 0;
//        }
//        
//        display.showNumberDec(currentValue, false, 4, 0);
//        display2.showNumberDec(currentValue, false, 4, 0);
//        display3.showNumberDec(currentValue, false, 4, 0);
//    }
//    if(i2){
//        currentValue2 += i2;
//        Serial.print("E2");
//        Serial.println(currentValue2);
//    }
//}

//#include <Encoder.h>
////#include <TM1637Display.h>
//
////#define CLK 2
////#define DIO 3
//
////TM1637Display display(CLK, DIO);
//Encoder myEnc(5, 6);
//
//void setup() {
//    Serial.begin(9600);
//    Serial.println("Basic Encoder Test:");
//    /*uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
//    display.setBrightness(0x0f);
//  
//    // All segments on
//    display.setSegments(data);*/
//}
//long oldPosition  = -999;
//void loop() {
//    long newPosition = myEnc.read();
//  if (newPosition != oldPosition) {
//    oldPosition = newPosition;
//    Serial.println(newPosition);
//  }
//  // put your main code here, to run repeatedly:
//
//}
