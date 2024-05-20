#define OC0A 0b01000000 //6번핀 사용  5,6번핀은 쿨러 회전을 위한 핀.
#define OC0B 0b00100000 //5번핀 사용
#define Relay_controll (1<<PB0) //릴레이 제어하는 핀은 8번핀.
uint16_t temperature=24.0;


void setup()
{    
    DDRB|=Relay_controll; //릴레이 제어하는 핀-> 세라믹 히터 on//off
    DDRD|=(OC0A)|(OC0B); // 쿨러 회전을 위한 핀 설정. PWM을 사용해야 하기 때문에 꼭 해당 핀 사용
    //타이머 카운터 작동 모드 Fast PWM으로 설정.
    TCCR0A|=(1<<WGM01)|(1<<WGM00);
    TCCR0B|=(0<<WGM02);
    //타이머 카운터가 동작하기 위해 사용되는 클럭 선택
    TCCR0B|=(1<<CS02)|(1<<CS01)|(1<<CS00);

    //OCOA의 출력모드 설정
    TCCR0A|=(1<<COM0A1);
    //OC0B의 출력 모드 설정
    TCCR0A|=(1<<COM0B1);
    TIMSK0|=(1<<OCIE0A)|(1<<OCIE0B);
}


void loop()
{
    if (temperature<=18.0)
    {
        //릴레이 on->세라믹 히터 on
        PORTB|=Relay_controll;
        
        SREG&=~1<<SREG_I; //SREG레지스터의 I비트를 clear.->인터럽트 사용 x

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=0;
        OCR0B=0;
    }
    else if (temperature>18.0 && temperature<23.0)
    {
        PORTB&=~Relay_controll;
        
        SREG&=~(1<<SREG_I); //SREG레지스터의 I비트를 clear.->인터럽트 사용 x

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=0;
        OCR0B=0;
    }

    else
    {
        PORTB&=~Relay_controll; //세라믹 히터 꺼줌
        
        SREG|=1<<SREG_I; //SREG레지스터의 I비트를 set.->인터럽트 사용 o

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=128;
        OCR0B=0;

    }
}
