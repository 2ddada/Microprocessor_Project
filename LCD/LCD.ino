#include <Wire.h>                     // i2C 통신을 위한 라이브러리
#include <LiquidCrystal_I2C.h>        // LCD 2004 I2C용 라이브러리

LiquidCrystal_I2C lcd(0x27, 20, 4);   // 접근주소: 0x3F or 0x27

void display_setup()
{
  lcd.begin();                     // LCD 초기화
  lcd.backlight();                 // 백라이트 켜기
  
  lcd.setCursor(0, 0);             // 첫번째 줄 문자열 출력
  lcd.print("Temperature : ??");
 
  lcd.setCursor(0, 1);             // 두번째 줄 문자열 출력
  lcd.print("Humidity : ??");
 
  lcd.setCursor(0, 2);             // 세번째 줄 문자열 출력
  lcd.print("Hello World!!");
  
  lcd.setCursor(0, 3);             // 네번째 줄 문자열 출력
  lcd.print("Water Refill!!");
}

<<<<<<< HEAD
void display_action(int8_t temperature, int8_t humid, bool water_refill)
{
  lcd.setCursor(0, 0);             // 첫번째 줄 문자열 출력
  lcd.print("Temperature : ");
  lcd.print(temperature);
  lcd.print("oC");
=======
void display_action(float temp, float humid, bool water_refill)
{
  lcd.setCursor(0, 0);             // 첫번째 줄 문자열 출력
  lcd.print("Temperature : ");
  lcd.print(temp);
>>>>>>> 922546fa113dd26afedb504e6da59ef29c7aa539
 
  lcd.setCursor(0, 1);             // 두번째 줄 문자열 출력
  lcd.print("Humidity : ");
  lcd.print(humid);
<<<<<<< HEAD
  lcd.print("%");


  
=======
>>>>>>> 922546fa113dd26afedb504e6da59ef29c7aa539
 
  lcd.setCursor(0, 2);             // 세번째 줄 문자열 출력
  lcd.print("Hello World!!");
  
  lcd.setCursor(0, 3);             // 네번째 줄 문자열 출력
  lcd.print("Water Refill : ");
  lcd.print(water_refill ? "Yes" : "No");
}

void setup()
{
  display_setup();
}

<<<<<<< HEAD
int count = 0;
int8_t temperature, humid;
=======
float count = 0;
>>>>>>> 922546fa113dd26afedb504e6da59ef29c7aa539

void loop()
{ 
  // 예시 값, 실제 프로젝트에서는 센서로부터 값을 받아옴
  count++;
  if (count==8) count=0;
  
<<<<<<< HEAD
  temperature = 25+count;
  humid = 60+count;
  bool water_refill = true;

  display_action(temperature, humid, water_refill);
=======
  float temp = 25.5+count;
  float humid = 60.2+count;
  bool water_refill = true;

  display_action(temp, humid, water_refill);
>>>>>>> 922546fa113dd26afedb504e6da59ef29c7aa539
  delay(2000); // 2초마다 업데이트
}
