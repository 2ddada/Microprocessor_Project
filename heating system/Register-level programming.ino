void heating_system(double temperature)
{
    #define OC0A 0b01000000;
    #define OC0B Ob00100000;
    #define Relay_controll (1<<PB0);

    DDRB|=Relay_controll; //릴레이 제어하는 핀-> 세라믹 히터 on//off
    DDRD|=OC0A|OC0B; // 쿨러 회전을 위한 핀 설정. PWM을 사용해야 하기 때문에 꼭 해당 핀 사용
    
    //타이머 카운터 작동 모드 Past PWM으로 설정.
    TCCR0A|=(1<<WGM01)|(1<<WGM00);
    TCCR0B|=(0<<WGM02)
    //타이머 카운터가 동작하기 위해 사용되는 클럭 선택
    TCCR0B|=(1<<CS22)|(1<<CS21)|(1<<CS20);

    //OCOA의 출력모드 설정
    TCCR0A|=(1<<COM0A1)|(0<<COM0A0);
    //OC0B의 출력 모드 설정
    TCCR0B|=(1<<COM0B1)|(0<<COM0B0);

    //타이머0의 출력 비교 인터럽트A를 활성화. OC0A핀과 관련
    TIMSK0|=(1<<OCIE0A); 
    //타이머0의 출력 비교 인터럽트B를 활성화. OC0B핀과 관련
    TIMSK0|=(1<<OCIE0B);

    //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 임의로 128 넣어놓음.
    OCR0A=128;
    OCR0B=128;

    if (temperature<=18.0)
    {
        //릴레이 on->세라믹 히터 on
        PORTB|=Relay_controll;
        
        cli(); //SREG레지스터의 I비트를 clear.->인터럽트 사용 x

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=0;
        OCR0B=0;
    }
    else if (temperature>18.0 && temperature<23.0)
    {
        PORTB&=~Relay_controll;
        
        cli(); //SREG레지스터의 I비트를 clear.->인터럽트 사용 x

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=0;
        OCR0B=0;
    }

    else
    {
        PORTB&=~Relay_controll;
        
        sei(); //SREG레지스터의 I비트를 set.->인터럽트 사용 o

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=128; 
        OCR0B=0;

    }

}

ISR(TIMER0_COMPA_vect) 
{
    //인터럽트가 발생했을 때 어떤 상황을 발생시킬 지 정의 후 작성 예정.
    //부저 이용해 팬 켜질 때 소리 한 번 나게 할 수 있다.
}  
   
