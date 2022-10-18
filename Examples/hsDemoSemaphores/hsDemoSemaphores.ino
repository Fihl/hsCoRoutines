//hsDemoSemaphores

#include <hsCoRoutines.h>

hsGLOBALS(10)

#define RXCnt 6
#define TXCnt 2
byte RXStacks[RXCnt] [120];
byte TXStacks[TXCnt] [120];

char SharedPrintFBuf[100];

byte SemaphoreVar=0;

//-----------------------------------------------------
hsTASK(Receivers)
{
  hsIdle(2000);
  //Serial.println("rx.");
  if (hsSemaWait(15000,SemaphoreVar)) sprintf(SharedPrintFBuf,"RX:%d ++",TaskIndex);
  else sprintf(SharedPrintFBuf,"RX:%d --",TaskIndex);
  if (SharedPrintFBuf[0]) Serial.println(SharedPrintFBuf);
}

hsTASK(Senders)
{
  hsIdle(random(2000,5000));
  for (byte ll=random(1,9); ll>0; ll--) {
    Serial.println("TX..");
    hsSemaSignal(SemaphoreVar);
  }
}

void setup()
{ // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("start");
  for (byte ll=0; ll<RXCnt; ll++) 
    hsTASK_SETUP_STACK(Receivers,RXStacks[ll]);
  for (byte ll=0; ll<TXCnt; ll++) 
    hsTASK_SETUP_STACK(Senders,TXStacks[ll]);
}

void loop()
{ // put your main code here, to run repeatedly:
  //Serial.println("MAIN Loop");
  char Buf[60];
  hsIdle(1000);
  //if(0)
  for (byte ll=1; ll<=TaskMax; ll++)
    //if (Tasks[ll].SPCur) Serial.println(hsStatusS60(Buf,ll));
    if (Tasks[ll].State) Serial.println(hsStatusS60(Buf,ll));
  hsIdle(4000);
}

/* Look for 
*/
