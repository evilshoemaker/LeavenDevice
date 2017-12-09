#include <OneWire.h>
#include <TM1637Display.h>
#include <iarduino_Encoder_tmr.h>
//#include <OneButton.h>
#include <Button.h>

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
const byte ENCODER_TIME_CLK = 4;
const byte ENCODER_TIME_DT = 5;
const byte ENCODER_TIME_SW = 6;

const byte ENCODER_TEMP_CLK = 7;
const byte ENCODER_TEMP_DT = 8;
const byte ENCODER_TEMP_SW = 9;

//===========================================================================
// Подключение реле
//===========================================================================
const byte RELAY_1_PIN = 10;
const byte RELAY_2_PIN = 11;
const byte RELAY_3_PIN = 12;

//===========================================================================
// Подключение температурных датчиков
//===========================================================================
const byte THERMOMETER_MILK_DATA = 2; 
const byte THERMOMETER_WATER_DATA = 3;

OneWire milkThermometer(THERMOMETER_MILK_DATA);
OneWire waterThermometer(THERMOMETER_WATER_DATA);
unsigned long milkThermometerBeginRead = 0;
unsigned long waterThermometerBeginRead = 0;


TM1637Display timeDisplay(DISPLAY_TIME_CLK, DISPLAY_TIME_DIO);
TM1637Display curentTempDisplay(DISPLAY_CURRENT_TEMP_CLK, DISPLAY_CURRENT_TEMP_DIO);
TM1637Display targetTempDisplay(DISPLAY_TARGET_TEMP_CLK, DISPLAY_TARGET_TEMP_DIO);


iarduino_Encoder_tmr timeEncoder(ENCODER_TIME_CLK, ENCODER_TIME_DT);
iarduino_Encoder_tmr tempEncoder(ENCODER_TEMP_CLK, ENCODER_TEMP_DT);

float milkCurrentTemperature = 0.0;
float waterCurrentTemperature = 0.0;

float milkTargetTemperature = 0.0;
float milkTargetEncoderTemperature = 0.0;
bool isMilkTargetTempEdit = false;
bool isMilkTargetTempShow = false;
unsigned long milkTargetTempShowTime = 0;
unsigned long milkTargetEncoderEditTime = 0;
Button encoderTemperatureButton(ENCODER_TEMP_SW, PULLUP);


int timerMin = 0;
int timerEncoderMin = 0;
int timerEncoderHour = 0;

bool isTimerMinEdit = false;
bool isTimerMinShow = false;

bool isTimerHourEdit = false;
bool isTimerHourShow = false;

unsigned long timerShowTime = 0;
unsigned long timerEncoderEditTime = 0;
Button encoderTimeButton(ENCODER_TIME_SW, PULLUP);


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

void getAllTemperature() 
{
    float tMilk = readTemperature(milkThermometer, THERMOMETER_MILK_DATA);
    float tWater = readTemperature(waterThermometer, THERMOMETER_WATER_DATA);
    if (tMilk != -1024) 
    {
        milkCurrentTemperature = tMilk;
    }
    if (tWater != -1024) 
    {
        waterCurrentTemperature = tWater;
    }

    /*Serial.print("Milk: ");
    Serial.print(milkCurrentTemperature);
    Serial.print("; Water: ");
    Serial.println(waterCurrentTemperature);*/
}

float readTemperature(OneWire &ds, int sensor)
{
    float celsius = -1024;
    
    unsigned long timeDiff = 0;
    if (sensor == THERMOMETER_MILK_DATA) {
        timeDiff = millis() - milkThermometerBeginRead;
    }
    else if (sensor == THERMOMETER_WATER_DATA) {
        timeDiff = millis() - waterThermometerBeginRead;
    }

    if (timeDiff > 1000) 
    {
        byte i;
        byte data[12];
        byte addr[8];
        
        // поиск адреса датчика
        if ( !ds.search(addr)) {
            ds.reset_search();
            //delay(250);
            return celsius;
        }
        
        ds.reset();
        ds.select(addr); 
        ds.write(0xBE); // команда на начало чтения измеренной температуры
    
        // считываем показания температуры из внутренней памяти датчика
        for ( i = 0; i < 9; i++) {
            data[i] = ds.read();
        }
    
        int16_t raw = (data[1] << 8) | data[0];
        // датчик может быть настроен на разную точность, выясняем её 
        byte cfg = (data[4] & 0x60);
        if (cfg == 0x00) raw = raw & ~7; // точность 9-разрядов, 93,75 мс
        else if (cfg == 0x20) raw = raw & ~3; // точность 10-разрядов, 187,5 мс
        else if (cfg == 0x40) raw = raw & ~1; // точность 11-разрядов, 375 мс
    
        // преобразование показаний датчика в градусы Цельсия 
        celsius = (float)raw / 16.0;

        ds.reset();
        ds.select(addr);
        ds.write(0x44, 1); // команда на измерение температуры

        if (sensor == THERMOMETER_MILK_DATA) {
            milkThermometerBeginRead = millis();
        }
        else if (sensor == THERMOMETER_WATER_DATA) {
            waterThermometerBeginRead = millis();
        }
    }

    return celsius;
}

