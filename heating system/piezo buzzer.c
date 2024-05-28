//완벽한 코드 이걸로 사용

#define OC0A 0b01000000 //6번핀 사용 쿨러 회전을 위한 핀.
#define OC2B 0b00001000 //5번핀 사용 피에조 부조를 위한 핀.
#define Relay_controll (1<<PD7) //릴레이 제어하는 핀은 8번핀.

uint16_t temperature=25.0;

const float frequencies[7] = { 
  392.00, // 솔 (G4)
  392.00, // 솔 (G4)
  440.00, // 라 (A4)
  440.00, // 라 (A4)
  392.00, // 솔 (G4)
  392.00, // 솔 (G4)
  329.63, // 미 (E4)
};

uint8_t freq_count=0;
uint8_t flag=1;


void setup()
{
    DDRD|=Relay_controll; //릴레이 제어하는 핀-> 세라믹 히터 on//off
    DDRD|=(OC0A)|(OC2B); // 쿨러 회전을 위한 핀 설정. PWM을 사용해야 하기 때문에 꼭 해당 핀 사용
    

    //타이머 카운터 작동 모드 Fast PWM으로 설정.
    TCCR0A|=(1<<WGM01)|(1<<WGM00);
    TCCR0B|=(0<<WGM02);
    //타이머 카운터가 동작하기 위해 사용되는 클럭 선택
    TCCR0B|=(0<<CS02)|(1<<CS01)|(1<<CS00);

    //OCOA의 출력모드 설정
    TCCR0A|=(1<<COM0A1)|(0<<COM0A0);
    //인터럽트 활성화
    TIMSK0|=(1<<OCIE0A);

    TCCR2A|=(1<<WGM21)|(1<<WGM20);
    TCCR2B|=(1<<WGM22);
    TCCR2B|=(1<<CS22)|(1<<CS21)|(0<<CS20);

    TCCR2A|=(1<<COM2B1);

    float freq_target=frequencies[freq_count];

    OCR2A=F_CPU/256/freq_target-1;
    OCR2B=OCR2A/2;



}


void loop()
{
    if (temperature<=18.0)
    {
        //릴레이 on->세라믹 히터 on
        PORTD|=Relay_controll;
        DDRD&=~(OC2B);
        
        SREG&=~1<<SREG_I; //SREG레지스터의 I비트를 clear.->인터럽트 사용 x

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=0;
        freq_count=0;
        flag=1;
    }
    else if (temperature>18.0 && temperature<23.0)
    {
        PORTD&=~Relay_controll;
        DDRD&=~(OC2B);
        
        SREG&=~(1<<SREG_I); //SREG레지스터의 I비트를 clear.->인터럽트 사용 x

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=0;
        freq_count=0;
        flag=1;
    }

    else
    {
        PORTD&=~Relay_controll;
        DDRD|=OC2B;
        SREG|=1<<SREG_I; //SREG레지스터의 I비트를 set.->인터럽트 사용 o
  
        //듀티 사이클 설정을 위한 ,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=200;
    }

}

ISR(TIMER0_COMPA_vect)
{
  if (flag == 1 && freq_count >= 0 && freq_count < 7)
  {
    freq_count+=1;
  
    float freq_target=frequencies[freq_count];
  
    OCR2A=F_CPU/256/freq_target-1;
  
    OCR2B=OCR2A/10000;
  
    for (uint16_t j=0;j<50;j++)
    {
      for (uint16_t i=0;i<64000;i++)
      {
        asm("nop");
      }
    }
  }
  else // 마지막 소리가 종료되면
  {
    flag = 0; // 플래그 설정하여 더 이상 ISR이 소리를 재생하지 않도록 함
  }
}