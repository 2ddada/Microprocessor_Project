// C++ code
//

// 일단 간단히 체크만 하기 위해
// 온도센서로부터 온도 받아오고 내가 정했던 온도(17~23도) 벗어나면
// 내장 LED 켜기


#define NUM_LEDS 4
#define ADC_RESOLUTION_IN_BITS 10


const uint16_t step = pow(2, ADC_RESOLUTION_IN_BITS)/NUM_LEDS;


void init_LED(){
    //내장led 키려고 하는거니까 PortB의 5번 비트 출력으로 설정
    // DDRB = 0b 0010 0000
    DDRB |= (1 << 0x20);
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

    // 뭐 지금 당장은 ADC0으로 들어오게 한다 해야겠다
    ADMUX |= (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);

    // ADC enable ADC가 작동할 준비
    ADCSRA |= (1 << ADEN);

    // prescaler 128로..(cpu 128 clock에 adc 1번)
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 

}

// 온도체크
// 일단은 interrupt 안하고 간단히 busywait으로 구현
// busywait함수를 loop함수에 직접적으로 넣어야하나 아니면 개별 함수마다 넣어야하나?
// 나중에 interrupt 구현하면 상관없을거같긴 한데 일단은 개별 함수마다 구현해야겠다
void temp_check(){
    // CPU가 ADSC 비트 1로 바꿔줄때까지 busywait 
    while (ADCSRA & (1 << ADSC));
    // 8비트 레지스터 두개 합쳐서 16비트의 값으로 바꾸고 temp_temperature 변수에 저장 (지금은 전압값인 상태)
    uint16_t temp_temperature = (uint16_t)ADCL | ((uint16_t)(ADCH) << 8);

    // 이다음에 실제 온도값으로 변경 , DHT22 센서 기준, 노션에 library 뜯어서 정리해둠
    // 아 라이브러리 adafruit꺼도 뜯어야되네

}


void setup()
{
//   init_Serial();
//   init_ADC();
//   init_LED();
}

void loop()
{

}