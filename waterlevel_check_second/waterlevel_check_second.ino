/// 타이머를 이용하여 주기적으로 인터럽트를 주기 위해서 CTC모드를 사용해서 내가 설정한 주기가 되면
/// 인터럽트가 일어나도록 할 거라고 생각하자.

#define PIN_1 3 // 포트b의 3번째 핀
#define PIN_2 6 //  포트d 의 6번째 핀

/////// 수위 조절
//// 일정 수위 이하로 떨어지면 불 들어오도록
/// 일단 DDRB 핀 첫번째, 두번째, 세번째에 꽂는다고 가정하고
//// 수위는 400이하, 400~500, 500이상으로 된다고 짰음.
//// 5v랑 ground 이런거 핀 어떻게 나눌지 생각좀 해야할듯

#define NUM_LEDS 3
//#define ADC_RESOULUTION_IN_BITS 10    //<-- 필요없는건가?
#define PIN_of_LED_1 0
#define PIN_of_LED_2 1
#define PIN_of_LED_3 2

volatile int num = 0;

ISR(TIMER1_COMPA_vect){
  num ++;
  

  // 5초동안 펌프를 켜기 // 이거는 맞아 왜냐면 11~15 까지 딱 5초잖아
  if (num > 10 && num <= 15){
    PORTB |= (1 << PIN_1); // 인터럽트가 발생하면, 펌프가 돌도록 1을 넣어준다. 
    if (num == 15){      // 시리얼 모니터에 num = 15는 뜰 수가 없음. 
                         // 14에서 15가 되는 순간, 즉 num = 15가 되는 순간 인터럽트가 생기는데 15니까 num이 매우 짧은 시간에 0이 되고,
                         // loop에서 돌아서 print하는거에는 0으로 뜰 거임.
      num = 0;
    }
    
  } 
  else{        // 위에서 봤듯, 0은 곧 15라고 봐준다고 생각하면, 씨리얼 모니터상에서 1부터 10까지 뜰때까지의 10초동안은 펌프 꺼진 상태.
    PORTB &= ~(1<<PIN_1);
  }
  
}

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


void waterlevel_check(){
  
  // Start ADC

  ADCSRA |= (1 << ADSC);

  // Wait until ADC is completed
  while(ADCSRA & (1 << ADSC));

  uint16_t value = ADC;

  // 400이하라면 깜빡이는 기능까지 추가. 
  if(value < 400){    /// 이게 최저점 의미는 맞는듯
    PORTB = (1 << PIN_of_LED_1) | ( 1 << PIN_of_LED_2) | (1 << PIN_of_LED_3);
    delay(500);
    PORTB &= ~((1 << PIN_of_LED_1) | (1 << PIN_of_LED_2) | (1 << PIN_of_LED_3));
    delay(500);
  }

  else if(value > 400 && value < 500){    // 두칸을 키려면 500말고 좀 더 큰 범위로 바꿔주면 될 듯? 
    PORTB = (1 << PIN_of_LED_1) | (1 << PIN_of_LED_2);
    PORTB &= ~(1 << PIN_of_LED_3);
  }
  
  else {
    PORTB = (1 << PIN_of_LED_1) | (1 << PIN_of_LED_2) | (1 << PIN_of_LED_3);

  }

  Serial.println(value);
  delay(100);

}



void printNum(){
  Serial.print("num  : ");
  Serial.println(num);
  delay(1000);  //왜 딜레이를 1초로 거냐? 위에 인터럽트 생기는걸 1초로 해놨으니까 시리얼 모니터에 딱딱 1초단위로 뜨게 하려고
}

void pump_setup(){
  DDRB |= (1 << PIN_1); 
  DDRD |= (1 << PIN_2);
  PORTB &= ~(1 << PIN_1); // 처음에는 꺼줘서 펌프가 작동이 안되는 상태로 만들기 , 나중에 PORTB만 1로 써주면 돌아감.
  PORTD &= ~(1 << PIN_2);

  Serial.begin(9600);
  cli();

  // 타이머 1 설정 - CTC 모드, 프리스케일러 1024, 기본적으로 TCNT는 0으로 되어있음.
  // 왜 하필 타이머 1을 썼냐? 1초 단위로 돌아가게 하고싶은데, 타이머 0,2 는 OCR0A가 256이 최대여서,
  // 2^16까지 되는 타이머 1을 썼다. OCR1A를 저렇게 만들어서, 1초 간격으로 인터럽트가 생기게 할 수 있었다.
  TCCR1A = 0; // 모든 비트를 0으로 설정 
  TCCR1B = 0; // 얘도 일단 초기화
  TCCR1B |= (1 << WGM12) | (1 << CS12) | (0 << CS11) | (1 << CS10) ; // CTC 모드(1 0 0), 프리스케일러 1024
  //TCCR1A |= (0<<COM1A1) | (1 << COM1A0);   // 책에 나온대로 총 파장 주기의 반만큼이 1초라는거 이해하기 쉽도록 토글링으로 설정했음.
                                           // 잘 모르겠는데 짜피 oc쓸게 아니라서 굳이 저렇게 만들 필요가 없을것같긴함.
                                           // 이 코드에서는 비교매치 인터럽트만 이용할건데 출력까지 이용할거면 핀도 OC1A에 맞게 꽂아야해.
  

  // 타이머 1 비교 매치 인터럽트 설정
  OCR1A = 15624; // (16 * 10^6) / (1024 * 1) - 1 = 15624 (1초마다 인터럽트 발생)///
                 //  한틱 세는데 : 1 / (16MHz / 1024), 15624개 곱하면 0.9999로 거의 1이라 칠 수 있음.
                
  //// 파형 자체의 주기는 2초지만, 인터럽트 1초로 맞춰준거임(토글링 그림 보면, 전체 파장 주기는 2이지만 인터럽트 주기는 1임.)
  TIMSK1 |= (1 << OCIE1A); // 타이머 1 비교 매치 인터럽트 활성화

  // 전역 인터럽트 활성화
  sei();
}

void setup(){
  init_Serial();
  init_ADC();
  init_LED();
  pump_setup();
}

void loop() {
  printNum();
  waterlevel_check();
}






