/////// 수위 조절
// 일정 수위 이하로 떨어지면 불 들어오도록
/// 일단 3번 핀에 LED를 연결한다고 치자
//// 그럼 짜피 const uint16_t step = pow( ~~~~ ) 얘들은 필요없는거?

#define NUM_LEDS 1
#define PIN_of_LED 3

void init_LED(){
  //// LED 셋업시키기. DDRB = 0000 0100으로 되도록
  DDRB |= (1<<PIN_of_LED);
}

void init_Serial(){
  //// serial 통신 위해서 세팅
  Serial.begin(9600);
  delay(1000);
}

void init_ADC(){
  ADMUX |= (0 << REFS1) | (1 << REFS0);
  ADMUX |= (0 << ADLAR);
  ADMUX |= (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);


  ADCSRA |= (1 << ADEN);
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

}


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
