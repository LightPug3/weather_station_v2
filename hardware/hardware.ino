//##################################################################################################################
//##                                NATHAN GORDON'S WEATHER STATION SYSTEM CODE                                   ##
//##                                                                                                              ##
//##################################################################################################################
// IMPORT ALL REQUIRED LIBRARIES
#include <rom/rtc.h>
#include <MCUFRIEND_kbv.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <SPI.h>
#include <Adafruit_ST7735.h>              // Hardware-specific library
#include <Adafruit_BMP280.h>              // include  Adafruit BMP280 sensor library
#include <DHT.h>
#include <Wire.h>                         //include Two Wire Interface library
#include <math.h>


// ############### IMPORT HEADER FILES ##################
#ifndef _WIFI_H 
#include <WiFi.h>
// #include <HTTPClient.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif
 
#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif


// DEFINE SENSOR VARIABLES, OTHER VARIABLES AND PIN MAPPINGS:
#define DHTPIN            32
// #define BMP280_ADDRESS    (0x76)
#define soilMoisturePin   36    // ESP: SVP
#define LDRpin            39    // ESP: SVN
#define ARDUINOJSON_USE_DOUBLE      1 
#define DHTTYPE           DHT22

// TFT VARIABLES AND PIN MAPPINGS:
#define TFT_CS    5
#define TFT_RST   16
#define TFT_DC    17
#define TFT_MOSI  23
#define TFT_SCK   18
#define TFT_MISO  19

// TFT COLOUR SCHEMES
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREY    0x2108

// RING METER COLOUR SCHEMES
#define RED2RED     0
#define GREEN2GREEN 1
#define BLUE2BLUE   2
#define BLUE2RED    3
#define GREEN2RED   4
#define RED2GREEN   5

// 10-SEG BAR GRAPH LED
#define LED_SCK   14           // clockPin
#define LED_LATCH 12           // latchPin
#define LED_DATA  13           // datapin


/* Initialize class objects*/
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST, TFT_MISO);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280  bmp;
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();


// MQTT CLIENT CONFIG  
static const char* pubtopic      = "620156694";                     // Add your ID number here
static const char* subtopic[]    = {"620156694_sub","/elet2415"};   // Array of Topics(Strings) to subscribe to
static const char* mqtt_server   = "broker.hivemq.com";         // Broker IP address or Domain name as a String 
static uint16_t mqtt_port        = 1883;


// WIFI CREDENTIALS
// const char* ssid       = "WPS";                       // Add your Wi-Fi ssid
// const char* password   = "W0LM3R$WP$";                // Add your Wi-Fi password 
const char* ssid       = "MonaConnect";
const char* password   = "";


// TASK HANDLES 
TaskHandle_t xMQTT_Connect          = NULL; 
TaskHandle_t xNTPHandle             = NULL;  
TaskHandle_t xLOOPHandle            = NULL;  
TaskHandle_t xUpdateHandle          = NULL;
TaskHandle_t xButtonCheckeHandle    = NULL;  


/* Declare and Initialize variables to use in code: */
float h                   = 0;
float t                   = 0;
float f                   = 0;
float hif                 = 0;
float hic                 = 0;
float D                   = 0;
float ah                  = 0;            // absolute humidity = water vapor density in g/m*3
// float rh                  = 0;         // relative humidity in %
float td                  = 0;            // dew point in celsius
float tf                  = 0;            // temperature in fahrenheit
float tdf                 = 0;            // dew point temperature in fahrenheit
float pressure            = 0;
float altitude            = 0;
int soil_moisture         = 0;
int mapped_soil_moisture  = 0;
int lightIntensity        = 0;
int mapped_lightIntensity = 0;
int ledLevel              = 0;
// int reading               = 0;            // Value to be displayed
// int reading1              = 0;            // Value to be displayed
int d                     = 0;            // Variable used for the sinewave test waveform
int tesmod                = 0;
int count                 = 0;
const int ledCount        = 8;
uint32_t runTime          = -99999;       // time for next update
int8_t ramp               = 1;
boolean alert             = 0;
char lightString [4];
String str;
char TempCelciusFahrenheit[6];
extern uint8_t temperatureIcon[];


