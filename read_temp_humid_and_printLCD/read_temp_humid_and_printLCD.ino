//-----------------MEMO---------------------


// LCD Display A4, A5 pin은 I2C 통신때문에 고정적으로 사용해야함!
// DHT-22 센서는 digital pin



//---------------------------header files---------------------

#include <Wire.h>                     // i2C 통신을 위한 라이브러리
#include <LiquidCrystal_I2C.h>        // LCD 2004 I2C용 라이브러리
#include "DHT.h"                       // DHT 센서 라이브러리



//-------------------define---------------------------------------------
#define DHTTYPE DHT22   // DHT22 (AM2302) 센서종류 설정


#define led_insdie_pin 0x20   // 내장 led pin번호
#define DHTPIN 2        // SDA 핀의 설정



//-------------------variables------------------------------------------

LiquidCrystal_I2C lcd(0x27, 20, 4);   // 접근주소: 0x3F or 0x27
DHT dht(DHTPIN, DHTTYPE);

int8_t temperature, humidity;
bool water_refill;


//------------------------------setup functions----------------------------------------------------

//Serial communication setup
void init_Serial(){
  Serial.begin(9600);
  // delay(1000);
}

void display_setup()
{
  lcd.begin();                     // LCD 초기화
  lcd.backlight();                 // 백라이트 켜기
  
  lcd.setCursor(0, 0);             // 첫번째 줄 문자열 출력
  lcd.print("Temperature : ??");
 
  lcd.setCursor(0, 1);             // 두번째 줄 문자열 출력
  lcd.print("Humidity : ??");
 
  lcd.setCursor(0, 2);             // 세번째 줄 문자열 출력
  lcd.print("Hello World!!");
  
  lcd.setCursor(0, 3);             // 네번째 줄 문자열 출력
  lcd.print("Water Refill!!");
}

void init_insideLED(){
  //내장led 키려고 하는거니까 PortB의 5번 비트 출력으로 설정
  // DDRB = 0b 0010 0000
  DDRB |= (1 << led_insdie_pin);
}



//ADC setup
void init_ADC(){
  //_ADMUX 


  // Voltage reference를 AVcc로..
  // REFS[1:0] == 01;
  ADMUX |= (0<<REFS1) | (1<<REFS0);

  // ADC Left Adjust 안하고 오른쪽으로 정렬
  ADMUX |= (0<<ADLAR);

  // ADC enable ADC가 작동할 준비
  ADCSRA |= (1 << ADEN);

  // prescaler 128로..(cpu 128 clock에 adc 1번)
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 
}


//-----------------------action------------------------------------------------

void display_action(int8_t temperature, int8_t humidity, bool water_refill)
{
  lcd.setCursor(0, 0);             // 첫번째 줄 문자열 출력
  // lcd.print("                      "); // 비우기     
  // lcd.setCursor(0, 0);
  lcd.print("Temperature : ");
  lcd.print(temperature);
  lcd.print("oC");

 
  lcd.setCursor(0, 1);             // 두번째 줄 문자열 출력
  // lcd.print("                       "); // 비우기     
  // lcd.setCursor(0, 1);
  lcd.print("Humidity : ");
  lcd.print(humidity);
  lcd.print("%");
  
 
  lcd.setCursor(0, 2);             // 세번째 줄 문자열 출력
  // lcd.print("                              "); // 비우기     
  // lcd.setCursor(0, 2);
  lcd.print("Hello World!!");
  
  lcd.setCursor(0, 3);             // 네번째 줄 문자열 출력
  // lcd.print("                              "); // 비우기     
  // lcd.setCursor(0, 3);
  lcd.print("Water Refill : ");
  lcd.print(water_refill ? "Yes" : "No");
}


// 온도체크
// 일단은 interrupt 안하고 간단히 busywait으로 구현
// busywait함수를 loop함수에 직접적으로 넣어야하나 아니면 개별 함수마다 넣어야하나?
// 나중에 interrupt 구현하면 상관없을거같긴 한데 일단은 개별 함수마다 구현해야겠다
int8_t read_temperature(){
  // 뭐 지금 당장은 ADC0으로 들어오게 한다 해야겠다
  // !!!!!!!!!!!!!!!!!!통합할때 수정 필요!!!!!!!!!!!!!!!!!!!
  ADMUX |= (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);

  ADCSRA |= (1 << ADSC); // ADC 변환 시작

  // CPU가 ADSC 비트 1로 바꿔줄때까지 busywait 
  while (ADCSRA & (1 << ADSC));
  // 8비트 레지스터 두개 합쳐서 16비트의 값으로 바꾸고 temp_temperature 변수에 저장 (지금은 전압값, 0~1023사이의 값)
  uint16_t temp_temperature = (uint16_t)ADCL | ((uint16_t)(ADCH) << 8);

  // 이다음에 실제 온도값으로 변경
  // 계산과정에서 정수말고 실수로 될 수 도 있으니 float하고
  // datasheet 분석하면 T = 100Vout

  temperature = (float)temp_temperature * 5.0 / 1024.0 * 100.0;
  return temperature;
}

int8_t read_humidity(){
  return (int8_t) dht.readHumidity();
}



void serial_print(int8_t temperature, int8_t humidity){
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, ");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  delay(1000);
}


//-------------------------MAIN FUNCTION------------------------------




void setup() {
  // put your setup code here, to run once:
  display_setup();
  init_Serial();
  init_ADC();
  init_insideLED();
  dht.begin();
}

int testcount=0;
void loop() {
  // put your main code here, to run repeatedly:
  temperature = (int8_t) read_temperature();
  humidity = (int8_t) read_humidity();
  if (temperature > 25) PORTB |= led_insdie_pin;
  else PORTB &= ~led_insdie_pin;  

  if(testcount++%2==0) water_refill=true;
  else water_refill=false;
  display_action(temperature, humidity, water_refill);
  serial_print(temperature, humidity);


  delay(1000); // 2초마다 업데이트
}

