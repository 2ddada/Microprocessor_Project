/*

//완벽한 코드 이걸로 사용 !



#define OC0A 0b01000000 //6번핀 사용  5,6번핀은 쿨러 회전을 위한 핀.
#define OC0B 0b00100000 //5번핀 사용
#define Relay_controll (1<<PB0) //릴레이 제어하는 핀은 8번핀.
uint16_t temperature=20.0;


void setup()
{
    DDRB|=Relay_controll; //릴레이 제어하는 핀-> 세라믹 히터 on//off
    DDRD|=(OC0A)|(OC0B); // 쿨러 회전을 위한 핀 설정. PWM을 사용해야 하기 때문에 꼭 해당 핀 사용
    //타이머 카운터 작동 모드 Fast PWM으로 설정.
    TCCR0A|=(1<<WGM01)|(1<<WGM00);
    TCCR0B|=(0<<WGM02);
    //타이머 카운터가 동작하기 위해 사용되는 클럭 선택
    TCCR0B|=(0<<CS02)|(1<<CS01)|(1<<CS00);

    //OCOA의 출력모드 설정
    TCCR0A|=(1<<COM0A1)|(0<<COM0A0);
    //OC0B의 출력 모드 설정
    TCCR0A|=(1<<COM0B1)|(0<<COM0B0);

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
        PORTB&=~Relay_controll;
        
        SREG|=1<<SREG_I; //SREG레지스터의 I비트를 set.->인터럽트 사용 o

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR0A=200; 
        OCR0B=0;

    }
}

ISR(TIMER0_COMPA_vect) 
{
    //인터럽트가 발생했을 때 어떤 상황을 발생시킬 지 정의 후 작성 예정.
    //부저 이용해 팬 켜질 때 소리 한 번 나게 할 수 있다. -> 사이클이 만날 때마다 발생하므로 하면 안됨. 처음에만 발생하게 할 수는 없을까? 그냥 i비트 끄면 되나?
    //어차피 i비트 먼저 키고 ocr0a를 지정함.
}  
