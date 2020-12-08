/*
 * main.c
 *
 * Created: 11/13/2020 11:23:29 AM
 *  Author: Hector
 */ 

#define F_CPU 1000000UL //Tell util/delay.h clock speed of ATmega328P

#include <xc.h>
#include <avr/io.h> //For IO.
#include <avr/interrupt.h> //For interrupts.
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#define FRQ_200 60535
#define FRQ_230 61187
#define FRQ_275 61899
#define FRQ_320 62410
#define FRQ_365 62795
#define FRQ_410 63096
#define FRQ_455 63337
#define FRQ_500 63535
#define FRQ_545 63700
#define FRQ_590 63840
#define FRQ_635 63960
#define FRQ_680 64064
#define FRQ_725 64156
#define FRQ_770 64236
#define FRQ_800 64285

//Possible output frequencies.
int Frequency[16] = {0, FRQ_200, FRQ_230, FRQ_275, FRQ_320, FRQ_365,
						FRQ_410, FRQ_455, FRQ_500, FRQ_545, FRQ_590,
						FRQ_635, FRQ_680, FRQ_725, FRQ_770, FRQ_800};
int MOTOR_SPEED = -1; //Current speed of the motor.
int LastSpeed = 5; //Speed of motor before it was paused. Starts at default of 5.
int Time = 0; //Keeps track of time.
int CurrentFrequency = 0; //Default frequency, mute.

void TurnOffMotor()
{
	PORTD &= 0b11011111; //Set PD5 off.
	PORTB &= 0b11110001; //Set PB1-PB3 off.
	return;
}

void PulseMotor(int Speed)
{
	TurnOffMotor(); //Turn off all pins to motor.
	PORTB |= 0b00000010; //Turn on Yellow/PB1.
	PORTB |= 0b00000100; //Turn on Red/PB2.
	_delay_ms(Speed);
	TurnOffMotor(); //Turn off all pins to motor.
	PORTB |= 0b00000100; //Turn on Red/PB2.
	PORTD |= 0b00100000; //Turn on Black/PD5.
	_delay_ms(Speed);
	TurnOffMotor(); //Turn off all pins to motor.
	PORTD |= 0b00100000; //Turn on Black/PD5.
	PORTB |= 0b00001000; //Turn on Blue/PB3.
	_delay_ms(Speed);
	TurnOffMotor(); //Turn off all pins to motor.
	PORTB |= 0b00001000; //Turn on Blue/PB3.
	PORTB |= 0b00000010; //Turn on Yellow/PB1.
	_delay_ms(Speed);
	
	return;
}

void PulseHeart()
{
	if(PINC & (1 << PINC5))
	{
		PORTC &= 0b11011111; //Set PC5 to low.
	}
	else
	{
		PORTC |= 0b00100000; //Set PC5 to high.
	}
	return;
}

/*
	Sets 7-segment display to output given number V.
*/
int SetDisplay(int V)
{
	PORTB &= 0b00111111; //Turn off all LEDs.
	PORTD &= 0b11100000;
	switch(V)
	{
		case -1: //Paused state. Letter P
			PORTB |= 0b11000000; //Turn on g/center, f/topleft.
			PORTD |= 0b00011001; //Turn on a/top, b/topright, e/bottomleft.
			break;
		case 0:
			PORTB |= 0b10000000; //Turn on f/topleft.
			PORTD |= 0b00011111; //Turn on all.
			break;
		case 1:
			PORTD |= 0b00001100; //Turn on c/bottomright, b/topright.
			break;
		case 2:
			PORTB |= 0b01000000; //Turn on g/center.
			PORTD |= 0b00011011; //Turn on a/top, b/topright, d/bottom, e/bottomleft.
			break;
		case 3:
			PORTB |= 0b01000000; //Turn on g/center.
			PORTD |= 0b00011110; //Turn on a/top, b/topright, c/bottomright, d/bottom.
			break;
		case 4:
			PORTB |= 0b11000000; //Turn on g/center, f/topleft.
			PORTD |= 0b00001100; //Turn on b/topright, c/bottomright.
			break;
		case 5:
			PORTB |= 0b11000000; //Turn on g/center, f/topleft.
			PORTD |= 0b00010110; //Turn on a/top, c/bottomright, d/bottom.
			break;
		case 6:
			PORTB |= 0b11000000; //Turn on g/center, f/topleft.
			PORTD |= 0b00010111; //Turn on a/top, c/bottomright, d/bottom, e/bottomleft.
			break;
		case 7:
			PORTD |= 0b00011100; //Turn on a/top, c/bottomright, b/topright.
		break;
		case 8:
			PORTB |= 0b11000000; //Turn on all.
			PORTD |= 0b00011111; //Turn on all.
			break;
		case 9:
			PORTB |= 0b11000000; //Turn on all.
			PORTD |= 0b00011110; //Turn on all, except e/bottomleft.
			break;
		default:
			return 1; //Error, given invalid number.
			break;
	}
	return 0; //Success.
}

