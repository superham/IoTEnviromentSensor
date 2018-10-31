/*
@Alex Kaariainen
Project Mimir Version A
Oct 7 2018

This software was developed for the following components:
  ESP32 WROOM MCU, GA1A12S202 Light Sensor, DHT22 Temperature/Humidity Sensor,
  Any low powered condensor microphone.

Mimir's purpose was to measure a room's sound levels, brightness, and Temperature/Humidity.
Then to post that to a google sheets page using Google's API and PushBox.
*/

#include <WiFi.h>
#include <DHT.h>

/*Pin definitions*/
#define GREENLEDPIN 12
#define REDLED0PIN 33
#define REDLED1PIN 27
#define DHTPIN 12
#define SOUNDSENSORPIN A0
#define LIGHTSENSORPIN A1

/*WiFi credentials*/
char* ssid="mimir";
char* password="password";

/*server credentials*/
char* devid = "v9218DC5A1E8A256";  //device id from pushbox
char server[] = "api.pushingbox.com";

/*Measurement variables*/
float data[10][4]; /*data[x]={temperature(F),humidity(%),sound level(dB), light level(lux)}*/

DHT dht(DHTPIN,DHT22);/*declartion for the dht sensor object*/

void setup() {

  Serial.begin(9600);
Serial.printf("\nprogram starting\n");

  /*c LEDs*/
  pinMode(GREENLEDPIN, OUTPUT);
  pinMode(REDLED0PIN, OUTPUT);
  pinMode(REDLED1PIN, OUTPUT);

  /*WiFi setup and connection*/
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    blinkLed(REDLED1PIN,500);//blink led to indicate an internet connection is being attempted
    Serial.printf("attempting to connect to:'%s' with:'%s' \n",ssid,password);
  }
  Serial.printf("connected to wifi\n");

  /*DHT22 Sensor setup*/
  dht.begin();
    
}

void loop() {
  Serial.printf("main loop starting\n");
  int i;

  /*sets each element in the 2-d array to 0*/
  memset(data, 0, sizeof(data));//TODO check that this works
  
  /*Take room measurments every 8.64 seconds 10 times*/
  for(i=0; i<10; i++){ //TODO check to make sure array is being filled
    
      data[i][0]=dht.readTemperature();
      data[i][1]=dht.readHumidity();
      data[i][2]=(analogRead(SOUNDSENSORPIN)+83.2073) / 11.003; //Convert ADC value to dB using the Regression values// TODO redo
      data[i][3]=rawToLux(analogRead(LIGHTSENSORPIN));
      Serial.printf("Temperature (C): %f \t \n",data[i][0]);

      //delay(8640);
      delay(500);
    }

  /*Sum each of the 10 measuremnts into the first row of the 2-d array*/
  //TODO work wiht a 1d array instead
  for(i=0; i<10; i++){ //TODO check this works!
    data[0][0]+=data[i][0];
    data[0][1]+=data[i][1];
    data[0][2]+=data[i][2];
    data[0][3]+=data[i][3];
    }
  /*Divide the first row by 10 to take the average*/
  for(i=0; i<4; i++){
    data[0][i]/=10;
    }

  /*Post the data*/
  /*data sent in a single string in the following format: temperature(C), humidity(%), sound level (dB), light level (lux)*/
  sprintf(postmsg,"GET /pushingbox?devid=%c&status=%f,%f,%f,%f HTTP/1.1",devid,data[0][0],data[0][1],data[0][2],data[0][3]);  // temperature value is inserted into the wed address. It follows 'status=' Change that value to whatever you want to post.
  
}


/*----------Class Functions Below------------*/

/*Blinks a selected LED for a selected period(ms)*/
void blinkLed(int ledPin, int delayTime){
    digitalWrite(ledPin, HIGH);
    delay(delayTime/2);
    digitalWrite(ledPin, LOW);
    delay(delayTime/2);
}

/*Converts raw ADC value from Light sensor to value in in LUX units*/
float rawToLux(int raw){
  float rawRange = 1024; // max digital value from adc when using 3.3V
  float logRange = 5;    // 3.3V = 10^5lux
  float logLux = raw * logRange / rawRange;
  return pow(10, logLux);
  }
