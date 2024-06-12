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
#define LED_Output_PIN_R 2
#define LED_Output_PIN_G 0
#define LED_Output_PIN_Y 1

#define OC0A 0b01000000 //6번핀 사용 fan 회전을 위한 핀., PORTD의 6번 비트
#define OC2B 0b00001000 //3번핀 사용 피에조 부조를 위한 핀. ,PORTD의 3번 비트
#define relay_heater 0b10000000 //릴레이 히터 제어하는 핀.
#define relay_cooler 0b00000100 //릴레이 쿨러 제어하는 핀.
#define cooler_fan_1 0b00010000 //쿨러 팬 제어하는 핀1.
#define cooler_fan_2 0b00000010 //쿨러 팬 제어하는 핀2.


#define COOLER_ON 0
#define WATERPUMP_ON 1
#define HEATER_ON 2
#define WATER_REFILL 3


//-------------------variables------------------------------------------

LiquidCrystal_I2C lcd(0x27, 20, 4);   // 접근주소: 0x3F or 0x27
SoftwareSerial btSerial(12, 13); // RX: 12, TX: 13 번 핀
DHT dht(DHTPIN, DHTTYPE);

int16_t temperature, humidity, mean_water_level, temp_water_level;

int water_index = 0;

int16_t water_level[10];

bool water_refill;


int freq_count=0;


volatile int32_t time_count = 0;

const float frequencies[4][7] = {
  {392.00, 392.00, 440.00, 440.00, 392.00, 392.00, 329.63}, // 펠티어 쿨러 실행될 때, 솔 (G4) 솔 (G4) 라 (A4) 라 (A4) 솔 (G4) 솔 (G4) 미 (E4)
  {261.63, 261.63, 329.63, 329.63, 392.00, 392.00, 523.25}, // 펌프 실행될 때, 도 (C4) 도 (C4), 미 (E4) 미 (E4), 솔 (G4) 솔 (G4), 도 (C5)
  {392.00, 392.00, 349.23, 349.23, 329.63, 329.63, 293.66}, // 히터 실행될 때, 솔 (G4) 솔 (G4), 파 (F4) 파 (F4), 미 (E4) 미 (E4), 레 (D4)
  {440.00, 440.00, 493.88, 493.88, 523.25, 523.25, 587.33}  // 물부족일 때, 라 (A4) 라 (A4), 시 (B4) 시 (B4), 도 (C5) 도 (C5), 레 (D5)
};

uint8_t sound_state_flag[4]={0, 0, 0, 0};
// 0이면 아예 실행할 필요 없는 상황,
// 1이면 최초 진입 후 한 사이클 실행중,
// 2이면 이미 한번 실행 완료했으니 더이상 신경 안쓰기

// flag[0] : fan 실행됐을 때, flag[1] : ....
 
int user_mode=0; //처음에는 자동화모드로 설정
char c='a'; // bluetooth로 받은 값, 처음에는 자동화 모드 설정
volatile int16_t user_operation = 0;

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



void heater_setup(){
  DDRD|=relay_heater; //릴레이 제어하는 핀-> 세라믹 히터 on
}

