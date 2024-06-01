//-----------------MEMO---------------------


// LCD Display A4, A5 pin은 I2C 통신때문에 고정적으로 사용해야함!
// DHT-22 센서는 digital pin


//waterlevel 센서
// 일단 하나 새거 주문하긴 했는데.
// 기본적으로, num은 인터럽트가 생기는 주기(1초)마다 1씩 커지는 것으로 잡아 두었음. 

//


// fan은 timer 0
// 펌프는 timer 1
// 피에조 부저 2


//---------------------------header files---------------------

#include <Wire.h>                     // i2C 통신을 위한 라이브러리
#include <LiquidCrystal_I2C.h>        // LCD 2004 I2C용 라이브러리
#include "DHT.h"                     // DHT 센서 라이브러리



//-------------------define---------------------------------------------
#define DHTTYPE DHT22   // DHT22 (AM2302) 센서종류 설정


#define led_insdie_pin 0x20   // 내장 led pin번호
#define DHTPIN 2        // SDA 핀의 설정


#define Waterpump_Output_PIN_1 3 // 포트b의 3번째 핀
#define Waterpump_Output_PIN_2 5 // 포트d의 5번째 핀
#define Temperature_Input_PIN 1 // 온도 읽어올 핀
#define Waterlevel_Input_PIN 0 // 수위 읽어올 핀

#define NUM_LEDS 3
#define LED_Output_PIN_R 1
#define LED_Output_PIN_G 0
#define LED_Output_PIN_Y 2

#define OC0A 0b01000000 //6번핀 사용 쿨러 회전을 위한 핀.
#define OC2B 0b00001000 //3번핀 사용 피에조 부조를 위한 핀. ,PORTD의 3번 비트
#define Relay_controll 0b10000000 //릴레이 제어하는 핀.



//-------------------variables------------------------------------------

LiquidCrystal_I2C lcd(0x27, 20, 4);   // 접근주소: 0x3F or 0x27
DHT dht(DHTPIN, DHTTYPE);

int16_t temperature, humidity, water_level;
bool water_refill;


volatile int32_t time_count = 0;

const float frequencies[7] = { 
  392.00, // 솔 (G4)
  392.00, // 솔 (G4)
  440.00, // 라 (A4)
  440.00, // 라 (A4)
  392.00, // 솔 (G4)
  392.00, // 솔 (G4)
  329.63, // 미 (E4)
};

uint8_t sound_state_flag=0; // 0이면 아예 실행할 필요 없는 상황, 1이면 최초 진입 후 한 사이클 실행중, 2이면 이미 한번 실행 완료했으니 더이상 신경 안쓰기


//----------------------------------ISR-------------------------------------------------

ISR(TIMER1_COMPA_vect) {
  // 펌프 작동하는 주기 관리
  time_count++;
  // Serial.print("num check : ");
  // Serial.println(num);
  // Serial.print("Debugging : TIMER1_COMPA ISR(1초씩 세는거)) execution times : ");
  // Serial.println(num);
  // 5초동안 펌프를 켜기. 작동 시간 바꾸고 싶으면 아래 숫자만 바꿔주면 끝.
  
  // if (num > 10 && num <= 15) {
  //   PORTB &= ~(1 << Waterpump_Output_PIN_1);
  //   if (num == 15) {
  //     num = 0;
  //   }
  // } else {
  //   PORTB |= (1 << Waterpump_Output_PIN_1);
  // }

  // Serial.print("waterpump check : ");
  // Serial.println(PORTB & (1 << Waterpump_Output_PIN_1));
}