void CheckButtons()
{
	if(Time > 50)
	{
		if(PIND & (1 << PIND6))
		{
			if(MOTOR_SPEED == -1)
			{
				MOTOR_SPEED = LastSpeed;
				LastSpeed = -1;
			}
			else
			{
				LastSpeed = MOTOR_SPEED;
				MOTOR_SPEED = -1;
				CurrentFrequency = Frequency[0]; //Mute speaker on pause.
			}
			Time = 0;
			SetDisplay(MOTOR_SPEED);
			return;
		}
		if(MOTOR_SPEED != -1) //If not paused, allow change of speed.
		{
			if((PINB & (1 << PINB0)) && (MOTOR_SPEED > 0))
			{
				MOTOR_SPEED -= 1;
				Time = 0;
				SetDisplay(MOTOR_SPEED);
				return;
			}
			if((PIND & (1 << PIND7)) && (MOTOR_SPEED < 9))
			{
				MOTOR_SPEED += 1;
				Time = 0;
				SetDisplay(MOTOR_SPEED);
				return;
			}
		}
	}
	return;
}

//Plays the current frequency.
ISR(TIMER1_OVF_vect)
{
	if(MOTOR_SPEED != -1) //If we're not mute.
	{
		PORTB ^= (1 << PINB4);
		TCNT1 = CurrentFrequency;
	}
}

//Decodes data from disk and changes frequency.
ISR(PCINT1_vect)
{
	if(PINC & (1 << PINC4))
	{
		//SetDisplay(PINC & 0b00001111); //Uncomment to display frequency instead of speed.
		CurrentFrequency = Frequency[PINC & 0b00001111]; //Set current frequency to 4-bit sound code.
	}
}

int main(void)
{
	//Motor pulse pins.
	DDRB |= 0b00001110; //Set PB1-PB3 as outputs.
	DDRD |= (1 << PORTD5); //Set PD5 as output.
	TurnOffMotor(); //Set all port outputs to motor driver off.
	
	//Heartbeat pin.
	DDRC |= 0b00100000; //Set PC5 as output.
	PORTC &= 0b11011111; //Set PC5 to low.
	
	//7-Segment Display pins
	DDRB |= 0b11000000; //Set PB6-PB7 as outputs.
	DDRD |= 0b00011111; //Set PD0-PD4 as outputs.
	
	//Audio Output pin.
	DDRB |= 0b00010000; //Set PB4 as output.
	
	//Ticks for each frequency.
	// T_tick = 1 MHz = 0.008 ms.
	//1 | 1/200 = 5 ms | 5 ms / 0.001 ms = 5000 ticks.
	//2 | 1/230 = 4348 ticks.
	//3 | 1/275 = 3636 ticks.
	//4 | 1/320 = 3125 ticks.
	//5 | 1/365 = 2740 ticks
	//6 | 1/410 = 2439 ticks.
	//7 | 1/455 = 2198 ticks.
	//8 | 1/500 = 2000 ticks.
	//9 | 1/545 = 1835 ticks.
	//10 | 1/590 = 1695 ticks.
	//11 | 1/635 = 1575 ticks.
	//12 | 1/680 = 1471 ticks.
	//13 | 1/725 = 1379 ticks.
	//14 | 1/770 = 1299 ticks.
	//15 | 1/800 = 1.25 ms = 1250 ticks.
	//Highest Count = 65535. Time = HC - ticks.
	
	//Set up audio timer.
	sei(); //Turn on global interrupts.
	TCCR1B = 0b00000001; //Turn on timer. No scaling.
	CurrentFrequency = Frequency[0]; //Speaker starts in off state.
	TCNT1 = CurrentFrequency; //Set timer.
	TIMSK1 |= (1 << TOIE1); //Enable timer interrupt.
	
	//Set up disk reader.
	DDRC &= 0b11100000; //Set up PC0-PC4 as inputs.
	PCICR |= (1 << PCIE1); // enable the pin change interrupt.
	PCMSK1 |= (1 << PCINT12); //Enable interrupt from verification bit.
	
	SetDisplay(MOTOR_SPEED);
	int cad = 0;
	while(1)
	{
		/* //Uncomment if you want speaker to play all sounds in a row.
		++cad;
		if(cad > 15)
		{
			cad = 0;
		}
		else
		{
			CurrentFrequency = Frequency[cad];
		}*/
		PulseHeart();
		CheckButtons();
		if(MOTOR_SPEED >= 0)
		{
			PulseMotor((10 - MOTOR_SPEED) * 15);
			Time += (10 - MOTOR_SPEED) * 15;
		}
		else
		{
			_delay_ms((10 - MOTOR_SPEED) * 15);
			Time += (10 - MOTOR_SPEED) * 15;
		}
	}
}