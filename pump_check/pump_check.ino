/// 타이머를 이용하여 주기적으로 인터럽트를 주기 위해서 CTC모드를 사용해서 내가 설정한 주기가 되면
/// 인터럽트가 일어나도록 할 거라고 생각하자.

#define PIN_1 3 //포트b의 3번째 쓸거임
#define PIN_2 6 //포트d의 6번째 쓸거임

volatile int num_of_interrupt = 0;  // OCRnx 값이 635xx 어쩌고 까지니까 이걸 5분 간격으로 돌릴려고 해도 말이 안되니까 넘는 횟수를 세서 ㄱㄱ

void pump_setup(){
  DDRB |= (1 << PIN_1);    // 포트b 해당 핀을 출력으로 설정해주고
  DDRD |= (1 << PIN_2);    // 포트d 해당 핀을 출력으로 설정해주고
  PORTB &= ~(1 << PIN_1);   //처음에는 둘다 꺼줘서 펌프가 작동이 안되는 상태로 만들기
  PORTD &= ~(1 << PIN_2);

  Serial.begin(9600);    // 시리얼 통신 설정하기

  cli();  // 인터럽트 처리가 진행되는 동안 다른 인터럽트 진행 안되도록 처음에는 cli()로 global in 
  // CTC모드를 만들어주기 0 1 0

  TCCR1A |= (1 << WGM11);
  TCCR1A &= ~(1 << WGM10);
  TCCR1B &= ~(1 << WGM12);
  
  // 프리스케일러 1024로 설정해주기

  TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);

  // foc = fclk / (2*prescaler) / (1 + OCRnx)를 생각해서 OCR1A를 설정하기
  // 일단 주기를 5분이라고 생각하자. 5분 = 300초
  // (300) * (16*10^6)/(1024 * 2) - 1 = 2343749 
 
  OCR1A = 23437.49;  // 2343749 를 100으로 나눈거니까, 이게 100번 돌아가게 되면 ISR 속의 if문이 돌아가게 되겠지.

  // 타이머 1 비교 인터럽트 켜기
  TIMSK1 |= (1 << OCIE1A);

  sei();   // 그 다음에 전역 인터럽트를 활성화하기.

}
void setup() {
  pump_setup();
}

ISR(TIMER_COMPA_vect){
  num_of_interrupt ++;

  if (num_of_interrupt >= 100){
    PORTB |= (1 << PIN_1); // 인터럽트가 발생하면, 정회전이 되도록 1을 넣어준다.
    delay(5000); // 5초동안 펌프가 동작되도록 한다. 
    num_of_interrupt = 0;
  }
  else{

  }

void loop() {
  // put your main code here, to run repeatedly:

}
