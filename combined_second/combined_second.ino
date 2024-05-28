//// waterlevel과 pump를 합친건데... 비상
// 그냥 끝에만 닿아도 최대값 1023이 뜨는 상황...
// 일단 하나 새거 주문하긴 했는데.
// 기본적으로, num은 인터럽트가 생기는 주기(1초)마다 1씩 커지는 것으로 잡아 두었음. 
#define PIN_1 3 // 포트b의 3번째 핀
#define PIN_2 6 // 포트d의 6번째 핀

#define NUM_LEDS 3
#define PIN_of_LED_1 0
#define PIN_of_LED_2 1
#define PIN_of_LED_3 2

volatile int num = 0;

void pump_setup() {
  DDRB |= (1 << PIN_1);
  DDRD |= (1 << PIN_2);
  PORTB &= ~(1 << PIN_1); // 처음에는 꺼줌
  PORTD &= ~(1 << PIN_2);

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
  DDRB |= (1 << PIN_of_LED_1) | (1 << PIN_of_LED_2) | (1 << PIN_of_LED_3);
}

void init_Serial() {
  Serial.begin(9600);
  delay(1000);
}

void init_ADC() {
  ADMUX |= (1 << REFS0);
  ADMUX |= (0 << ADLAR);
  ADMUX |= (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0); 

  ADCSRA |= (1 << ADEN);
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void waterlevel_check() {
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));

  uint16_t value = ADC;

  if (value < 400) {
    PORTB |= (1 << PIN_of_LED_1) | (1 << PIN_of_LED_2) | (1 << PIN_of_LED_3);
    delay(500);
    PORTB &= ~((1 << PIN_of_LED_1) | (1 << PIN_of_LED_2) | (1 << PIN_of_LED_3));
    delay(500);

    // else if 가 안돌아가는데 왜냐면 조금이라도 닿기만 하면 바로 1023이 나와서 그럼.....하 ...
    // 그럼 1023일때만 세개 다 켜지고 애매한 값일때는 두개만 켜지도록 해보자
  } else if (value == 1023) {
    PORTB |= (1<< PIN_of_LED_1) | (1 << PIN_of_LED_2) | (1 << PIN_of_LED_3);
  } else {
    PORTB &= ~(1 << PIN_of_LED_3);  // 세번째 led는 끄고 2개만 키도록
    PORTB |= (1 << PIN_of_LED_1) | (1 << PIN_of_LED_2);
  }

  Serial.println(value);
  
}

void setup() {
  pump_setup();
  init_Serial();
  init_ADC();
  init_LED();
}

// 루프는 도는데, 워터레벨체크 함수를 1초마다 시행되게 하고 싶음. 
// millis로 시행된지 얼마나 지났는지 읽어오고, 그게 1초보다 크면 waterlevel_check로 adc실행되도록
// 
void loop() {
  static unsigned long lastMillis = 0;

  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();
    waterlevel_check();
  }

  //Serial.print("num  : ");
  //Serial.println(num);
  //delay(1000); // 인터럽트와 동기화
}

ISR(TIMER1_COMPA_vect) {
  num++;

  // 5초동안 펌프를 켜기. 작동 시간 바꾸고 싶으면 아래 숫자만 바꿔주면 끝.
  if (num > 10 && num <= 15) {
    PORTB |= (1 << PIN_1);
    if (num == 15) {
      num = 0;
    }
  } else {
    PORTB &= ~(1 << PIN_1);
  }
}
