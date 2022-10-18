/*
    Copyright (c) 2014 Christen Fihl, Christen@ihl.net, http://www.fihl.net

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

    $Id$
*/

#include "hsCoRoutines.h"
#include <avr/pgmspace.h>

char* eStateS[]={"-", "E_Released", "E_TOut", "E_Sema", "E_Event", "E_Mail"};

//https://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
int __attribute__ ((always_inline)) getSP()
{
  int n;
  asm volatile (
    "in %B0,__SP_H__ \n\t"
    "in %A0,__SP_L__ \n\t"
    "nop"
   : "=r" (n)
   :
   :);
  return n;
}

void __attribute__ ((always_inline)) setSP(int newSP)
{
  asm volatile (
    "cli                          $"
    "out    __SP_H__,  %B[newSP]  $" 
    "out    __SP_L__,  %A[newSP]  $"
    "sei                          $"
  : 
  : [newSP]  "r"  (newSP)
  : 
  );
}

//http://web.stanford.edu/class/ee281/projects/aut2002/yingzong-mouse/media/GCCAVRInlAsmCB.pdf
//Disable interrupts, and later restore to same state
#define hsDI { byte IRestore; asm volatile ("in %0, __SREG__ $ cli" : "=r" (IRestore) :: );
#define hsEI                  asm volatile ("out __SREG__, %0" :: "r" (IRestore):); }

//-----------------------------------------------------

byte TaskCount;

static unsigned long lastmillis;
boolean UpdateAllDelays()
{ unsigned long ms2=millis(); 
  unsigned long ms=ms2-lastmillis; 
  lastmillis = ms2;
  if (0==ms) return 0;
  for (byte ll=0;ll<=TaskCount;ll++) {
    if (!Tasks[ll].SPCur) continue;
    unsigned int newDelay=Tasks[ll].Delay;
    if (newDelay>0) {
      if (ms>=newDelay) newDelay=0;
      else              newDelay -= ms; 
      
      Tasks[ll].Delay=newDelay;
    }
  }
  return 1;
}

int hsSetupXptr;
byte newTaskIndex;

//Return True if E_Released
byte hsSwap()
{ asm __volatile__ (
    "":::
    "r0","r1","r2","r3","r4","r5","r6","r7",
    "r8","r9","r10","r11","r12","r13","r14","r15",
    "r16","r17","r18","r19","r20","r21","r22","r23",
    "r24","r25","r26","r27","r28","r29","r30","r31"
  );
  Tasks[TaskIndex].SPCur=getSP();
  
  if (newTaskIndex) {
    TaskIndex=newTaskIndex;
    newTaskIndex=0;
    setSP(Tasks[TaskIndex].SPCur);
    int ret=hsSetupXptr;
    asm volatile ("movw r30, %0 $ ldi r24,2 $ ijmp" :: "r" (ret):);
  }
  
  //digitalWrite(A0, 1);
  while(1) {
    if (!UpdateAllDelays()) {delay(10); continue;}  //////////Sleep now
    
    TaskIndex=0;
    
    for (byte ll=0;ll<=TaskCount;ll++) {
      if (++TaskIndex > TaskCount) TaskIndex=0;
      if (!Tasks[TaskIndex].SPCur) continue;
      if (Tasks[TaskIndex].Delay) continue;
      //digitalWrite(A0, 0);
      setSP(Tasks[TaskIndex].SPCur);
      byte ST= Tasks[TaskIndex].State==E_Released?1:0;
      Tasks[TaskIndex].State = E_None;
      //nono Tasks[TaskIndex].Delay = 0;
      Tasks[TaskIndex].Obj   = 0;
      return ST;
    }
  }
}

void hsIdle(unsigned int waitMS)
{
  Tasks[TaskIndex].Delay=waitMS;
  hsSwap();
}

void hsSetup(int newIP, int newSP, int newSPsize)
{
  if (TaskCount>=TaskMax) return;
  newTaskIndex=++TaskCount;
  hsSetupXptr=newIP;
  Tasks[newTaskIndex].SPBot=(byte*)newSP;
  Tasks[newTaskIndex].SPCur=newSP+newSPsize-1;
  hsIdle(0);
}

//-----------------------------------------------------------
void SetFlags(byte TaskIx, byte State, int Delay, int Obj)
{
  hsDI;
    Tasks[TaskIx].State = (eState)State;
    Tasks[TaskIx].Delay = Delay;
    Tasks[TaskIx].Obj   = Obj;
  hsEI;
}

void Signal(int AObj, byte AState)
{
  hsDI;
  for (byte ll=0;ll<=TaskCount;ll++) {
    if (AState==Tasks[ll].State)
      if (AObj==Tasks[ll].Obj) {
        SetFlags(ll,E_Released,0,0);
        if (AState == E_Sema) break;
      }
  }
  hsEI;
}

//======================================================================================
//Semaphores. Works on 1-byte user allocated memory
boolean hsSemaWait(int TOut, byte& SNo)
{
  if (!SNo) {
    SetFlags(TaskIndex,E_Sema, TOut, (int)&SNo);
    if (!hsSwap()) return 0; //Timeout
  }
  hsDI;
    SNo--;
  hsEI;
  return 1;
}
void hsSemaSignal(byte& SNo)
{
  hsDI;
    SNo++;
    Signal((int)&SNo,E_Sema);
  hsEI;
  //hsIdle(0);
}
void hsSemaSignal_interrupt(byte& SNo)
{
  hsDI;
    SNo++;
    Signal((int)&SNo,E_Sema);
  hsEI;
}

//======================================================================================
//Events, when signaled, all is released
boolean hsEventWait(int TOut, byte ENo)
{
  SetFlags(TaskIndex, E_Event, TOut, ENo);
  return hsSwap();
}
void hsEventSignal(byte ENo)
{
  Signal(ENo,E_Event);
  //hsIdle(0);
}
void hsEventSignal_interrupt(byte ENo)
{
  Signal(ENo,E_Event);
}

//======================================================================================
//Message sending, a message = an int.
boolean getQ(byte MNo, int& val)
{
  hsDI;
  hsEI;
  val=0; return 1;
}
boolean addQ(byte MNo, int val)
{
  hsDI;
  hsEI;
  return 0;
}

boolean hsMailWait(int TOut, byte MNo, int& Result)
{
  SetFlags(TaskIndex, E_Mail, TOut, MNo);
  while (1) {
    if (getQ(MNo,Result)) {SetFlags(TaskIndex, E_None, 0, 0); return 1;}
    if (!hsSwap()) return 0; //Timeout
  }
}
boolean hsMailSend(byte MNo, int val)
{
  if (!addQ(MNo,val)) return 0;
  Signal(MNo,E_Mail);
  //hsIdle(0);
  return 1;
}
void hsMailSend_interrupt(byte MNo, int val)
{
  addQ(MNo,val);
  Signal(MNo,E_Mail);
}
//======================================================================================

int hsStackFree(byte Task)
{
  int res=0;
  byte *P=Tasks[Task].SPBot;
  if (P) while (!*P++) res++;
  return res;
}

char* hsStatusS60(char* buf, byte Task)
{ snprintf_P(buf,60,PSTR("T:%2d, StackFree:%3dB, Timer:%5dmSec, State: %s,%d"),
    Task,  
    hsStackFree(Task), 
    //Tasks[Task].SPCur-(int)Tasks[Task].SPBot, 
    Tasks[Task].Delay, 
    eStateS[Tasks[Task].State], 
    Tasks[Task].Obj );
  return buf;
}
//======================================================================================