ISR(TIMER0_COMPA_vect)
{ // 버저에서 나는 소리 조절
  // static int debuggingnum = 0;
  // debuggingnum++;

  // if (debuggingnum%100 == 0){
  //   Serial.print("Debugging : TIMER0_COMPA ISR (팬 쓰는거) execution times : ");
  //    Serial.println(debuggingnum);
  // }

  // if (flag == 1 && freq_count >= 0 && freq_count < 7)
  // {
  //   freq_count+=1;
  
  //   float freq_target=frequencies[freq_count];
  
  //   OCR2A=F_CPU/256/freq_target-1;
  
  //   OCR2B=OCR2A/10000;

  //   Serial.println("Debugging : nop executed ");

  //   for (uint16_t j=0;j<50;j++)
  //   {
  //     for (uint16_t i=0;i<64000;i++)
  //     {
  //       asm("nop");
  //     }
  //   }
  // }
  // else // 마지막 소리가 종료되면
  // {
  //   flag = 0; // 플래그 설정하여 더 이상 ISR이 소리를 재생하지 않도록 함
  //   freq_count = -1;
  //   TIMSK2 &= ~(1<<OCIE2A);
  // }
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
  lcd.print("Hello World!!");

  lcd.setCursor(0, 1);             // 두번째 줄 문자열 출력
  lcd.print("Welcome to our SmartFarm!");
 
  lcd.setCursor(0, 2);             // 세번째 줄 문자열 출력
  lcd.print("Now booting.......");
  
  lcd.setCursor(0, 3);             // 네번째 줄 문자열 출력
  lcd.print("Now booting.......");
}

void init_insideLED(){
  //내장led 키려고 하는거니까 PortB의 5번 비트 출력으로 설정
  // DDRB = 0b 0010 0000
  DDRB |= (1 << led_insdie_pin);
}

void init_interrupt(){
  SREG |= 0x01 << SREG_I;
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
}

void timecount_setup(){
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
  DDRB |= (1 << LED_Output_PIN_R) | (1 << LED_Output_PIN_Y) | (1 << LED_Output_PIN_G);
}



void relay_setup(){
  DDRB|=Relay_controll; //릴레이 제어하는 핀-> 세라믹 히터 on//off

}

void fan_setup(){//팬은 Counter 0 사용

  //타이머 카운터 작동 모드 Fast PWM으로 설정.
  TCCR0A|=(1<<WGM01)|(1<<WGM00);
  TCCR0B|=(0<<WGM02);
  //타이머 카운터가 동작하기 위해 사용되는 클럭 선택
  TCCR0B|=(0<<CS02)|(1<<CS01)|(1<<CS00);

  //OCOA의 출력모드 설정
  TCCR0A|=(1<<COM0A1)|(0<<COM0A0);
  //인터럽트 활성화
  // TIMSK0|=(1<<OCIE0A);

  //팬의 dutycyle 조절
  OCR0A=200;
}

void buzzer_setup(){
  TCCR2A|=(1<<WGM21)|(1<<WGM20);
  TCCR2B|=(1<<WGM22);
  TCCR2B|=(1<<CS22)|(1<<CS21)|(0<<CS20);

  TCCR2A|=(1<<COM2B1);

  // float freq_target=frequencies[freq_count];

  // OCR2A=F_CPU/256/freq_target-1;
  // OCR2B=OCR2A/2;
}
//-----------------------action------------------------------------------------

