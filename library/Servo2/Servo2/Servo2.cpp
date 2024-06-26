#include <avr/interrupt.h>
//#include <wiring.h>
#include <wiring_private.h> //Zimbu added this line and commented out wiring.h based on internet advice to prevent error when compiling
#include <Servo2.h>

/*
  Servo2.cpp - 2 Servo hardware timer Library
  Author: Jim Studt, jim@federated.com
  Modified for Mega Pins by Michael Margolis 
  Copyright (c) 2007 David A. Mellis.  All right reserved.
  
  This is the Servo library distributed with Arduino 0016 and earlier.
  It can drive up to two servos using pins 9 and 10 on a standard board
  or 11 and 12 on a Mega. Other pins wont work.
    
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if defined(__AVR_ATmega1280__)
// Mega uses pins 11 and 12 for timer 1
const uint8_t servoAPin = 11;
const uint8_t servoBPin = 12;
#else
// Pins 9 and 10 for Standard Arduino (ATmega168/328
const uint8_t servoAPin = 9;
const uint8_t servoBPin = 10;
#endif

uint8_t Servo::attachedA = 0;
uint8_t Servo::attachedB = 0;

void Servo::seizeTimer1()
{
  uint8_t oldSREG = SREG;

  cli();
  TCCR1A = _BV(WGM11); /* Fast PWM, ICR1 is top */
  TCCR1B = _BV(WGM13) | _BV(WGM12) /* Fast PWM, ICR1 is top */
  | _BV(CS11) /* div 8 clock prescaler */
  ;
  OCR1A = 3000;
  OCR1B = 3000;
  ICR1 = clockCyclesPerMicrosecond()*(20000L/8);  // 20000 uS is a bit fast for the refresh, 20ms, but 
                                                  // it keeps us from overflowing ICR1 at 20MHz clocks
                                                  // That "/8" at the end is the prescaler.
#if defined(__AVR_ATmega8__)
  TIMSK &= ~(_BV(TICIE1) | _BV(OCIE1A) | _BV(OCIE1B) | _BV(TOIE1) );
#else
  TIMSK1 &=  ~(_BV(OCIE1A) | _BV(OCIE1B) | _BV(TOIE1) );
#endif

  SREG = oldSREG;  // undo cli()    
}

void Servo::releaseTimer1() {}

#define NO_ANGLE (0xff)

Servo::Servo() : pin(0), angle(NO_ANGLE) {}

uint8_t Servo::attach(int pinArg)
{
  return attach(pinArg, 544, 2400);
}

uint8_t Servo::attach(int pinArg, int min, int max)
{
  if (pinArg != servoAPin && pinArg != servoBPin) return 0;
  
  min16 = min / 16;
  max16 = max / 16;

  pin = pinArg;
  angle = NO_ANGLE;
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);

  if (!attachedA && !attachedB) seizeTimer1();

  if (pin == servoAPin) {
    attachedA = 1;
    TCCR1A = (TCCR1A & ~_BV(COM1A0)) | _BV(COM1A1);
  }
  
  if (pin == servoBPin) {
    attachedB = 1;
    TCCR1A = (TCCR1A & ~_BV(COM1B0)) | _BV(COM1B1);
  }
  return 1;
}

void Servo::detach()
{
  // muck with timer flags
  if (pin == servoAPin) {
    attachedA = 0;
    TCCR1A = TCCR1A & ~_BV(COM1A0) & ~_BV(COM1A1);
    pinMode(pin, INPUT);
  } 
  
  if (pin == servoBPin) {
    attachedB = 0;
    TCCR1A = TCCR1A & ~_BV(COM1B0) & ~_BV(COM1B1);
    pinMode(pin, INPUT);
  }

  if (!attachedA && !attachedB) releaseTimer1();
}

void Servo::write(int angleArg)
{
  uint16_t p;

  if (angleArg < 0) angleArg = 0;
  if (angleArg > 180) angleArg = 180;
  angle = angleArg;

  // bleh, have to use longs to prevent overflow, could be tricky if always a 16MHz clock, but not true
  // That 8L on the end is the TCNT1 prescaler, it will need to change if the clock's prescaler changes,
  // but then there will likely be an overflow problem, so it will have to be handled by a human.
  p = (min16*16L*clockCyclesPerMicrosecond() + (max16-min16)*(16L*clockCyclesPerMicrosecond())*angle/180L)/8L;
  if (pin == servoAPin) OCR1A = p;
  if (pin == servoBPin) OCR1B = p;
}

uint8_t Servo::read()
{
  return angle;
}

uint8_t Servo::attached()
{
  if (pin == servoAPin && attachedA) return 1;
  if (pin == servoBPin && attachedB) return 1;
  return 0;
}