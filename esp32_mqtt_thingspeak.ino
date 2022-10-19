#include <SimpleDHT.h>
#include "ThingSpeak.h"
#include <WiFi.h>
#include "mysecret.h"

#define BLINK_GPIO (gpio_num_t)12
int pinDHT = 15;
SimpleDHT11 dht11(pinDHT);
unsigned long myChannelNumber = 1898172;
const char * myWriteAPIKey = "M5BMU8SW099Y9JRO";
const char * myReadAPIKey = "D3O86YSOYT68NNTY";
char ssid[] = myWifiSSID;   // your network SSID (name)
char pass[] = myWifiPwd;   // your network password
WiFiClient  client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  gpio_pad_select_gpio(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("IP address:");
  Serial.println(WiFi.localIP());
}

void loop() {
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); 
    Serial.println(err); 
    delay(1000);
    return;
  }
  Serial.print("DHT11 Sample OK: ");
  Serial.print((int)temperature); Serial.print(" ºC, "); 
  Serial.print((int)humidity); Serial.println(" H");  
  vTaskDelay(15000 / portTICK_PERIOD_MS); 
   
  ThingSpeak.setField(2, (int)temperature);
  ThingSpeak.setField(3, (int)humidity);
  int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (httpCode == 200) {
    Serial.println("Channel write successful.");
  }
  else {
    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
  }
  vTaskDelay(15000 / portTICK_PERIOD_MS); 
  // Read in field 1 of the private channel
  long led_status = ThingSpeak.readLongField(myChannelNumber, 1, myReadAPIKey);  
  // Check the status of the read operation to see if it was successful
  httpCode = ThingSpeak.getLastReadStatus();
  if(httpCode == 200){
    Serial.println("led_status: " + String(led_status));
    if (led_status == 1)
       gpio_set_level(BLINK_GPIO, 1);
    else if (led_status == 0)
       gpio_set_level(BLINK_GPIO, 0);
  }
  else{
    Serial.println("Problem reading channel. HTTP error code " + String(httpCode)); 
  }
  
}
