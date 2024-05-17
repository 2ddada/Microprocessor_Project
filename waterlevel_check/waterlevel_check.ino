/////// 수위 조절
//// 일정 수위 이하로 떨어지면 불 들어오도록
/// 일단 DDRB 핀 첫번째, 두번째, 세번째에 꽂는다고 가정하고
//// 수위는 400이하, 400~500, 500이상으로 된다고 짰음.

#define NUM_LEDS 3
#define ADC_RESOULUTION_IN_BITS 10    //<-- 필요없는건가?
#define PIN_of_LED_1 0
#define PIN_of_LED_2 1
#define PIN_of_LED_3 2

void init_LED(){
  //// LED 셋업시키기. DDRB = 0000 0111으로 되도록
  DDRB |= (1<<PIN_of_LED_1) | (1 << PIN_of_LED_2) | (1 << PIN_of_LED_3);
}

void init_Serial(){
  //// serial 통신 위해서 세팅
  Serial.begin(9600);
  delay(1000);
}

void init_ADC(){
  ADMUX |= (0 << REFS1) | (1 << REFS0);
  ADMUX |= (0 << ADLAR);
  ADMUX |= (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);  //// 이거나중에수정해야해수정해야해


  ADCSRA |= (1 << ADEN);
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

}


void setup() {
  init_Serial();
  init_ADC();
  init_LED();
  // put your setup code here, to run once:

}

void waterlevel_check(){
  
  // Start ADC

  ADCSRA |= (1 << ADSC);

  // Wait until ADC is completed
  while(ADCSRA & (1 << ADSC));

  uint16_t value = ADC;

  // 400이하라면 깜빡이는 기능까지 추가. 
  if(value < 400){
    PORTB = (1 << PIN_of_LED_1) | ( 1 << PIN_of_LED_2) | (1 << PIN_of_LED_3);
    delay(500);
    PORTB &= ~((1 << PIN_of_LED_1) | (1 << PIN_of_LED_2) | (1 << PIN_of_LED_3));
    delay(500);
  }

  else if(value > 400 && value < 500){
    PORTB = (1 << PIN_of_LED_1) | (1 << PIN_of_LED_2);
  }
  else {
    PORTB = (1 << PIN_of_LED_1) | (1 << PIN_of_LED_2) | (1 << PIN_of_LED_3);

  }

  Serial.println(value);
  delay(100);

}

void loop(){
  waterlevel_check();
  
}
