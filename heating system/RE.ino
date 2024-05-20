//코드 상 아무 문제 없음. 회로 연결이 문제.


#define OC2A 0x08
#define Relay_controll (1<<PB0) //릴레이 제어하는 핀은 8번핀.
uint16_t temperature=22;


void setup()
{
    DDRB|=Relay_controll|OC2A; //릴레이 제어하는 핀-> 세라믹 히터 on//off
    //타이머 카운터 작동 모드 Fast PWM으로 설정.
    TCCR2A|=(1<<WGM21)|(1<<WGM20);
    //타이머 카운터가 동작하기 위해 사용되는 클럭 선택
    TCCR2B|=(1<<CS22)|(1<<CS21)|(1<<CS20);
    //OCOA의 출력모드 설정
    TCCR2A|=(1<<COM2A1);
    //OC0B의 출력 모드 설정
    TCCR2A|=(1<<COM2B1);
    //인터럽트 활성화
    TIMSK2|=(1<<OCIE2A)|(1<<OCIE2B);
}


void loop()
{
    if (temperature<=18.0)
    {
        //릴레이 on->세라믹 히터 on
        PORTB|=Relay_controll;
        
        SREG&=~1<<SREG_I; //SREG레지스터의 I비트를 clear.->인터럽트 사용 x

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR2A=0;
        
    }
    else if (temperature>18.0 && temperature<23.0)
    {
        PORTB&=~Relay_controll;
        
        SREG&=~(1<<SREG_I); //SREG레지스터의 I비트를 clear.->인터럽트 사용 x

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR2A=10;
        
    }

    else
    {
        PORTB&=~Relay_controll; //세라믹 히터 꺼줌
        
        SREG|=1<<SREG_I; //SREG레지스터의 I비트를 set.->인터럽트 사용 o

        //듀티 사이클 설정을 위한 OCR0A,OCR0B값 설정. 0이면 duty cycle 0. 255이면 duty cycle 100.
        OCR2A=255;
       
    }
}