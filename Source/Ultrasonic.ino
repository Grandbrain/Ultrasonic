#include "Arduino.h"

class Ultrasonic
{
public:
  
    Ultrasonic(int trig, int echo)
    {
      pinMode(trig, OUTPUT);
      pinMode(echo, INPUT);
      Trig = trig;
      Echo = echo;
    }
    
    long Timing()
    {
      digitalWrite(Trig, LOW);
      delayMicroseconds(2);
      digitalWrite(Trig, HIGH);
      delayMicroseconds(10);
      digitalWrite(Trig, LOW);
      long duration = pulseIn(Echo, HIGH);
      return duration;
    }
    
    long Ranging()
    {
      long duration = Timing();
      long distacne = duration / 29 / 2;
      return distacne;
    }

private:

    int Trig;
    int Echo;
};

Ultrasonic ultrasonic(9, 8);

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  int dist = ultrasonic.Ranging();
  if(dist > 2 && dist < 400){
    Serial.print(dist);
    Serial.print(" ");
    delay(100);
  }
}