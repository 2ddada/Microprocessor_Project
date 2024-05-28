//-----------------MEMO---------------------


// LCD Display A4, A5 pin은 I2C 통신때문에 고정적으로 사용해야함!
// DHT-22 센서는 digital pin


//waterlevel 센서
// 일단 하나 새거 주문하긴 했는데.
// 기본적으로, num은 인터럽트가 생기는 주기(1초)마다 1씩 커지는 것으로 잡아 두었음. 


//---------------------------header files---------------------

#include <Wire.h>                     // i2C 통신을 위한 라이브러리
#include <LiquidCrystal_I2C.h>        // LCD 2004 I2C용 라이브러리
#include "DHT.h"                       // DHT 센서 라이브러리



//-------------------define---------------------------------------------
#define DHTTYPE DHT22   // DHT22 (AM2302) 센서종류 설정


#define led_insdie_pin 0x20   // 내장 led pin번호
#define DHTPIN 2        // SDA 핀의 설정


#define Waterpump_Output_PIN_1 3 // 포트b의 3번째 핀
#define Waterpump_Output_PIN_2 6 // 포트d의 6번째 핀
#define Temperature_Input_PIN 0 // 온도 읽어올 핀
#define Waterlevel_Input_PIN 2 // 수위 읽어올 핀

#define NUM_LEDS 3
#define LED_Output_PIN_1 0
#define LED_Output_PIN_2 1
#define LED_Output_PIN_3 2



//-------------------variables------------------------------------------

LiquidCrystal_I2C lcd(0x27, 20, 4);   // 접근주소: 0x3F or 0x27
DHT dht(DHTPIN, DHTTYPE);

int8_t temperature, humidity;
bool water_refill;


volatile int num = 0;


//----------------------------------ISR-------------------------------------------------

ISR(TIMER1_COMPA_vect) {
  num++;
  Serial.print("num check : ");
  Serial.println(num);
  // 5초동안 펌프를 켜기. 작동 시간 바꾸고 싶으면 아래 숫자만 바꿔주면 끝.
  if (num > 10 && num <= 15) {
    PORTB &= ~(1 << Waterpump_Output_PIN_1);
    if (num == 15) {
      num = 0;
    }
  } else {
    PORTB |= (1 << Waterpump_Output_PIN_1);
  }
  Serial.print("waterpump check : ");
  Serial.println(PORTB & (1 << Waterpump_Output_PIN_1));

}
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


void pump_setup() {
  DDRB |= (1 << Waterpump_Output_PIN_1);
  DDRD |= (1 << Waterpump_Output_PIN_2);
  PORTB &= ~(1 << Waterpump_Output_PIN_1); // 처음에는 꺼줌
  PORTD &= ~(1 << Waterpump_Output_PIN_2);

  cli();

  // 타이머 1 설정 - CTC 모드, 프리스케일러 1024
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC 모드, 프리스케일러 1024
  OCR1A = 15624; // 1초마다 인터럽트 발생
  TIMSK1 |= (1 << OCIE1A); // 타이머 1 비교 매치 인터럽트 활성화

  sei();
}

void init_LED() {
  DDRB |= (1 << LED_Output_PIN_1) | (1 << LED_Output_PIN_2) | (1 << LED_Output_PIN_3);
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


int read_ADC(uint8_t pin) {
  // ADC 채널 설정
  ADMUX = (ADMUX & 0xF0) | (pin & 0x0F);
  // ADC 변환 시작
  ADCSRA |= (1 << ADSC);
  // 변환 완료 대기
  while (ADCSRA & (1 << ADSC));
  // 결과 반환
  return ADC;
}


// 온도체크
// 일단은 interrupt 안하고 간단히 busywait으로 구현
// busywait함수를 loop함수에 직접적으로 넣어야하나 아니면 개별 함수마다 넣어야하나?
// 나중에 interrupt 구현하면 상관없을거같긴 한데 일단은 개별 함수마다 구현해야겠다
int8_t read_temperature(){
  // // 뭐 지금 당장은 ADC0으로 들어오게 한다 해야겠다
  // // !!!!!!!!!!!!!!!!!!통합할때 수정 필요!!!!!!!!!!!!!!!!!!!
  // ADMUX |= (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);

  // ADCSRA |= (1 << ADSC); // ADC 변환 시작

  // // CPU가 ADSC 비트 1로 바꿔줄때까지 busywait 
  // while (ADCSRA & (1 << ADSC));
  // // 8비트 레지스터 두개 합쳐서 16비트의 값으로 바꾸고 temp_temperature 변수에 저장 (지금은 전압값, 0~1023사이의 값)
  // uint16_t temp_temperature = (uint16_t)ADCL | ((uint16_t)(ADCH) << 8);

  uint16_t temp_temperature = read_ADC(Temperature_Input_PIN);


  // 이다음에 실제 온도값으로 변경
  // 계산과정에서 정수말고 실수로 될 수 도 있으니 float하고
  // datasheet 분석하면 T = 100Vout

  temperature = (float)temp_temperature * 5.0 / 1024.0 * 100.0;
  return temperature;
}

int8_t read_humidity(){
  return (int8_t) dht.readHumidity();
}

int8_t read_temperature_digital(){
  return (int8_t) dht.readTemperature();
}


void waterlevel_check() {

  // ADCSRA |= (1 << ADSC);
  // while (ADCSRA & (1 << ADSC));
  // uint16_t value = ADC;

  uint16_t value = read_ADC(Waterlevel_Input_PIN);

  if (value < 400) {
    PORTB |= (1 << LED_Output_PIN_1) | (1 << LED_Output_PIN_2) | (1 << LED_Output_PIN_3);
    delay(500);
    PORTB &= ~((1 << LED_Output_PIN_1) | (1 << LED_Output_PIN_2) | (1 << LED_Output_PIN_3));
    delay(500);

    // else if 가 안돌아가는데 왜냐면 조금이라도 닿기만 하면 바로 1023이 나와서 그럼.....하 ...
    // 그럼 1023일때만 세개 다 켜지고 애매한 값일때는 두개만 켜지도록 해보자
  } else if (value == 1023) {
    PORTB |= (1<< LED_Output_PIN_1) | (1 << LED_Output_PIN_2) | (1 << LED_Output_PIN_3);
  } else {
    PORTB &= ~(1 << LED_Output_PIN_3);  // 세번째 led는 끄고 2개만 키도록
    PORTB |= (1 << LED_Output_PIN_1) | (1 << LED_Output_PIN_2);
  }

  Serial.print("waterlevel sensor value : ");
  Serial.println(value);
  
}

void execute_waterlevel() {
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();
    waterlevel_check();
  }

  //Serial.print("num  : ");
  //Serial.println(num);
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
  pump_setup();
  init_LED();
}

int testcount=0;
void loop() {
  // put your main code here, to run repeatedly:


  temperature = (int8_t) read_temperature();
  humidity = (int8_t) read_humidity();
  display_action(temperature, humidity, water_refill);
  serial_print(temperature, humidity);  

  execute_waterlevel();


  if(testcount++%2==0) water_refill=true;
  else water_refill=false;


  delay(1000); // 2초마다 업데이트
}