void printInfoOnDisplay()
{
    curentTempDisplay.showNumberDec((int)milkCurrentTemperature, false, 2, 0);
    curentTempDisplay.showNumberDec((int)waterCurrentTemperature, false, 2, 2);

    //показываем выбранную температуру
    if (!isMilkTargetTempEdit) {
        targetTempDisplay.showNumberDec((int)milkTargetEncoderTemperature, false, 4, 0);
    }
    else {       
        if (isMilkTargetTempShow) {
            targetTempDisplay.showNumberDec((int)milkTargetEncoderTemperature, false, 2, 2);
        }
        else {
            uint8_t data[] = { 0x00, 0x00, 0x00, 0x00 };
            targetTempDisplay.setSegments(data);
        }
    }

    //показываем время
    if (!isTimerMinEdit) {
        timeDisplay.showNumberDec((int)(timerMin % 60), false, 2, 2);
    }
    else {
        if (isTimerMinShow) {
            timeDisplay.showNumberDec((int)timerEncoderMin, false, 2, 2);
        }
        else {
            uint8_t data[] = { 0x00, 0x00 };
            timeDisplay.setSegments(data, 2, 2);
        }
    }
    if (!isTimerHourEdit) {
        timeDisplay.showNumberDec((int)(timerMin / 60), false, 2, 0);
    }
    else {
        if (isTimerHourShow) {
            timeDisplay.showNumberDec(timerEncoderHour, false, 2, 0);
        }
        else {
            uint8_t data[] = { 0x00, 0x00 };
            timeDisplay.setSegments(data, 2, 0);
        }
    }
}

void targetTemperatureEncoderRead() 
{
    int encoderValue = tempEncoder.read();
    
    if(encoderValue != 0)
    {
        milkTargetEncoderEditTime = millis();
        milkTargetTempShowTime = millis();
        isMilkTargetTempEdit = true;
        isMilkTargetTempShow = true;

        milkTargetEncoderTemperature += encoderValue;
        if (milkTargetEncoderTemperature < 0) {
            milkTargetEncoderTemperature = 0;
        }
        if (milkTargetEncoderTemperature > 99) {
            milkTargetEncoderTemperature = 99;
        }
    }
    else 
    {
        if (isMilkTargetTempEdit) 
        {
            if (millis() - milkTargetTempShowTime > 500) {
                isMilkTargetTempShow = !isMilkTargetTempShow;
                milkTargetTempShowTime = millis();
            }
            
            //проверяем, что время на редактирование вышло
            if (millis() - milkTargetEncoderEditTime < 5000)
            {
                if (encoderTemperatureButton.uniquePress()) 
                {
                    milkTargetTemperature = milkTargetEncoderTemperature; 
                    isMilkTargetTempEdit = false;
                }
            }
            else {
                milkTargetEncoderTemperature = milkTargetTemperature; 
                isMilkTargetTempEdit = false;
            }
        }
    }
}

void timerEncoderRead() 
{
    int encoderValue = timeEncoder.read();

    if(encoderValue != 0)
    {
        if (!isTimerMinEdit && !isTimerHourEdit) {
            isTimerMinEdit = true;
        }

        if (isTimerMinEdit) 
        {
            timerEncoderEditTime = millis();
            timerShowTime = millis();
            isTimerMinEdit = true;
            isTimerMinShow = true;
    
            timerEncoderMin += encoderValue;
            if (timerEncoderMin < 0) {
                timerEncoderMin = 59;
            }
            if (timerEncoderMin > 59) {
                timerEncoderMin = 0;
            }
        }
        else if (isTimerHourEdit) 
        {
            timerEncoderEditTime = millis();
            timerShowTime = millis();
            isTimerHourEdit = true;
            isTimerHourShow = true;
    
            timerEncoderHour += encoderValue;
            if (timerEncoderHour < 0) {
                timerEncoderHour = 72;
            }
            if (timerEncoderHour > 72) {
                timerEncoderHour = 0;
            }
        }
    }
    else 
    {
        if (isTimerMinEdit) 
        {
            if (millis() - timerShowTime > 500) {
                isTimerMinShow = !isTimerMinShow;
                timerShowTime = millis();
            }
            
            //проверяем, что время на редактирование вышло
            if (millis() - timerEncoderEditTime < 5000)
            {
                if (encoderTimeButton.uniquePress()) 
                {
                    timerMin = timerEncoderMin + (timerEncoderHour * 60); 
                    isTimerMinEdit = false;
                    isTimerHourEdit = true;

                    timerEncoderEditTime = millis();
                    timerShowTime = millis();
                }
            }
            else {
                timerEncoderMin = timerMin; 
                isTimerMinEdit = false;
                isTimerHourEdit = false;
            }
        }
        else if (isTimerHourEdit) 
        {
            if (millis() - timerShowTime > 500) {
                isTimerHourShow = !isTimerHourShow;
                timerShowTime = millis();
            }
            
            //проверяем, что время на редактирование вышло
            if (millis() - timerEncoderEditTime < 5000)
            {
                if (encoderTimeButton.uniquePress()) 
                {
                    timerMin = timerEncoderMin + (timerEncoderHour * 60);
                    isTimerMinEdit = false;
                    isTimerHourEdit = false;
                }
            }
            else {
                timerEncoderMin = timerMin; 
                isTimerMinEdit = false;
                isTimerHourEdit = false;
            }
        }
    }
}

void setup()
{
    Serial.begin(9600);
    initDisplays();
    initEncoders();
}

void loop(){
    getAllTemperature();
    printInfoOnDisplay();
    targetTemperatureEncoderRead();
    timerEncoderRead();

    Serial.print("timerEncoderMin: ");
    Serial.print(timerEncoderMin);
    Serial.print("; timerMin: ");
    Serial.println(timerMin);
    
    //delay(1000);
}
