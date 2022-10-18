//hsDemoLED

#include <hsCoRoutines.h>

#define LED 13

//-----------------------------------------------------
hsTASK(T1)
{
  if (Setup) {
    Serial.println("T1 i");
    pinMode(LED, OUTPUT);
    pinMode(A1, OUTPUT);
    digitalWrite(A1, 0);
  }
  else Serial.println("T1...");
  digitalWrite(LED, 1); digitalWrite(A1, 0);
  hsIdle(500);
  digitalWrite(LED, 0); digitalWrite(A1, 1);
  //Serial.println("T1 idle 3000");
  hsIdle(1500);
}

hsTASK(T2)
{
  static char b2;
  if (Setup) {pinMode(A2, OUTPUT); digitalWrite(A2, 0);}
  if (Setup) Serial.println("T2 i"); else Serial.println("T2...");
  hsIdle(5000);
  digitalWrite(A2, 1 & ++b2);
}

hsGLOBALS(10)  //Max 10 coRoutines

void setup()
{ // put your setup code here, to run once:

  Serial.begin(9600);
  delay(1000);
  Serial.println("hsDemoLED");
  pinMode(A0, OUTPUT); digitalWrite(A0, 1);
  delay(1000);
  hsTASK_SETUP(T1,200);
  hsTASK_SETUP(T2,200);
}

void loop()
{ // put your main code here, to run repeatedly:
  Serial.println("MAIN Loop");
  char Buf[60];
  for (byte ll=1; ll<=TaskMax; ll++)
    if (Tasks[ll].SPCur) Serial.println(hsStatusS60(Buf,ll));
  hsIdle(10000);
}

