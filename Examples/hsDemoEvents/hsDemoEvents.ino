//hsDemoEvents

#include <hsCoRoutines.h>

hsGLOBALS(10)

#define RXCnt 3
#define TXCnt 2
byte RXStacks[RXCnt] [150];
byte TXStacks[TXCnt] [150];

char SharedPrintFBuf[100];

//-----------------------------------------------------
hsTASK(Receivers)
{
  hsIdle(500);
  if (hsEventWait(5000,123)) sprintf(SharedPrintFBuf,"RX:%d ++",TaskIndex);
  else SharedPrintFBuf[0]=0; //Serial.println(sprintf(SharedPrintFBuf,"RX:%d --",TaskIndex));
  if (SharedPrintFBuf[0]) Serial.println(SharedPrintFBuf);
}

hsTASK(Senders)
{
  hsIdle(random(2000,5000));
  hsEventSignal(123);
  Serial.println("TX..");
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
  for (byte ll=1; ll<=TaskMax; ll++)
    //if (Tasks[ll].SPCur) Serial.println(hsStatusS60(Buf,ll));
    if (Tasks[ll].State) Serial.println(hsStatusS60(Buf,ll));
  hsIdle(4000);
}

/* Look for 
TX..
RX:1 ++
RX:2 ++
RX:3 ++
TX..
RX:1 ++

or

TX..
RX:1 ++
RX:2 ++
RX:3 ++
TX..
TX..
RX:1 ++
RX:2 ++
RX:3 ++

When RX i not ready (doing hsIdle(500);), it does not get the signal
*/
