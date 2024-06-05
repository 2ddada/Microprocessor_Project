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
#include <SoftwareSerial.h> // 블루투스 통
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

#define OC0A 0b01000000 //6번핀 사용 쿨러 회전을 위한 핀., PORTD의 6번 비트
#define OC2B 0b00001000 //3번핀 사용 피에조 부조를 위한 핀. ,PORTD의 3번 비트
#define Relay_controll 0b10000000 //릴레이 제어하는 핀.



//-------------------variables------------------------------------------

LiquidCrystal_I2C lcd(0x27, 20, 4);   // 접근주소: 0x3F or 0x27
SoftwareSerial btSerial(12, 13); // RX: 12, TX: 13 번 핀
DHT dht(DHTPIN, DHTTYPE);

int16_t temperature, humidity, mean_water_level, temp_water_level;

int water_index = 0;

int16_t water_level[10];

bool water_refill;


bool usermode = false;
volatile int16_t user_operation = 0;


volatile int32_t time_count = 0;

const float frequencies[4][7] = {
  {392.00, 392.00, 440.00, 440.00, 392.00, 392.00, 329.63}, // fan 킬 때, 솔 (G4) 솔 (G4) 라 (A4) 라 (A4) 솔 (G4) 솔 (G4) 미 (E4)
  {0},
  {0},
  {0}
};

int8_t sound_select;
uint8_t sound_state_flag[4]={0}; // 0이면 아예 실행할 필요 없는 상황, 1이면 최초 진입 후 한 사이클 실행중, 2이면 이미 한번 실행 완료했으니 더이상 신경 안쓰기
// flag[0] : fan 실행됐을 때, flag[1] : ....
 
int user_mode=0; //처음에는 자동화모드로 설정
char c='a'; //처음에는 자동화 모드 설정


//----------------------------------ISR-------------------------------------------------

