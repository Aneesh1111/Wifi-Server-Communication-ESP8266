#include <Arduino.h>
#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
#include "WiFiClient.h"
#include "ESP8266WebServer.h"
#include <Arduino_JSON.h>

/*
/// HTTP client errors
#define HTTPC_ERROR_CONNECTION_FAILED   (-1)
#define HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
#define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
#define HTTPC_ERROR_NOT_CONNECTED       (-4)
#define HTTPC_ERROR_CONNECTION_LOST     (-5)
#define HTTPC_ERROR_NO_STREAM           (-6)
#define HTTPC_ERROR_NO_HTTP_SERVER      (-7)
#define HTTPC_ERROR_TOO_LESS_RAM        (-8)
#define HTTPC_ERROR_ENCODING            (-9)
#define HTTPC_ERROR_STREAM_WRITE        (-10)
#define HTTPC_ERROR_READ_TIMEOUT        (-11)
*/

/* global variables for server connection */
const char *ssid = "";     // Enter SSID
const char *password = ""; // Enter Password
const char* serverName = ""; 
// const char* serverName = "http://date.jsontest.com/";
// const char *serverName = "http://arduinojson.org/example.json";


/* global variables for the interrupt */
const byte interrupt_pin = 13;  // Pin D7 on the board
volatile bool interrupt_counter = false;
int number_of_interrupts = 0;
int current_time = millis();
int delay_between_interrupts_ms = 1000;

/* Interrupt handler function -> sets interrupt_counter - if statement reduces debouncing */
void IRAM_ATTR handleInterrupt() {
  if(millis()-current_time > delay_between_interrupts_ms) {
    interrupt_counter = true;
  }
}


void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting...");
  }

  Serial.println("");
  Serial.println("WiFi connection Successful");
  Serial.print("The IP Address of ESP8266 Module is: ");
  Serial.print(WiFi.localIP()); // Print the IP address

  /* set pin D7 as an interrupt pin - set it high (pullup) */
  pinMode(interrupt_pin, INPUT_PULLUP);
  /* set handleInterrupt function as ISR for interrupts - trigger on falling edge */
  attachInterrupt(digitalPinToInterrupt(interrupt_pin), handleInterrupt, FALLING);
}

void loop()
{

  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverName);
  int httpResponseCode = http.GET();
  String payload = "{}";

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
    Serial.println(payload);

    JSONVar muOb = JSON.parse(payload);
    JSONVar keys = muOb.keys();
    for (int i = 0; i < keys.length(); i++)
    {
      JSONVar value = muOb[keys[i]];
      Serial.print(keys[i]);
      Serial.print(" = ");
      Serial.println(value);
    }

    delay(1000);

    if(interrupt_counter) {

      interrupt_counter = false;
      number_of_interrupts++;
      Serial.print(number_of_interrupts);
      Serial.println(" Interrupts Have Occurred");

      http.addHeader("Content-Type", "application/json");
      http.POST("{\"value3\":\"1005.14\"}");
      
      current_time = millis();
    }

  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
  delay(2000);
}
