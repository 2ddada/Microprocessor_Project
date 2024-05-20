/*
void setup()
{
    pinMode(8,OUTPUT); //릴레이 제어를 위한 핀
    pinMode(5,OUTPUT); // 쿨러 회전을 위한 핀
    pinMode(6,OUTPUT); // 쿨러 회전을 위한 핀
}

void heating_system(temperature)
{
    if (temperature<=18.0)
    {
        digitalWrite(8,1);
        analogWrite(5,0);
        analogWrite(6,0);
    }
    else if (temperature>18.0 && temperature<23.0)
    {
        digitalWrite(8,0);
        analogWrite(5,0);
        analogWrite(6,0);
    }
    else
    {
        analogWrite(5,255);
        analogWrite(6,0);
        digitalWrite(8,0);
    }
}
*/