/** FUNCTION DECLARATIONS **/
void checkHEAP(const char* Name);         // RETURN REMAINING HEAP SIZE FOR A TASK
void initMQTT(void);                      // CONFIG AND INITIALIZE MQTT PROTOCOL
unsigned long getTimeStamp(void);         // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
void callback(char* topic, byte* payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
void vButtonCheck(void * pvParameters);
void vUpdate(void * pvParameters);  
bool isNumber(double number);
void LEDBarGraph(int );
double dewPoint(double celsius, double humidity){
  double RATIO = 373.15 / (273.15 + celsius);
  double RHS = -7.90298 * (RATIO - 1);
  RHS += 5.02808 * log10(RATIO);
  RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / RATIO ))) - 1);
  RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1);
  RHS += log10(1013.246);
  double VP = pow(10, RHS - 3) * humidity;
  double T = log(VP / 0.61078);                                   // temp var
  return (241.88 * T) / (17.558 - T);
}


/* WEATHER ICON BITMAPS */
uint8_t sun[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x38, 0x00,
  0x80, 0x06, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x1c,
  0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x03, 0x03, 0xf0, 0x20, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x70, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x01,
  0xc0, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0xc0, 0x00,
  0x00, 0x00, 0x01, 0x80, 0x00, 0x60, 0x00, 0x00, 0x0f, 0xf1, 0x80, 0x00, 0x63, 0xfc, 0x00, 0x0f,
  0xe1, 0x80, 0x00, 0x63, 0xf8, 0x00, 0x00, 0x01, 0x80, 0x00, 0x60, 0x00, 0x00, 0x00, 0x01, 0x80,
  0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x80,
  0x00, 0x00, 0x00, 0x00, 0x60, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x38, 0x07, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x1e, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x03,
  0x01, 0xe0, 0x30, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00,
  0x1c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x38, 0x00, 0xc0, 0x07, 0x00,
  0x00, 0x00, 0x20, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t partlyCloudyDay[]  = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x06,
  0x00, 0xc0, 0x18, 0x00, 0x00, 0x00, 0x07, 0x00, 0xc0, 0x38, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00,
  0x70, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xc3, 0xf0, 0xc0, 0x00,
  0x00, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x1c, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x38, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x30, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x60,
  0x03, 0xc0, 0xe0, 0x00, 0x00, 0x00, 0x60, 0x07, 0x00, 0x30, 0x00, 0x00, 0xfe, 0x60, 0x0e, 0x00,
  0x18, 0x00, 0x00, 0xfe, 0x60, 0x0c, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x60, 0x1c, 0x00, 0x0c, 0x00,
  0x00, 0x00, 0x70, 0x18, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x30, 0x18, 0x00, 0x07, 0xfc, 0x00, 0x00,
  0x18, 0x18, 0x00, 0x04, 0x0e, 0x00, 0x00, 0x1c, 0x18, 0x00, 0x00, 0x07, 0x00, 0x00, 0x04, 0x78,
  0x00, 0x00, 0x03, 0x00, 0x00, 0xc1, 0xf8, 0x00, 0x00, 0x01, 0x80, 0x01, 0xc3, 0x80, 0x00, 0x00,
  0x01, 0x80, 0x03, 0x83, 0x00, 0x00, 0x00, 0x01, 0x80, 0x07, 0x06, 0x00, 0x00, 0x00, 0x00, 0xc0,
  0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x06,
  0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x06, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x03, 0x00, 0x00,
  0x00, 0x03, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x3c,
  0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t cloud[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f,
  0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x78, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x80, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0e, 0x00, 0x00, 0x00,
  0x00, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x0c,
  0x00, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x0c, 0x00, 0x01,
  0x81, 0xc0, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x30,
  0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x38, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x18, 0x00, 0x07,
  0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00,
  0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00,
  0x18, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00,
  0x0e, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x03, 0x00,
  0x00, 0x00, 0x00, 0xe0, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0xff, 0xff, 0xff,
  0xff, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t rain[]  = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xe0, 0x78, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x1c, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x03, 0xff,
  0x80, 0x00, 0x00, 0x0c, 0x00, 0x01, 0x81, 0xc0, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xe0, 0x00,
  0x00, 0x7c, 0x00, 0x00, 0x00, 0x30, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x38, 0x00, 0x03, 0x80,
  0x00, 0x00, 0x00, 0x18, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x06, 0x00, 0x00, 0x00,
  0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x0c,
  0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x0c,
  0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x06, 0x00, 0x00,
  0x00, 0x00, 0x70, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03,
  0xc0, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x80, 0x60, 0x00, 0x00, 0x00, 0x0c, 0x03, 0x00,
  0xc0, 0x00, 0x00, 0x00, 0x1c, 0x02, 0x01, 0x80, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x30, 0x0c, 0x00, 0x00, 0x00, 0x01,
  0x80, 0x60, 0x18, 0x00, 0x00, 0x00, 0x03, 0x80, 0x40, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t snow[]  = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xe0, 0x78, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x1c, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x03, 0xff,
  0x80, 0x00, 0x00, 0x0c, 0x00, 0x01, 0x81, 0xc0, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xe0, 0x00,
  0x00, 0x7c, 0x00, 0x00, 0x00, 0x30, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x38, 0x00, 0x03, 0x80,
  0x01, 0x80, 0x00, 0x18, 0x00, 0x07, 0x00, 0x01, 0x80, 0x00, 0x18, 0x00, 0x06, 0x00, 0x07, 0xc0,
  0x00, 0x0c, 0x00, 0x0c, 0x00, 0x03, 0xc0, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0xf1, 0x9f, 0x00, 0x0c,
  0x00, 0x0c, 0x00, 0xf1, 0x9e, 0x00, 0x18, 0x00, 0x0c, 0x00, 0x7d, 0xbe, 0x00, 0x18, 0x00, 0x0c,
  0x00, 0x6f, 0xf6, 0x00, 0x18, 0x00, 0x0e, 0x00, 0x07, 0xc0, 0x00, 0x30, 0x00, 0x06, 0x00, 0x07,
  0xe0, 0x00, 0x70, 0x00, 0x03, 0x00, 0x7f, 0xfe, 0x00, 0xe0, 0x00, 0x03, 0xc0, 0x79, 0x9e, 0x03,
  0xc0, 0x00, 0x00, 0xf8, 0xf1, 0x9f, 0x1f, 0x00, 0x00, 0x00, 0x30, 0xf1, 0x9a, 0x1c, 0x00, 0x00,
  0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x80, 0x01, 0x80, 0x00, 0x01, 0x80, 0x01, 0x00, 0x01, 0xf0, 0x00, 0x01, 0xf0, 0x00, 0x00,
  0x01, 0xe0, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0xc0,
  0x00, 0x00, 0x60, 0x00, 0xc0, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
  0x70, 0x30, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x20, 0x3e, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x7c,
  0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x80, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


#ifndef MQTT_H
#include "mqtt.h"
#endif

#ifndef NTP_H
#include "NTP.h"
#endif


void setup() {
  Serial.begin(115200);
  
  // LED PIN CONFIGS
  pinMode(LED_SCK, OUTPUT);
  pinMode(LED_LATCH, OUTPUT);
  pinMode(LED_DATA, OUTPUT);

  Serial.println("Weather Station Test!");

  // initialize SPI
  SPI.begin();

  // initialize DHT
  dht.begin();

  // initialize bmp280
  // if(bmp280.begin(BMP280_I2C_ADDRESS) == 0 ) {          // connection error or device address wrong!
  //   Serial.print("Connection Error");
  //   Serial.println(BMP280_I2C_ADDRESS);
  //   while(1);
  // }

  // bmp.begin();

  while ( !Serial ) delay(100);   // wait for native usb
  unsigned status;
  status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("        ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  // bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
  //                 Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
  //                 Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
  //                 Adafruit_BMP280::FILTER_X16,      /* Filtering. */
  //                 Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  // initialize TFT
  tft.begin(0x1580);
  
  tft.setRotation(1);               // Set Rotation at 0 degress (default)

  tft.fillScreen(BLACK);            //Set Background Color with BLACK

  // INIT WIFI, MQTT & NTP 
  initialize();
  // vButtonCheckFunction();        // UNCOMMENT IF USING BUTTONS THEN ADD LOGIC FOR INTERFACING WITH BUTTONS IN THE vButtonCheck FUNCTION

}

void loop() {
  /* reading sensor data: */
  sensors_event_t pressure_event;

  // dht temp and humid:
  h   = dht.readHumidity();
  t   = dht.readTemperature();
  f   = dht.readTemperature(true);
  hif = dht.computeHeatIndex(f, h);
  hic = dht.computeHeatIndex(t, h, false);
  D   = (dewPoint(t, h ));

  // bmp pressure & altitude:
  bmp_pressure->getEvent(&pressure_event);
  altitude = bmp.readAltitude(1013.25);
  pressure = pressure_event.pressure;

  // soil moisture:
  soil_moisture = analogRead(soilMoisturePin);
  mapped_soil_moisture = map(soil_moisture, 3300, 1500, 0, 4095);         // air, water, lowest percent, highest percent
  
  // light sensor:
  lightIntensity = analogRead(LDRpin);
  mapped_lightIntensity = map(lightIntensity, 0, 4095, 100, 0);
  // str = String(mapped_lightIntensity)+"%";
  // str.toCharArray(lightString,5);  // LED BAR GRAPH indicator: based on HUMIDITY
  
  // int ledLevel = map(soil_moisture, 4095, 3300, 0, ledCount);                // led bar graph meter
  ledLevel = map(h, 0, 100, 0, ledCount);                                       // led bar graph meter
  LEDBarGraph(ledLevel);

  // deal with cases where mapped_soil_moisture may occasionally exceed 100 and go below 0:
  if(mapped_soil_moisture>100){mapped_soil_moisture=100;}

  if(mapped_soil_moisture<0){mapped_soil_moisture=0;}


  // absolute humidity calculaton:
  ah = (6.112 * pow(2.71828, ((17.67 * t) / (243.5 + t))) * h * 2.1674) / (273.15 + t);
  
  // DEBGGING, TO REMOVE:
  count+=1;

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read DHT sensor!");
    return;
  }

  int xpos = 0, ypos = 5, gap = 4, radius = 52;
  
  // Draw a large meter
  xpos = 320/2 - 150, ypos = 15, gap = 100, radius = 50;
  
  // Draw analogue meter: temp in C (DHT)
  ringMeter(t, 0, 40, 0, 0, radius, "CELSIUS", GREEN );

  // Draw analogue meter: temp in F (DHT)
  // ringMeter(f,0,150, 108,0,radius,"FAHRENHEIT",GREEN );
  
  // Draw analogue meter: pressure in hPa (BMP)
  ringMeter(pressure, 0, 1100, 108, 0, radius, "PRESSURE", GREEN);

  // Draw analogue meter: humidity (DHT)
  ringMeter(h, 0, 100, 215, 0, radius, "HUMIDITY", GREEN);

  // Draw analogue meter: abs. hum (DHT)
  // ringMeter(ah, 0,40, 0,120,radius,"Abs. Hum",GREEN );
 
  // Draw analogue meter: altitude (BMP)
  ringMeter(altitude, 0, 2000, 0, 120, radius, "ALTITUDE", GREEN);

  // Draw analogue meter: dew point (DHT)
  // ringMeter(D, 0,40, 108,120,radius,"Dew P",GREEN );

  // Draw analogue meter: soil moisture (SOIL_MOISTURE)
  ringMeter(soil_moisture, 0, 100, 108, 120, radius, "SOIL MOISTURE", GREEN);

  // update DHT temp. & unit
  tft.setCursor (32, 90);
  tft.setTextSize (1);
  tft.setTextColor (GREEN, BLACK);
  tft.print (t, 1); 
  tft.setTextSize (1);
  tft.setTextColor (GREEN, BLACK);
  tft.print (" C ");
  tesmod = 1;

  // update BMP Pressure & UNIT only
  tft.setCursor (138, 90);
  tft.setTextSize (1);
  tft.setTextColor (MAGENTA, BLACK);
  tft.print (pressure, 1); 
  // tft.setCursor(250, 50);
  tft.setTextSize (1);
  tft.setTextColor (MAGENTA, BLACK);
  tft.print (" hPa ");
  
  // update DHT Humidity & unit
  tft.setCursor (247, 90);
  tft.setTextSize (1);
  tft.setTextColor (GREEN, BLACK);
  tft.print (h, 1);
  tft.setTextSize (1);
  tft.setTextColor (GREEN, BLACK);
  tft.print (" % ");
  tesmod = 1;

  // update BMP altitude & unit
  tft.setCursor(27, 210);  
  tft.setTextSize (1);
  tft.setTextColor ( YELLOW , BLACK);
  tft.print (altitude);
  //tft.setCursor(250, 50);  
  tft.setTextSize (1);
  tft.setTextColor ( YELLOW , BLACK);
  tft.print (" m ");
  
  // update the soil moisture & unit
  tft.setCursor (142,210);
  tft.setTextSize (1);
  tft.setTextColor (GREEN,BLACK);
  tft.print (mapped_soil_moisture);
  tft.setTextSize (1);
  tft.setTextColor (GREEN,BLACK);
  tft.print (" % ");
  tesmod=1;

  // PERSONALIZED TEXT:
  tft.setCursor(229, 130);  
  tft.setTextSize (1);
  tft.setTextColor ( MAGENTA , BLACK);
  tft.print("Nathan's Mini");
  tft.setCursor(222, 139);  
  tft.setTextSize (1);
  tft.setTextColor ( MAGENTA , BLACK);
  tft.print("Weather Station");

  // FORECAST ICON
  tft.setCursor(245, 170);  
  tft.setTextSize (1);
  tft.setTextColor ( YELLOW , BLACK);
  tft.print("Forecast");

  // PRINT FORECAST BASED ON TEMP AND HUMID CALCULATION
  // rainy
  if((h)>70 ) {
    tft.drawBitmap(245, 185, rain, 50, 50, BLUE, BLACK);
  }

  // sunny
  else if((t)>25 && (h)<50 ) {
    tft.drawBitmap(245, 185, sun, 50, 50, YELLOW, BLACK);    
  }
  
  // partly cloudy day
  else if((t)>20 && (t)<30 && (h)>40 && (h)<60 ) {
    tft.drawBitmap(245, 185, partlyCloudyDay, 50, 50, WHITE, BLACK);
  }

  // snowy
  else if((t)>5 && (h)>70 ) {
    tft.drawBitmap(245, 185, snow, 50, 50, WHITE, BLACK);
  }
  
  // cloudy 
  else {
    tft.drawBitmap(245, 185, cloud, 50, 50, WHITE, BLACK);      
  }

  if (millis() - runTime >= 1000) {     // Execute every 500ms
    runTime = millis();
  }

  // DEBUGGING, PRINTING TO CONSOLE:
  Serial.print(count);
  Serial.println();

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");

  Serial.print("Fahrenheight: ");
  Serial.print(f);
  Serial.println(" *F\t");

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%\t");

  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.print(" hPa");
  Serial.println();

  Serial.print("Approx. Altitude: ");
  Serial.print(altitude);
  Serial.println(" m");

  Serial.print("Raw Soil Moisture: ");
  Serial.print(soil_moisture);
  Serial.print("%");
  Serial.println();

  Serial.print("Processed Soil Moisture: ");
  Serial.print(mapped_soil_moisture);
  Serial.print("%");
  Serial.println();

  Serial.print("Heat index F: ");
  Serial.print(hif);
  Serial.println(" *F ");

  Serial.print("Heat index C: ");
  Serial.print(hic);
  Serial.println(" *C ");

  Serial.print("Dew Point: ");
  Serial.print(dewPoint(t, h ));
  Serial.println(" Celsius ");
  
  Serial.print("Raw Light: ");
  Serial.println(lightIntensity);
  
  Serial.print("Mapped Light: ");
  Serial.print(mapped_lightIntensity);
  Serial.println(" % ");

  // Serial.print("DS Temp: ");
  // Serial.print(tempC);
  // Serial.println(" *C ");

  Serial.println();
  // delay(1000);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
//####################################################################
//#                          UTIL FUNCTIONS                          #
//####################################################################
void vButtonCheck( void * pvParameters )  {
  configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
  for( ;; ) {
    // Add code here to check if a button(S) is pressed
    // then execute appropriate function if a button is pressed  

    vTaskDelay(200 / portTICK_PERIOD_MS);  
  }
}

void vUpdate( void * pvParameters )  {
  configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
  for( ;; ) {
    // #######################################################
    // ## This function must PUBLISH to topic every second. ##
    // #######################################################

    if(isNumber(t)){
      // ##Publish update according to ‘{"id": "student_id", "timestamp": 1702212234, "temperature": 30, "pressure":1012, "humidity":40, "altitude":159, "soil_moisture":3018}’

      // 1. Create JSon object
      StaticJsonDocument<1000> doc; // Create JSon object
      
      // 2. Create message buffer/array to store serialized JSON object
      char message[1100]  = {0};

      // 3. Add key:value pairs to JSon object based on above schema
      doc["id"]               = "620156694";
      doc["timestamp"]        = getTimeStamp();
      doc["temperature"]      = t;
      doc["pressure"]         = pressure;
      doc["humidity"]         = h;
      doc["altitude"]         = altitude;
      doc["soil_moisture"]    = soil_moisture;     // mapped_

      // 4. Seralize / Covert JSon object to JSon string and store in message array
      serializeJson(doc, message);

      // 5. Publish message to a topic sobscribed to by both backend and frontend                
      if(mqtt.connected() ){
        publish(pubtopic, message);
      }

      Serial.printf("Published Message: %s\n\n", message);

    }
  vTaskDelay(1000 / portTICK_PERIOD_MS);  
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // // ############## MQTT CALLBACK  ######################################
  // // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  
  // Serial.printf("\nMessage received : ( topic: %s ) \n",topic ); 
  // char *received = new char[length + 1] {0}; 
  
  // for (int i = 0; i < length; i++) { 
  //   received[i] = (char)payload[i];    
  // }

  // // PRINT RECEIVED MESSAGE
  // Serial.printf("Payload : %s \n",received);

 
  // // CONVERT MESSAGE TO JSON
  // StaticJsonDocument<1000> doc;
  // DeserializationError error = deserializeJson(doc, received);  

  // if (error) {
  //   Serial.print("deserializeJson() failed: ");
  //   Serial.println(error.c_str());
  //   return;
  // }


  // // PROCESS MESSAGE
  // const char* type = doc["type"]; 
  // // process messages with ‘{"type": "controls", "brightness": 255, "leds": 7, "color": { "r": 255, "g": 255, "b": 255, "a": 1 } }’ schema

  // if (strcmp(type, "controls") == 0){
  //   // 1. EXTRACT ALL PARAMETERS: NODES, RED,GREEN, BLUE, AND BRIGHTNESS FROM JSON OBJECT
  //   int nodes       = doc["leds"]; 
  //   int brightness  = doc["brightness"];
  //   int red         = doc["color"]["r"];
  //   int green       = doc["color"]["g"];
  //   int blue        = doc["color"]["b"];


  //   // 2. ITERATIVELY, TURN ON LED(s) BASED ON THE VALUE OF NODES. Ex IF NODES = 2, TURN ON 2 LED(s)
  //   for(unsigned char i=0; i<nodes; i++){
  //     leds[i]=CRGB(red, green, blue);
  //     FastLED.setBrightness(brightness);
  //     FastLED.show();
  //     delay(50);
  //   }
  //   // 3. ITERATIVELY, TURN OFF ALL REMAINING LED(s).
  //   for(unsigned char x=nodes; x<NUM_LEDS; x++){
  //     leds[x]=CRGB::Black;
  //     FastLED.setBrightness(brightness);
  //     FastLED.show();
  //     delay(50);
  //   }
  // }
}

unsigned long getTimeStamp(void) {
  // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
  time_t now;         
  time(&now);         // Retrieve time[Timestamp] from system and save to &now variable
  return now;
}

bool publish(const char *topic, const char *payload){   
  bool res = false;
  try{
    res = mqtt.publish(topic,payload);
    // Serial.printf("\nres : %d\n",res);
    if(!res){
      res = false;
      throw false;
    }
  }
  catch(...){
    Serial.printf("\nError (%d) >> Unable to publish message\n", res);
  }
  return res;
}

bool isNumber(double number){
  char item[20];
  snprintf(item, sizeof(item), "%f\n", number);
  if(isdigit(item[0]))
    return true;
  return false;
}

int ringMeter(int value, int vmin, int vmax, int x, int y, int r, char *units, byte scheme) {
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;   // Calculate coords of centre of ring
  int w = r / 3;    // Width of outer ring is 1/4 of radius 
  int angle = 150;  // Half the sweep angle of meter (300 degrees)
  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v
  byte seg = 3; // Segments are 3 degrees wide = 100 segments for 300 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring
  // Variable to save "value" text colour from scheme and set default
  int colour = GREEN;
 
  // Draw colour blocks every inc degrees
  for (int i = -angle+inc/2; i < angle-inc/2; i += inc) {
    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      switch (scheme) {
        case 0: colour = GREEN; break; // Fixed colour
        case 1: colour = RED; break; // Fixed colour
        case 2: colour = BLUE; break; // Fixed colour
        case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
        case 4: colour = rainbow(map(i, -angle, angle, 70, 127)); break; // Green to red (high temperature etc)
        case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
        default: colour = GREEN; break; // Fixed colour
      }
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      //text_colour = colour; // Save the last colour drawn
    }
    else {        // Fill in blank segments
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, RED);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, BLACK);
    }
  }
  // Convert value to a string
  char buf[10];
  byte len = 2; if ((value > 99) && (value <= 999)) len = 3; else if (value > 999) len = 4;   //        else if (value <= 9) len = 1;
  dtostrf(value, len, 0, buf);
  buf[len] = ' '; buf[len] = 0;             // Add blanking space and terminator, helps to centre text too!
  // Set the text colour to default

  // Serial.println(buf);
  // Serial.println(len);

  tft.setTextSize(1);

  if (len==3){
  tft.setTextColor(CYAN,BLACK);
  tft.setCursor(x-25,y-10);tft.setTextSize(3);
  tft.print(buf);}

  // else{
    if(value>9){
    tft.setTextColor(CYAN,BLACK);
    tft.setCursor(x-25,y-10);
    tft.setTextSize(3);
    if(len==4){tft.setTextSize(2.5);}
    tft.print(buf);}

    if(value<10){
    tft.setTextColor(colour,BLACK);
    tft.setCursor(x-25,y-10);tft.setTextSize(3);
    tft.print(buf);}

    // tft.setTextColor(CYAN,BLACK);
    // tft.setCursor(x-25,y-10);tft.setTextSize(3);
    // tft.print(buf);

    // VARIABLE LABELS
    tft.setTextColor(CYAN,BLACK);
    tft.setCursor(x-20,y+55);tft.setTextSize(1);
    tft.print(units);
  // }
  // Calculate and return right hand side x coordinate
  return x + r;
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value) {
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits
  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// #########################################################################
float sineWave(int phase) {
  return sin(phase * 0.0174532925);
}

void LEDBarGraph(int ledLevel) {
  // Shift out the data to the shift register
  digitalWrite(LED_LATCH, LOW);
  shiftOut(LED_DATA, LED_SCK, MSBFIRST, (1 << ledLevel) - 1); // shift out the appropriate number of 1s
  digitalWrite(LED_LATCH, HIGH);
}