void cooler_setup(){
  DDRC|=relay_cooler; //릴레이 제어하는 핀-> 펠티어 쿨러 on
  DDRD|=cooler_fan_1; //펠티어 쿨러에 달려있는 fan 1 on
  DDRC|=cooler_fan_2; //펠티어 쿨러에 달려있는 fan 2 on

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
  lcd.print(c == 'a' | 'b' ? "Auto" : "User");

  
  lcd.setCursor(0, 3);             // 네번째 줄 문자열 출력
  if(c == 'a'){ // 자동모드일 경우 물 보충 여부 출력
    lcd.print("Water Refill : ");
    if(water_refill) lcd.print("Yes");
    else  lcd.print("No");
  }
  else{ // 자동모드면 물 보충여부 출력  
    lcd.print("Operating : ");
    switch(c){
      case 'b':
        lcd.print("SELECTING");
        break;
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

  // Serial.print("waterlevel sensor value : ");
  // Serial.println(temp_water_level); 
  int16_t sum = 0;

  water_level[(water_index++) % 10] = temp_water_level;
  for(int i=0 ; i<10; i++){
    sum += water_level[i];
  }
  mean_water_level = sum / 10;
  // Serial.println(mean_water_level);
  if (mean_water_level < 550) { // 물 보충 필요할 때 : 모든 LED가 깜빡, water_refill 변수 true로
    sound(WATER_REFILL);
    if(time_count % 2 == 0){
      PORTB |= (1 << LED_Output_PIN_R) | (1 << LED_Output_PIN_Y) | (1 << LED_Output_PIN_G);
    }
    else{
      PORTB &= ~((1 << LED_Output_PIN_R) | (1 << LED_Output_PIN_Y) | (1 << LED_Output_PIN_G));
    }
    water_refill = true;
  }
  else if (mean_water_level>=550 && mean_water_level<580) { // 물 살짝 부족할 때 : 노란색 LED가 깜빡
    PORTB &= ~((1 << LED_Output_PIN_R) | (1 << LED_Output_PIN_Y) | (1 << LED_Output_PIN_G));
    if(time_count % 2 == 0){
      PORTB |= (1 << LED_Output_PIN_Y);
    }
    else{
      PORTB &= ~(1 << LED_Output_PIN_Y);
    }
    water_refill = false;
  }
  else { // 정상 : 초록 LED만 들어옴.
    PORTB &= ~((1 << LED_Output_PIN_R) | (1 << LED_Output_PIN_Y) | (1 << LED_Output_PIN_G));
    PORTB |= (1 << LED_Output_PIN_G);
    water_refill = false;
  }  
  // Serial.print(",  mean value : ");
  // // Serial.println(mean_water_level);
  // Serial.print("mean water_level : ");
  // Serial.println(mean_water_level);
}

void execute_waterpump(){
  // 펌프 실행
  pumptime_num = time_count % 15;
  // Serial.print("time_count : ");
  // Serial.println(time_count);
  // Serial.print("pumptime_num : ");
  // Serial.println(pumptime_num);
  // if (pumptime_num >= 10) PORTB |= (1 << Waterpump_Output_PIN_1);
  // else PORTB &= ~(1 << Waterpump_Output_PIN_1);
  PORTB |= (1 << Waterpump_Output_PIN_1);
}



void serial_print(int8_t temperature, int8_t humidity){
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, ");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.print("Current time : ");
  Serial.println(time_count);
  // delay(1000);
}

// 다른 멜로디들은 다 끄고, sound_select에 해당하는 멜로디만 재생, loop돌때마다 반복하지 않게 하는건 loop로 통제
void sound(int8_t sound_select){
  if (sound_select == -1){
    PORTD &= ~OC2B;  // OC2B (PORTD의 3번비트) 출력 끄기
    TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0));
    for(int i=0; i<4; i++){
      sound_state_flag[i] = 0;
    }
    freq_count = 0;
    return;
  }

  bool other_sound_check = false; // 서로 다른 두 멜로디가 동시에 출력을 요청할 때를 다루기 위한 변수
  for(int i=0; i< sound_select ; i++){ // 서로 다른 두 멜로디가 동시에 sound_state_flag에 접근하는 race condition 방지 위해 sound_select까지만 i를 증가 -> 무조건 index 작은 멜로디가 우선순위를 가지도록...
    if(i != sound_select){
      if(sound_state_flag[i] != 0){
        other_sound_check = true;
      }
    }
  }  

  if (other_sound_check == true) return; // 자신보다 앞선 index에서 출력 flag를 가진다면 자신은 exit

  for(int i=0; i<4; i++){
    if(i != sound_select){
      sound_state_flag[i] = 0;
    }
  }
  if(sound_state_flag[sound_select] != 2){
    sound_state_flag[sound_select] = 1; // 이미 완료된 상태가 아니라면 (첫 진입이라면) flag 1로 설정
  }
  if(sound_state_flag[sound_select] == 1){
    sound_play(sound_select); // 아직 완료된 상태가 아니라면 소리 재생
  }
  else if(sound_state_flag[sound_select] == 1){
    sound_play(-1);
  }
  return;
}


