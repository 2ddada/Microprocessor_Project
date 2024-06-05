
//user_mode=0 -> 자동화모드 user_mode=1-> 사용자모드
int user_mode=0; //처음에는 자동화모드로 설정
char c='a'; //처음에는 자동화 모드 설정

void loop() {

    if (btSerial.available()){  //블루투스로 문자 받아옴
        c=btSerial.read();

    if (user_mode==0&c!='a')  //자동화모드에서 다른 문자가 선택되면 무시하기 위한 코드. 받아온 문자를 다시 a로 바꿔줌.
        c='a';

    if (c=='a')  //자동화모드가 선택된다면 원래 루프 함수 돌림
    {
            user_mode=0; //사용자 모드 비활성화 시킴

            //원래 루프 함수
    }

    else //자동화모드 버튼 말고 다른 것이 눌렸을 때
    {   
        if (c=='b') //사용자모드 버튼이 눌렸다면 user_mode가 1이 되게 하여, 사용자모드 활성화
            user_mode+=1;
        if (user_mode==1) //사용자모드로 선택되어 있다면, 환기 워터펌프 히팅 쿨링 관련 버튼 눌렀을 때 활성화.
        {
            switch (c)
            {
                case 'c':
                    //환기팬 on하는 코드 입력
                    break;
                
                case 'd':
                    //환기팬 off하는 코드 입력
                    break;
                
                case 'e':
                    //워터펌프 on하는 코드 입력
                    break;
                
                case 'f':
                    //워터펌프 off하는 코드 입력
                    break;
                
                case 'g':
                    //히팅 on하는 코드 입력
                    break;
                
                case 'h':
                    //히팅 off하는 코드 입력
                    break;
                
                case 'i':
                    //쿨링 on하는 코드 입력
                    break;
                
                case 'j':
                    //쿨링 off하는 코드 입력
                    break;
            }
        }
    }
    
  }
}