#include <SoftwareSerial.h>

// SoftwareSerial 객체를 생성합니다.
// RX_PIN과 TX_PIN을 실제 사용하고 있는 핀 번호로 바꿔야 합니다.
SoftwareSerial btSerial(12, 13); // RX, TX

//user_mode=0 -> 자동화모드 user_mode=1-> 사용자모드
uint8_t user_mode=0; //처음에는 자동화모드로 설정
char c='a';




void setup(){
   Serial.begin(9600);
   btSerial.begin(9600);
   Serial.println("Bluetooth Serial Test");
   
}

void loop() {

  
    if (btSerial.available()){  //블루투스로 문자 받아옴
        char temp=btSerial.read();
        if (temp == 'a' || temp == 'b' || temp == 'c' || temp == 'd' ||
            temp == 'e' || temp == 'f' || temp == 'g' || temp == 'h' ||
            temp == 'i' || temp == 'j')
          c=temp;
    }

    Serial.print("바깥 Received: ");
    Serial.print(user_mode); // user_mode 출력
    Serial.println(c);

    if (c=='a')  //자동화모드가 선택된다면 원래 루프 함수 돌림
    {
        user_mode=0; //사용자 모드 비활성화 시킴

        //원래 루프 함수
        Serial.print("Received: ");
        Serial.println(c);
    }

    else //자동화모드 버튼 말고 다른 것이 눌렸을 때
    {   
        if (c=='b') //사용자모드 버튼이 눌렸다면 user_mode가 1이 되게 하여, 사용자모드 활성화
            user_mode=1;
        if (user_mode==1) //사용자모드로 선택되어 있다면, 환기 워터펌프 히팅 쿨링 관련 버튼 눌렀을 때 활성화.
        {
            switch (c)
            {
                case 'c':
                    //환기팬 on하는 코드 입력
                    Serial.print("Received: ");
                    Serial.println(c);
                    break;
                
                case 'd':
                    //환기팬 off하는 코드 입력
                    Serial.print("Received: ");
                    Serial.println(c);
                    break;
                
                case 'e':
                    //워터펌프 on하는 코드 입력
                    Serial.print("Received: ");
                    Serial.println(c);
                    break;
                
                case 'f':
                    //워터펌프 off하는 코드 입력
                    Serial.print("Received: ");
                    Serial.println(c);
                    break;
                
                case 'g':
                    //히팅 on하는 코드 입력
                    Serial.print("Received: ");
                    Serial.println(c);
                    break;
                
                case 'h':
                    //히팅 off하는 코드 입력
                    Serial.print("Received: ");
                    Serial.println(c);
                    break;
                
                case 'i':
                    //쿨링 on하는 코드 입력
                    Serial.print("Received: ");
                    Serial.println(c);
                    break;
                
                case 'j':
                    //쿨링 off하는 코드 입력
                    Serial.print("Received: ");
                    Serial.println(c);
                    break;
            }
        }
    }
}