void sound_play(uint8_t sound_select){  // i번째 멜로디 출력
  static uint8_t i = sound_select;
  Serial.print("freq_count : ");
  Serial.println(freq_count);
  DDRD |= OC2B;  // OC2B (PORTD의 3번비트) output모드로
  if (freq_count >= 0 && freq_count < 6)
  {
    PORTD |= OC2B;  // OC2B (PORTD의 3번비트) 출력 주기
    TCCR2A|=(1<<COM2B1); // TIMER2 켜기
    float freq_target = frequencies[i][freq_count];
    OCR2A = F_CPU / 256 / freq_target - 1; // 목표 freq에 맞는 출력 나오게 OCR2A 설정
    OCR2B = OCR2A / 10; // 실제 출력할 것의 duty cycle 설정
    freq_count += 1;
  }
  else if (freq_count >= 6)
  {    // 출력 마무리작업 (초기상태로 되돌리기)
    PORTD &= ~OC2B;  // OC2B (PORTD의 3번비트) 출력 끄기
    // 타이머 2 를 비활성화 (TCCR2A의 COM2B 핀만 00 -> noraml port operation, OC0A disconnected)
    TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0));
    freq_count = 0;   // 음계 index 초기화
    sound_state_flag[sound_select] = 2; // 재생 완료했다고 flag 설정
  }
}

void fan_on(int8_t target){
  // Serial.println("inside fan_on()");
  PORTD |= OC0A;  // OC0A(PORTD의 6번비트)출력 주기
  TCCR0A |= (1<<COM0A1); // TIMER0 켜기
  OCR0A = target;
}

void fan_off(){
  TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));   // 타이머 0 를 비활성화 (TCCR0A의 COM0A 핀만 00 -> noraml port operation, OC0A disconnected)
  PORTD &= ~( 1 << OC0A);  // OC0A(PORTD의 6번비트) 출력 끄기
}

void heater_on(){
  PORTD |= relay_heater;    // 릴레이 히터 켜기
}

void heater_off(){
  PORTD &= ~relay_heater;    // 릴레이 히터 끄기
}

void cooler_on(){
  PORTC |= relay_cooler;    // 릴레이 쿨러 켜기
  PORTD |= cooler_fan_1;    // 쿨러에 있는 fan 1 켜기
  PORTC |= cooler_fan_2;    // 쿨러에 있는 fan 2 켜기

}

void cooler_off(){
  PORTC &= ~relay_cooler;    // 릴레이 쿨러 끄기
  PORTD &= ~cooler_fan_1;    // 쿨러에 있는 fan 1 끄기
  PORTC &= ~cooler_fan_2;    // 쿨러에 있는 fan 2 끄기

}

// char bluetooth_input(){
//   if (btSerial.available()){  //블루투스로 모드, 동작 제어 받기 위한 값 받아옴
//     char temp=btSerial.read();
//     if (temp == 'a' || temp == 'b' || temp == 'c' || temp == 'd' ||
//         temp == 'e' || temp == 'f' || temp == 'g' || temp == 'h' ||
//         temp == 'i' || temp == 'j')
//     return temp;
//   }  
// }

//-------------------------MAIN FUNCTION--------------------------------------

void setup() {
  // put your setup code here, to run once:
  init_Serial();
  btSerial.begin(9600);
  display_setup();
  init_ADC();
  timecount_setup();
  init_interrupt();
  init_insideLED();
  dht.begin();
  pump_setup();
  init_LED();
  heater_setup();
  cooler_setup();
  fan_setup();
  buzzer_setup();
}