ISR(TIMER1_COMPA_vect) {
  // 1초에 1씩 count
  time_count++;
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

  DDRD |= OC0A;

  //타이머 카운터 작동 모드 Fast PWM으로 설정.
  TCCR0A|=(1<<WGM01)|(1<<WGM00);
  TCCR0B|=(0<<WGM02);
  //타이머 카운터가 동작하기 위해 사용되는 클럭 선택
  TCCR0B|=(0<<CS02)|(1<<CS01)|(1<<CS00);

  //OCOA의 출력모드 설정
  // TCCR0A|=(1<<COM0A1)|(0<<COM0A0);
  TCCR0A &= ~((1<<COM0A1)|(1<<COM0A0));


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

void display_action(int8_t temperature, int8_t humidity, bool water_refill, char c)
{
  lcd.clear();

  lcd.setCursor(0, 0);             // 첫번째 줄 문자열 출력
  lcd.print("Temperature : ");
  lcd.print(temperature);
  lcd.print("oC");

 
  lcd.setCursor(0, 1);             // 두번째 줄 문자열 출력
  lcd.print("Humidity : ");
  lcd.print(humidity);
  lcd.print("%");
  
 
  lcd.setCursor(0, 2);             // 세번째 줄 문자열 출력
  lcd.print("Current mode : ");
  lcd.print(usermode ? "Usermode" : "Automode");

  
  lcd.setCursor(0, 3);             // 네번째 줄 문자열 출력
  if(c == 'a'){ // 자동모드일 경우 물 보충 여부 출력
    lcd.print("Water Refill : ");
    lcd.print(water_refill ? "Yes" : "No");
  }
  else{ // 자동모드면 물 보충여부 출력  
    lcd.print("Operating : ");
    switch(c){
      case 'c':
        lcd.print("Fan ON");
        break;
      case 'd':
        lcd.print("Fan OFF");
        break;
      case 'e':
        lcd.print("Waterpump ON");
        break;
      case 'f':
        lcd.print("Waterpump OFF");
        break;
      case 'g':
        lcd.print("Heating ON");
        break;
      case 'h':
        lcd.print("Heating OFF");
        break;
      case 'i':
        lcd.print("Cooling ON");
        break;
      case 'j':
        lcd.print("Cooling OFF");
        break;
      default: 
        lcd.print("invalid operation");
        break;
    }
  }
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

  temp_water_level = read_ADC(Waterlevel_Input_PIN);

  Serial.print("waterlevel sensor value : ");
  Serial.print(temp_water_level); 
  int16_t sum = 0;

  water_level[(water_index++) % 10] = temp_water_level;
  for(int i=0 ; i<10; i++){
    sum += water_level[i];
  }
  mean_water_level = sum / 10;

  Serial.print(",  mean value : ");
  Serial.println(mean_water_level);
  // // water_level = 300;
  // Serial.print("mean water_level : ");
  // Serial.println(meaa water_level);
}

void execute_waterpump(){
  // 펌프 실행
  pumptime_num = time_count % 15;
  // Serial.print("time_count : ");
  // Serial.println(time_count);
  // Serial.print("pumptime_num : ");
  // Serial.println(pumptime_num);
  if (pumptime_num >= 10) PORTB &= ~(1 << Waterpump_Output_PIN_1);
  else PORTB |= (1 << Waterpump_Output_PIN_1);

  if (mean_water_level < 400) { // 물 보충 필요할 때 : 빨간색 LED가 깜빡, water_refill 변수 true로
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
  else if (mean_water_level>=400 && mean_water_level<600) { // 물 살짝 부족할 때 : 노란색 LED가 깜빡
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

}



void serial_print(int8_t temperature, int8_t humidity){
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, ");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  // delay(1000);
}

// 다른 멜로디들은 다 끄고, sound_select에 해당하는 멜로디만 재생, loop돌때마다 반복하지 않게 하는건 loop로 통제
void sound(int8_t sound_select){
  for(int i=0; i<4; i++){
    if(i != sound_select){
      sound_state_flag[i] = 0;
    }
  }
  if (sound_select == -1) return;
  if(sound_state_flag[sound_select] != 2) sound_state_flag[sound_select] = 1; // 이미 완료된 상태가 아니라면 (첫 진입이라면) flag 1로 설정
  if(sound_state_flag[0] == 1) sound_play(sound_select); // 아직 완료된 상태가 아니라면 소리 재생
  return;
}


void sound_play(uint8_t sound_slect){  // i번째 멜로디 출력
  static int freq_count=0;
  static uint8_t i = sound_select;

  DDRD |= OC2B;  // OC2B (PORTD의 3번비트) output모드로

  if (freq_count >= 0 && freq_count < 7)
  {
    PORTD |= OC2B;  // OC2B (PORTD의 3번비트) 출력 주기
    TCCR2A|=(1<<COM2B1); // TIMER2 켜기
    float freq_target = frequencies[i][freq_count];

    OCR2A = F_CPU / 256 / freq_target - 1; // 목표 freq에 맞는 출력 나오게 OCR2A 설정
    OCR2B = OCR2A / 100; // 실제 출력할 것의 duty cycle 설정

    // Serial.println("Debugging : nop executed ");

    // for (uint16_t j = 0; j < 50; j++)
    // {
    //   for (uint16_t i = 0; i < 32000; i++)
    //   {
    //     asm("nop");
    //   }
    // }
    freq_count += 1;
  }
  else if (freq_count >= 7)
  {    // 출력 마무리작업 (초기상태로 되돌리기)
    PORTD &= ~OC2B;  // OC2B (PORTD의 3번비트) 출력 끄기
    // 타이머 2 를 비활성화 (TCCR2A의 COM2B 핀만 00 -> noraml port operation, OC0A disconnected)
    TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0));

    freq_count = 0;   // 음계 index 초기화
    sound_state_flag[i] = 2; // 재생 완료했다고 flag 설정
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
  //블루투스로 문자 받아옴
  if (btSerial.available()) c=btSerial.read();
  //자동화모드에서 다른 문자가 선택되면 무시하기 위한 코드. 받아온 문자를 다시 a로 바꿔줌.
  if (user_mode==0 & c!='a') c='a';

  temperature = read_temperature_digital();  
  humidity = read_humidity();
  display_action(temperature, humidity, water_refill, c);

  if(c=='a'){ // 자동모드
    // serial_print(temperature, humidity);  
    user_mode=0; //사용자 모드 비활성화 시킴

    waterlevel_check();
    execute_waterpump();

    if(temperature <= 18){  //18도 이하일때
      // Serial.println("temp under 18 entered");
      sound_select = 0; // fan 실행시 발생하는 멜로디 : 0번 멜로디
      sound(sound_select); // 다른 멜로디들은 다 끄고, sound_select에 해당하는 멜로디만 재생, loop돌때마다 반복하지 않게 하는건 loop로 통제
      
      // 팬 끄기
      TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));   // 타이머 0 를 비활성화 (TCCR0A의 COM0A 핀만 00 -> noraml port operation, OC0A disconnected)
      PORTD &= ~OC0A;  // OC0A(PORTD의 6번비트) 출력 끄기
    }
    else if(temperature>18 & temperature<23){  // 정상 온도일때
      sound_select = -1; // fan 실행시 발생하는 멜로디 : 0번 멜로디
      sound(sound_select); // 다른 멜로디들은 다 끄고, sound_select에 해당하는 멜로디만 재생, loop돌때마다 반복하지 않게 하는건 loop로 통제

      Serial.println("inside if 18 23");

      // 팬 끄기
      TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));   // 타이머 0 를 비활성화 (TCCR0A의 COM0A 핀만 00 -> noraml port operation, OC0A disconnected)
      PORTD &= ~OC0A;  // OC0A(PORTD의 6번비트) 출력 끄기
  
    }
    else{  // 23도 이상일때
      PORTB &= ~Relay_controll;    // 릴레이 끄기 (히터 끄기)

      //팬 켜기
      PORTD |= OC0A;  // OC0A(PORTD의 6번비트)출력 주기
      TCCR0A|=(1<<COM0A1); // TIMER0 켜기

      sound_select = 0; // fan 실행시 발생하는 멜로디 : 0번 멜로디
      sound(sound_select); // 다른 멜로디들은 다 끄고, sound_select에 해당하는 멜로디만 재생, loop돌때마다 반복하지 않게 하는건 loop로 통제

      // PORTD &= ~OC2B;  // OC2B (PORTD의 3번비트) 출력 끄기
      // // 타이머 2 를 비활성화 (TCCR2A의 COM2B 핀만 00 -> noraml port operation, OC0A disconnected)
      // TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0));
      
    }    
  }
    else //자동화모드 버튼 말고 다른 것이 눌렸을 때
  {   
    //사용자모드 버튼이 눌렸다면 user_mode가 1이 되게 하여, 사용자모드 활성화
    if (c=='b') user_mode=1;
    if (user_mode==1) //사용자모드로 선택되어 있다면, 환기 워터펌프 히팅 쿨링 관련 버튼 눌렀을 때 활성화.
    {
    switch (c)
    {
      case 'c':
        //환기팬 on하는 코드 입력
        break;
      
      case 'd':
        //환기팬 off하는 코드 입력
        break;
      
      case 'e':
        //워터펌프 on하는 코드 입력
        break;
      
      case 'f':
        //워터펌프 off하는 코드 입력
        break;
      
      case 'g':
        //히팅 on하는 코드 입력
        break;
      
      case 'h':
        //히팅 off하는 코드 입력
        break;
      
      case 'i':
        //쿨링 on하는 코드 입력
        break;
      
      case 'j':
        //쿨링 off하는 코드 입력
        break;
      }
    }
  }
  delay(1000); // 1초마다 업데이트
}




