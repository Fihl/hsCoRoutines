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

#ifndef hsCoRoutines_H
#define hsCoRoutines_H

#include "Arduino.h"

enum eState {E_None=0, E_Released, E_TOut, E_Sema, E_Event, E_Mail};
struct hsTasks
{ int  SPCur;
  byte* SPBot;
  unsigned int Delay;
  enum eState State;
  int Obj;
};

extern hsTasks Tasks[];
extern byte TaskIndex, TaskMax;


//To be declared in main program  
#define hsGLOBALS(MaxTaskTotal) \
  hsTasks Tasks[MaxTaskTotal+1]; \
  byte TaskIndex, TaskMax=MaxTaskTotal;

#define hsTASK_SETUP(proc,stackSize) \
	static char Stack##proc[stackSize]; \
	hsSetup((int) proc##__, (int)Stack##proc, stackSize);

#define hsTASK_SETUP_STACK(proc,stack) \
	hsSetup((int) proc##__, (int)stack, sizeof(stack) );

#define hsTASK_SETUP_STACK_SIZE(proc,stack,stackSize) \
	hsSetup((int) proc##__, (int)stack, stackSize );

#define hsTASK(proc) \
  void proc(boolean Setup); \
  void proc##__() { for(proc(1);;proc(0))hsIdle(0); } \
  void proc(boolean Setup) 


//======================================================================================
void hsIdle(unsigned int waitMS);
void hsSetup(int newIP, int newSP, int newSPsize);
byte hsSwap();

int hsStackFree(byte Task);
char* hsStatusS60(char* buf, byte Task);


//======================================================================================

//Semaphores. Works on 1-byte user allocated memory
boolean hsSemaWait(int TOut, byte& SNo);
void hsSemaSignal(byte& SNo);
void hsSemaSignal_interrupt(byte& SNo);

//Events, when signaled, all is released
boolean hsEventWait(int TOut, byte ENo);
void hsEventSignal(byte ENo);
void hsEventSignal_interrupt(byte ENo);

//Message sending, a message = an int.
//NOT working yet    boolean hsMailWait(int TOut, byte MNo, int& Result);
//NOT working yet    boolean hsMailSend(byte MNo, int val);
//NOT working yet    void hsMailSend_interrupt(byte MNo, int val);

#endif
