#include "Arduino_FreeRTOS.h"
#include "task.h"
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "TFT9341Touch.h"
#define PIN 2   // input pin Neopixel is attached to

// Shared data structure to store sensor readings
//מבנה נתונים לשמירת מאפיינים למשימות במקביל//
typedef struct {
  float temperature;
  float waterLevel;
  int timer = 0;
} SensorData;

SensorData sensorReadings;

//משתני טמפרטורה//
const int LM75_addr = 0x48;
float TEMP;

//טיימר//
int counter = 0;
int timeDesired = 30;


//בר לדים//
int LED_COUNT = 24;
Adafruit_NeoPixel strip(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);
int greenColor = 0;
int blueColor = 0;
int redColor = 0;


//הגדרת טווחי מעלות//
const int HIGHTTEMP[] = {90, 110};
const int MEDTEMP[] = {80, 89};
const int LOWTEMP[] = {70, 79};

//הבחירה של עוצמת הבישול//
int myChosenTemperature[2] = {90, 110};



//הגדלת להבה//
void highFlame(){
  greenColor = 25;
  blueColor = 5;
  redColor = 255;
}


//הנמכת להבה//
void lowFlame(){
  greenColor = 15;
  blueColor = 2;
  redColor = 60;
}

void flameExtinguished(){
  greenColor = 0;
  blueColor = 0;
  redColor = 0;
}


//פעולת שינוי הלהבה//
void changeFlameLevel(){
  for(int i = 0;i < LED_COUNT; i++){
    strip.setPixelColor(i, redColor, greenColor, blueColor);
    strip.show();
  }
}


void cook(){
  // Initialize Arduino library   
  init();
  //setup();

  // Create and start tasks
  xTaskCreate(temperatureSensorTask, "TemperatureSensorTask", 128, &timeDesired, 1, NULL);
  xTaskCreate(waterLevelSensorTask, "WaterLevelSensorTask", 128, &timeDesired, 1, NULL);
  Serial.print("water");
  xTaskCreate(counterTask, "counterTask", 128, &timeDesired, 1, NULL);

  // Start the FreeRTOS scheduler
  vTaskStartScheduler();
}



// Task for reading temperature sensor data
//משימה לקריאת החיישן טמפרטורה//
void temperatureSensorTask(int *pvParameters) {
    for( ;(*pvParameters) > counter;) {
        
           // Read temperature sensor data
        Wire.beginTransmission(LM75_addr);
        Wire.requestFrom(LM75_addr, 2, true);
        TEMP = ((Wire.read() << 8 | Wire.read()) >> 5) * 0.125;
        Serial.print("Temperature: ");
        Serial.print(TEMP);
        Serial.println(" oC");
      
        float temperature = TEMP;
        // Update shared data structure
        sensorReadings.temperature = temperature;

        if(temperature < myChosenTemperature[0]){
          highFlame();
          changeFlameLevel();
        }
        if(temperature >  myChosenTemperature[1]){
          lowFlame();
          changeFlameLevel();
        }
      
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay between readings 
      }
      vTaskDelete(NULL); 
 
}


// Task for reading water level sensor data
//משימה לקריאת החיישן רמת מים//
void waterLevelSensorTask(int *pvParameters) {
  for( ;(*pvParameters) > counter; ) {
            // Read water level sensor data
    int waterSensorPin = A0;
    float sensorValue = analogRead(waterSensorPin);
  
    Serial.print("Water Level: ");
    if(sensorValue > 0){
      Serial.println(sensorValue);
    }
    else {
      Serial.println("no water");
    }
    float waterLevel = sensorValue;
  
    // Update shared data structure
    sensorReadings.waterLevel = waterLevel;

    if(sensorValue > 400){
      lowFlame();
      changeFlameLevel();
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay between readings
  }
  vTaskDelete(NULL);

}


// Task for counting every second and updating the counter
//משימה לדימוי טיימר//
void counterTask(int *pvParameters) {
    for( ;(*pvParameters) > counter;counter++ ) {
      sensorReadings.timer = counter;
    
      // Print the counter value
      Serial.print("Counter: ");
      Serial.println(sensorReadings.timer);
    
      // Delay for 1 second
      vTaskDelay(1000 / portTICK_PERIOD_MS);

      /*int timeLeft = timeDesired - counter;
      char timerToShow[20];
      itoa(timeLeft, timerToShow, 20);*/
      
    }
    flameExtinguished();
    changeFlameLevel();
    vTaskDelete(NULL);
}



void setup() {
  
  Wire.begin();
  Wire.beginTransmission(LM75_addr);
  Wire.write(0x00);
  Wire.endTransmission(true);
  Serial.begin(9600);
  
  //בר לדים//
  strip.begin();
  strip.show();

}

void loop() { 
   // Initialize Arduino library   
  init();
  setup();

  // Create and start tasks
  xTaskCreate(temperatureSensorTask, "TemperatureSensorTask", 128, &timeDesired, 1, NULL);
  xTaskCreate(waterLevelSensorTask, "WaterLevelSensorTask", 128, &timeDesired, 1, NULL);
  xTaskCreate(counterTask, "counterTask", 128, &timeDesired, 1, NULL);

  // Start the FreeRTOS scheduler
  vTaskStartScheduler();
  

  
}  