void loop() {
  if (btSerial.available()){  //블루투스로 문자 받아옴
      char temp=btSerial.read();
      if (temp == 'a' || temp == 'b' || temp == 'c' || temp == 'd' ||
          temp == 'e' || temp == 'f' || temp == 'g' || temp == 'h' ||
          temp == 'i' || temp == 'j')
        c=temp;
  }
  // Serial.print("c: ");
  // Serial.println(c);


  // 온습도 읽기 및 LCD 출력
  temperature = read_temperature_digital();


  humidity = read_humidity();

  // Serial.println(temperature);
  display_action(temperature, humidity, water_refill, c);
  // serial_print(temperature,humidity);

  if(c=='a'){ // 자동모드
    // Serial.println("automode 1");
    user_mode=0; //사용자 모드 비활성화 시킴
    Serial.println("automode");

    waterlevel_check(); // 수위 체크
    execute_waterpump(); // 정해진 시간마다 워터펌프 실행

    if(temperature <= 19){  //18도 이하일때

      heater_on();      // 히터 켜기
      sound(HEATER_ON); // 다른 멜로디들은 다 끄고, sound_select에 해당하는 멜로디만 재생, loop돌때마다 반복하지 않게 하는건 loop로 통제
      fan_on(50);      //팬 약하게 틀기(OCR2A 50)

      cooler_off();
    }
    else if(temperature>19 & temperature<23){  // 정상 온도일때

      sound(-1); // 다른 멜로디들은 다 끄기
      heater_off(); // 히터 끄기
      fan_off();  // 팬 끄기
      cooler_off(); // 쿨러 끄기
      
  
    }
    else{  // 23도 이상일때

      cooler_on();
      fan_on(255);  //팬 켜기(OCR2A 200)
      sound(COOLER_ON); // 다른 멜로디들은 다 끄고, sound_select에 해당하는 멜로디만 재생, loop돌때마다 반복하지 않게 하는건 loop로 통제      

      heater_off(); // 히터 끄기
    }    
  }

  else //자동화모드 버튼 말고 다른 것이 눌렸을 때
  {   
    //사용자모드 버튼이 눌렸다면 user_mode가 1이 되게 하여, 사용자모드 활성화
      Serial.println("usermode");
 
    if (c=='b') user_mode=1;
      user_mode = true;
    Serial.println(user_mode);

    if (user_mode==1) //사용자모드로 선택되어 있다면, 환기 워터펌프 히팅 쿨링 관련 버튼 눌렀을 때 활성화.
    {
      heater_off();
      cooler_off();
      fan_off();
      Serial.println(c);
      switch (c)
      {
        case 'c': //환기팬 on
          Serial.println("useroperation : fan on");
          fan_on(200);
          break;
        
        case 'd': // 환기팬 off
          Serial.println("useroperation : fan off");
          fan_off();
          sound(-1); // 다른 멜로디들은 다   끄기
          break;
        case 'e': //워터펌프 on
          Serial.println("useroperation : pump on");
          PORTB |= (1 << Waterpump_Output_PIN_1);
          sound(WATERPUMP_ON);
          break;
        
        case 'f': //워터펌프 off
          Serial.println("useroperation : pump off");
          PORTB &= ~(1 << Waterpump_Output_PIN_1);
          sound(-1); // 다른 멜로디들은 다 끄기
          break;
        
        case 'g': // 히팅 on
          Serial.println("useroperation : heating on");
          heater_on();
          fan_on(50);
          sound(HEATER_ON);
          break;
        
        case 'h': //히팅 off
          Serial.println("useroperation : heating off");
          heater_off();
          fan_off();
          sound(-1); // 다른 멜로디들은 다 끄기
          break;
        
        case 'i': //쿨링 on
          Serial.println("useroperation : cooling off");
          cooler_on();
          fan_on(200);
          sound(COOLER_ON);
          break;
        
        case 'j': //쿨링 off
          Serial.println("useroperation : cooling off");
          cooler_off();
          fan_off();
          sound(-1); // 다른 멜로디들은 다 끄기
          break;
      }
    }
  }
  delay(500); // 1초마다 업데이트
}