void display_action(int8_t temperature, int8_t humidity, bool water_refill)
{
  lcd.clear();

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


int16_t read_ADC(uint8_t pin) {
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
int16_t read_temperature(){
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

int16_t read_humidity(){
  return (int16_t) dht.readHumidity();
}

int16_t read_temperature_digital(){
  return (int16_t) dht.readTemperature();
}


int pumptime_num;


void waterlevel_check() {

  // ADCSRA |= (1 << ADSC);
  // while (ADCSRA & (1 << ADSC));
  // uint16_t value = ADC;

  water_level = read_ADC(Waterlevel_Input_PIN);
  // // water_level = 300;
  // Serial.print("water_level : ");
  // Serial.println(water_level);

  // 펌프 실행
  pumptime_num = time_count % 15;
  Serial.print("time_count : ");
  Serial.println(time_count);
  // Serial.print("pumptime_num : ");
  // Serial.println(pumptime_num);
  if (pumptime_num >= 10) PORTB &= ~(1 << Waterpump_Output_PIN_1);
  else PORTB |= (1 << Waterpump_Output_PIN_1);

  if (water_level < 400) { // 물 보충 필요할 때 : 빨간색 LED가 깜빡, water_refill 변수 true로
    if(time_count % 2 == 0){
      PORTB |= (1 << LED_Output_PIN_R) | (1 << LED_Output_PIN_Y) | (1 << LED_Output_PIN_G);
    }else{
      PORTB &= ~((1 << LED_Output_PIN_R) | (1 << LED_Output_PIN_Y) | (1 << LED_Output_PIN_G));
    }

    water_refill = true;

    // PORTB |= (1 << LED_Output_PIN_R) | (1 << LED_Output_PIN_Y) | (1 << LED_Output_PIN_G);
    // delay(500);
    // PORTB &= ~((1 << LED_Output_PIN_R) | (1 << LED_Output_PIN_Y) | (1 << LED_Output_PIN_G));
    // delay(500);

    // else if 가 안돌아가는데 왜냐면 조금이라도 닿기만 하면 바로 1023이 나와서 그럼.....하 ...
    // 그럼 1023일때만 세개 다 켜지고 애매한 값일때는 두개만 켜지도록 해보자
  }
  else if (water_level>=400 && water_level<600) { // 물 살짝 부족할 때 : 노란색 LED가 깜빡
    PORTB &= ~((1 << LED_Output_PIN_R) | (1 << LED_Output_PIN_Y) | (1 << LED_Output_PIN_G));
    // Serial.print("e");
    if(time_count % 2 == 0){
      PORTB |= (1 << LED_Output_PIN_Y);
    }else{
      PORTB &= ~(1 << LED_Output_PIN_Y);
    }
  }
  else { // 정상 : 초록 LED만 들어옴.
    PORTB &= ~((1 << LED_Output_PIN_R) | (1 << LED_Output_PIN_Y) | (1 << LED_Output_PIN_G));
    PORTB |= (1 << LED_Output_PIN_G);
  }

  // Serial.print("waterlevel sensor value : ");
  // Serial.println(water_level); 
}

// void execute_waterlevel() {
//   static unsigned long lastMillis = 0;
//   if (millis() - lastMillis >= 1000) {
//     lastMillis = millis();
//     waterlevel_check();
//   }

//   //Serial.print("num  : ");
//   //Serial.println(num);
// }


void serial_print(int8_t temperature, int8_t humidity){
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, ");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  // delay(1000);
}

// void sound_play(){
//   static int freq_count=0;
//   DDRD |= OC2B;  // OC2B (PORTD의 3번비트) output모드로

//   if (freq_count >= 0 && freq_count < 7)
//   {
//     PORTD |= OC2B;  // OC2B (PORTD의 3번비트) 출력 주기


//     float freq_target=frequencies[freq_count];

//     OCR2A=F_CPU/256/freq_target-1; // 목표 freq에 맞는 출력 나오게 OCR2A 설정

//     OCR2B=OCR2A/100; // 실제 출력할 것의 dutycyle 설정

//     Serial.println("Debugging : nop executed ");

//     for (uint16_t j=0;j<50;j++)
//     {
//       // for (uint16_t i=0;i<64000;i++)
//       for (uint16_t i=0;i<32000;i++)
//       {
//         asm("nop");
//       }
//     }
//     freq_count+=1;
//   }
//   if(freq_count >= 7){    // 출력 마무리작업 (초기상태로 되돌리기)
  
//     PORTD&=~OC2B;  // OC2B (PORTD의 3번비트) 출력 끄기
//     freq_count = 0;   // 음계 index 초기화
//     sound_state_flag = 2; // 재생 완료했다고 flag 설정
//   }
// }
void sound_play(){
  static int freq_count=0;
  DDRD |= OC2B;  // OC2B (PORTD의 3번비트) output모드로

  if (freq_count >= 0 && freq_count < 7)
  {
    PORTD |= OC2B;  // OC2B (PORTD의 3번비트) 출력 주기
    TCCR2A|=(1<<COM2B1); // TIMER2 켜기
    float freq_target = frequencies[freq_count];

    OCR2A = F_CPU / 256 / freq_target - 1; // 목표 freq에 맞는 출력 나오게 OCR2A 설정
    OCR2B = OCR2A / 100; // 실제 출력할 것의 duty cycle 설정

    // Serial.println("Debugging : nop executed ");

    for (uint16_t j = 0; j < 50; j++)
    {
      for (uint16_t i = 0; i < 64000; i++)
      {
        asm("nop");
      }
    }
    freq_count += 1;
  }
  else if (freq_count >= 7)
  {    // 출력 마무리작업 (초기상태로 되돌리기)
    PORTD &= ~OC2B;  // OC2B (PORTD의 3번비트) 출력 끄기

    // 타이머 2 를 비활성화 (TCCR2A의 COM2B 핀만 00 -> noraml port operation, OC0A disconnected)
    TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0));
    // TCCR2B &= ~((1 << CS22) | (1 << CS21) | (0 << CS20));

    freq_count = 0;   // 음계 index 초기화
    sound_state_flag = 2; // 재생 완료했다고 flag 설정
  }
}



//-------------------------MAIN FUNCTION--------------------------------------

void setup() {
  // put your setup code here, to run once:
  init_Serial();
  display_setup();
  init_ADC();
  timecount_setup();
  init_interrupt();
  init_insideLED();
  dht.begin();
  pump_setup();
  init_LED();
  relay_setup();
  fan_setup();
  buzzer_setup();
}


void loop() {

  // temperature = read_temperature_digital();
  // temperature = 25;
  if(time_count <15 ) temperature = 25;
  if(time_count >15 & time_count < 20 ) temperature = 20;
  if(time_count >20 ) temperature = 25;
  
  humidity = read_humidity();

  display_action(temperature, humidity, water_refill);
  serial_print(temperature, humidity);  

  // execute_waterlevel();
  waterlevel_check();

  if(temperature <= 18){  //18도 이하일때
    // Serial.println("temp under 18 entered");
    sound_state_flag = 0;
    PORTB|=Relay_controll;    //릴레이 on->세라믹 히터 on
    OCR0A = 0;
    // TCCR0A &= ~ ( (1 << COM0A1) | (1 << COM0A0) );   // 팬 끄기
    // PORTD &= ~(1<<OC0A);    //팬 끄기

  }
  else if(temperature>18 & temperature<23){  // 정상 온도일때
    sound_state_flag = 0;
    PORTB &= ~Relay_controll;    // 릴레이 끄기 (히터 끄기)
        Serial.print("Flag : ");

    Serial.println(sound_state_flag);

    PORTD &= ~(1<<OC0A);    //팬 끄기
    PORTD &= ~OC2B;  // OC2B (PORTD의 3번비트) 출력 끄기
    // 타이머 2 를 비활성화 (TCCR2A의 COM2B 핀만 00 -> noraml port operation, OC0A disconnected)
    TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0));
  }
  else{  // 23도 이상일때
    PORTB &= ~Relay_controll;    // 릴레이 끄기 (히터 끄기)
    PORTD |= (1<<OC0A);    //팬 켜기
    if(sound_state_flag != 2) sound_state_flag = 1; // 이미 완료된 상태가 아니라면 (첫 진입이라면) flag 1로 설정
    Serial.print("Flag : ");
    Serial.println(sound_state_flag);
    if(sound_state_flag == 1) sound_play(); // 아직 완료된 상태가 아니라면 소리 재생
  }
  delay(1000); // 1초마다 업데이트
}




