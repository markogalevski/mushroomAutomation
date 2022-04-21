
#include "DHT.h"
#include <stdint.h>
#include <stdlib.h>
#include <avr/wdt.h>

#define RELAY_FAN_PIN (7UL)
#define RELAY_HUMID_PIN (5UL)
#define SENSOR_HUMID_PIN (6UL)
#define MILLISECONDS_PER_MINUTE (60000UL)

//#define DEBUG
typedef enum 
{
  STATE_OFF = LOW,
  STATE_ON = HIGH
} state_t;

typedef enum
{
  PER_10_MINUTES = 10,
  PER_15_MINUTES = 15,
  PER_30_MINUTES = 30,
  PER_40_MINUTES = 40,
  PER_45_MINUTES = 45,
  PER_HOUR = 60
} timeframe_t;

typedef struct
{
  state_t currentState;
  uint32_t controlPin;
  uint32_t onPeriodMs;
  uint32_t offPeriodMs;
  uint32_t onTimeMs;
  uint32_t offTimeMs;
} fanData_t;

typedef struct
{
  state_t currentState;
  float rhUpperBound;
  float rhLowerBound;
  uint32_t controlPin;
  DHT *sensor;
  uint32_t lastMeasured;
  uint32_t padding0;
} humidifierData_t;

DHT dht(SENSOR_HUMID_PIN, DHT11);
fanData_t fanInstance;
humidifierData_t hmdInstance;

void initFan(fanData_t *fan, uint32_t controlPin, uint32_t onTimeMinutesPerUnit, timeframe_t timeframe);
void initHumidifier(humidifierData_t *hmd, uint32_t controlPin, DHT *sensor, float rhLowerBound, float rhUpperBound);
void manageFan(fanData_t *fan);
void manageHumidity(humidifierData_t *hmd);
void (* resetArduino) (void) = 0;

void setup() {
  initHumidifier(&hmdInstance, RELAY_HUMID_PIN, &dht, 83.F, 85.F);
  initFan(&fanInstance, RELAY_FAN_PIN, 1, PER_15_MINUTES); 
  wdt_enable(WDTO_8S);
#ifdef DEBUG
  Serial.begin(9600);
#endif
}

void loop() {
  // put your main code here, to run repeatedly:
  wdt_reset();
  manageFan(&fanInstance);
  manageHumidity(&hmdInstance);
  delay(2000);
}

void initHumidifier(humidifierData_t *hmd, uint32_t controlPin, DHT *sensor, float rhLowerBound, float rhUpperBound)
{
  hmd->currentState = STATE_ON;
  hmd->rhUpperBound = rhUpperBound;
  hmd->rhLowerBound = rhLowerBound;
  hmd->controlPin = controlPin;
  hmd->sensor = sensor;
  hmd->lastMeasured = 0UL;
  sensor->begin();
  pinMode(hmd->controlPin, OUTPUT);
}


void initFan(fanData_t *fan, uint32_t controlPin, uint32_t onTimeMinutesPerUnit, timeframe_t timeframe)
{
  fan->currentState = STATE_ON;
  fan->onPeriodMs = onTimeMinutesPerUnit * MILLISECONDS_PER_MINUTE;
  if (onTimeMinutesPerUnit < timeframe)
  {
    fan->offPeriodMs = (timeframe - onTimeMinutesPerUnit) * MILLISECONDS_PER_MINUTE;
  }
  else
  {
    fan->offPeriodMs = 0UL;
  }
  fan->controlPin = controlPin;
  fan->offTimeMs = millis();
  pinMode(fan->controlPin, OUTPUT);
  fan->onTimeMs = millis();
}

void manageFan(fanData_t *fan)
{
  uint32_t currentTime = millis();
  if (fan->currentState == STATE_ON)
  {
    #ifdef DEBUG
    Serial.print("Fan is on! Time (ms) till off: ");
    Serial.println(fan->onPeriodMs - (currentTime - fan->onTimeMs));
    #endif
    if ((currentTime - fan->onTimeMs) >= fan->onPeriodMs)
    {
      fan->currentState = STATE_OFF;
      fan->offTimeMs = millis();
    }
  }
  else if (fan->currentState == STATE_OFF)
  { 
    #ifdef DEBUG
    Serial.print("Fan is off! Time (ms) till on: ");
    Serial.println(fan->offPeriodMs - (currentTime - fan->offTimeMs));
    #endif
    if ((currentTime - fan->offTimeMs) >= fan->offPeriodMs)
    {
      fan->currentState = STATE_ON;
      fan->onTimeMs = millis();
    }
  }
  else
  {
     resetArduino();
  }
  digitalWrite(fan->controlPin, (uint32_t) fan->currentState);
}

void manageHumidity(humidifierData_t *hmd)
{
  // TODO modification: turn sensor on
  float currentRh = hmd->sensor->readHumidity();
  // TODO modification: turn sensor off
  if (hmd->currentState == STATE_ON)
  {
    #ifdef DEBUG
    Serial.print("Humidifier is on! ");
    #endif
    if (currentRh >= hmd->rhUpperBound)
    {
      hmd->currentState = STATE_OFF;
    }
  }
  else if (hmd->currentState == STATE_OFF)
  {
    //Serial.print("Humidifier is off! ");
    if (currentRh <= hmd->rhLowerBound)
    { 
      hmd->currentState = STATE_ON;
    }
  }
  else
  {
    resetArduino();
  }
  digitalWrite(hmd->controlPin, (uint32_t) hmd->currentState);
  
  #ifdef DEBUG
  Serial.print("Current Humidity: ");
  Serial.print(currentRh);
  Serial.print(" Humidity Lower: ");
  Serial.print(hmd->rhLowerBound);
  Serial.print(" Humidity Upper: ");
  Serial.println(hmd->rhUpperBound);
  #endif
}
