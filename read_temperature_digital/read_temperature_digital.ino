#include "DHT.h"
 
#define DHTPIN 2        // SDA 핀의 설정
#define DHTTYPE DHT22   // DHT22 (AM2302) 센서종류 설정

int8_t temperature, humid;
 
DHT dht(DHTPIN, DHTTYPE);
 

void read_temp_humid(){
  humid =  dht.readHumidity();
  temperature = dht.readTemperature();
}



void setup() {
  Serial.begin(9600); 
  dht.begin();
}
 


void loop() {
  // 센서의 온도와 습도를 읽어온다.
  
  read_temp_humid();
  if (isnan(temperature) || isnan(humid)) {
    //값 읽기 실패시 시리얼 모니터 출력
    Serial.println("Failed to read from DHT");
  } else {
    //온도, 습도 표시 시리얼 모니터 출력
    Serial.print("Humidity: "); 
    Serial.print(humid);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
    Serial.print(temperature);
    Serial.println(" *C");
  }
  delay(1000);
}
 