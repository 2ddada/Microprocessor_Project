
#define OC0A 0b01000000 //나중에 10진수로 바꾸기
#define OC0B 0b00100000 //나중에 10진수로 바꾸기
#define Relay_controll (1<<PB0)
#define temp_ledpin 0x20


void fan_setup(){
  DDRB|=Relay_controll; //릴레이 제어하는 핀-> 세라믹 히터 on//off
  DDRD|=(OC0A)|(OC0B); // 쿨러 회전을 위한 핀 설정. PWM을 사용해야 하기 때문에 꼭 해당 핀 사용
  //타이머 카운터 작동 모드 Past PWM으로 설정.
  TCCR0A|=(1<<WGM01)|(1<<WGM00);
  TCCR0B|=(0<<WGM02);
  //타이머 카운터가 동작하기 위해 사용되는 클럭 선택
  TCCR0B|=(1<<CS22)|(1<<CS21)|(1<<CS20);
  //OCOA의 출력모드 설정
  TCCR0A|=(1<<COM0A1)|(0<<COM0A0);
  //OC0B의 출력 모드 설정
  TCCR0B|=(1<<COM0B1)|(0<<COM0B0);    
}

void init_LED(){
    //내장led 키려고 하는거니까 PortB의 5번 비트 출력으로 설정
    // DDRB = 0b 0010 0000
    DDRB |= (1 << temp_ledpin);
}

//Serial communication setup
void init_Serial(){
    Serial.begin(9600);
    // delay(1000);
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

// 온도체크
// 일단은 interrupt 안하고 간단히 busywait으로 구현
// busywait함수를 loop함수에 직접적으로 넣어야하나 아니면 개별 함수마다 넣어야하나?
// 나중에 interrupt 구현하면 상관없을거같긴 한데 일단은 개별 함수마다 구현해야겠다
int8_t temp_check(){

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
    int8_t temperature = (float)temp_temperature * 5.0 / 1024.0 * 100.0;
    return temperature;
}

    
void heating_system(int8_t temperature)
{
    if (temperature<=18.0)
    {
        //릴레이 on->세라믹 히터 on
        PORTB|=Relay_controll;
        
        // SREG&=~1<<SREG_I; //SREG레지스터의 I비트를 clear.->인터럽트 사용 x

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=0;
        OCR0B=0;
    }
    else if (temperature>18.0 && temperature<23.0)
    {
        PORTB&=~Relay_controll;
        
        // SREG&=~1<<SREG_I; //SREG레지스터의 I비트를 clear.->인터럽트 사용 x

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=0;
        OCR0B=0;
    }

    else
    {
        PORTB&=~Relay_controll;
        
        // SREG|=1<<SREG_I; //SREG레지스터의 I비트를 set.->인터럽트 사용 o

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=128; 
        OCR0B=0;

    }
}



void setup() {
  // put your setup code here, to run once:
  init_Serial();
  fan_setup();
  init_LED();
  init_ADC();
}
  int8_t temp; 

void loop() {
  // put your main code here, to run repeatedly:

  temp = temp_check();
  heating_system(temp);

  if (temp > 25) PORTB |= temp_pin;
  else PORTB &= ~temp_pin;

  Serial.print("Temperature: ");
  Serial.println(temp);
  delay(1000);
}
