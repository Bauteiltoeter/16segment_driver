/*
 * bigDisplay.c
 *
 * Created: 29.10.2016 17:54:19
 *  Author: Torben
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "segmentDriver.h"


#define GET_BIT(val, bit) ((val)&(1<<(bit)))
#define SET_BIT(val, bit) ((val)|=(1<<(bit)))



void set_shiftregisters(uint32_t value);
void send_byte(uint8_t value);
void set_anode(uint8_t anode);
uint16_t mangle_segments(uint16_t character, char_pos_t char_pos);
uint8_t ascii_to_index(char ascii);
uint32_t framebuffer[10];
char charbuffer[2][11];


uint16_t charset[256] = {
	//utsrpnmkhgfedcba
	0b0100010011111111,		//0
	0b0000010000001100,		//1
	0b1000100001110111,		//2
	0b0000100000111111,		//3
	0b1000100010001100,		//4
	0b1000100010111011,		//5
	0b1000100011111000,		//6
	0b0000000000001111,		//7
	0b1000100011111111,		//8
	0b1000100010111111,		//9
	0b1000100011001111,		//a
	0b0010101000111111,		//b		
	0b0000000011110011,		//c
	0b0010001000111111,		//d
	0b1000000011110011,		//e
	0b1000000011000011,		//f
	0b0000100011111011,		//g
	0b1000100011001100,		//h
	0b0010001000110011,		//i
	0b0000000001111100,		//j
	0b1001010011000000,		//k
	0b0000000011110000,		//l
	0b0000010111001100,		//m
	0b0001000111001100,		//n
	0b0000000011111111,		//o
	0b1000100011000111,		//p
	0b0001000011111111,		//q
	0b1001100011000111,		//r
	0b1000100010111011,		//s
	0b0010001000000011,		//t
	0b0000000011111100,		//u
	0b0100010011000000,		//v
	0b0101000011001100,		//w
	0b0101010100000000,		//x
	0b0010010100000000,		//y
	0b0100010000110011,		//z
	0b0000000000000000,		//Space   ASCII-Table from ' ' to '/'
	0b0000000000000000,		//!
	0b0000001000000100,		//"
	0b1010101000111100,		//#
	0b1010101010111011,		//$
	0b1110111010011001,		//%
	0b0000000000000000,		//&
	0b0000000000000000,		//bank
	0b0010001000010010,		//(
	0b0010001000100001,		//)
	0b1111111100000000,		//*
	0b1010101000000000,		//+
	0b0010000000000000,		//,
	0b1000100000000000,		//-
	0b0000000000010000,		//.
	0b0100010000000000,		// /
	0b0000101000000110,		// °
	0b1111111111111111
};

int i=0;



void init()
{
	DDR_595 |= (1<<RESET_595) | (1<<OE_595) | (1<<LATCH_595) | (1<<DATA_595) | (1<<SCK_595);
	PORT_595 |= (1<<RESET_595) | (1<<LATCH_595);
	PORT_595 &=~( (1<<OE_595)  | (1<<SCK_595));
	ANODE_DDR |= (0x0F);
	
	//100µs timer interrupt for multiplexing
	TCCR0A = (1<<WGM01); //CTC mode
	TCCR0B = (1<<CS01) | (1<<CS00); //Prescaler 64
	TIMSK0 = (1<<OCIE0A); //
	OCR0A=25;
	
	//PWM generation for dim
	
	TCCR1A = (1<<COM1A1) | (1<<COM1A0); //Set OC1A on compare Match
	TCCR1A|= (1<<WGM11)  | (1<<WGM10); //Fast PWM Mode
	TCCR1B = (1<<CS10);
	sei();
}

void update_framebuffer()
{
	for(uint8_t i=0; i < 10; i++)
	{
		framebuffer[i] = (uint32_t)mangle_segments(charset[ascii_to_index(charbuffer[0][i])],up)<<16 | mangle_segments(charset[ascii_to_index(charbuffer[1][i])],down);
	}
}

void set_anode(uint8_t anode)
{
	anode = anode & 0x0F;
	ANODE_PORT = (ANODE_PORT & 0xF0) | anode;
}

uint8_t ascii_to_index(char ascii)
{
	if(ascii >= '0' && ascii <= '9')
	{
		return ascii-'0';
	}
	if(ascii>='a' && ascii <= 'z')
	{
		return ascii-'a' + 10; //get index of a and add 10 because of numbers
	}
	if(ascii>='A' && ascii <='Z')
	{
		return ascii-'A' + 10; //get index of a and add 10 because of numbers	
	}
	if(ascii>=' ' && ascii <='/')
	{
		return ascii-' ' + 36;
	}
	
	if(ascii=='\0')
		return ascii_to_index(' ');
		
	if(ascii=='°')
		return 52;
		
	if(ascii=='~')
		return 53;
}

uint16_t mangle_segments(uint16_t character, char_pos_t char_pos)
{
	uint16_t temp=0;
	if(char_pos==down)
	{
		if(GET_BIT(character,0)) SET_BIT(temp,14);	//a
		if(GET_BIT(character,1)) SET_BIT(temp,15);	//b
		if(GET_BIT(character,2)) SET_BIT(temp,11);	//c
		if(GET_BIT(character,3)) SET_BIT(temp,5);	//d
		if(GET_BIT(character,4)) SET_BIT(temp,3);	//e
		if(GET_BIT(character,5)) SET_BIT(temp,1);	//f
		if(GET_BIT(character,6)) SET_BIT(temp,0);	//g
		if(GET_BIT(character,7)) SET_BIT(temp,8);	//h
		if(GET_BIT(character,8)) SET_BIT(temp,10);	//k
		if(GET_BIT(character,9)) SET_BIT(temp,12);	//m
		if(GET_BIT(character,10)) SET_BIT(temp,13);	//n
		if(GET_BIT(character,11)) SET_BIT(temp,9);	//p
		if(GET_BIT(character,12)) SET_BIT(temp,7);	//r
		if(GET_BIT(character,13)) SET_BIT(temp,4);	//s
		if(GET_BIT(character,14)) SET_BIT(temp,2);	//t
		if(GET_BIT(character,15)) SET_BIT(temp,6);	//u
	}
	else
	{
		if(GET_BIT(character,0)) SET_BIT(temp,15);	//a
		if(GET_BIT(character,1)) SET_BIT(temp,14);	//b
		if(GET_BIT(character,2)) SET_BIT(temp,10);	//c
		if(GET_BIT(character,3)) SET_BIT(temp,4);	//d
		if(GET_BIT(character,4)) SET_BIT(temp,0);	//e
		if(GET_BIT(character,5)) SET_BIT(temp,2);	//f
		if(GET_BIT(character,6)) SET_BIT(temp,1);	//g
		if(GET_BIT(character,7)) SET_BIT(temp,9);	//h
		if(GET_BIT(character,8)) SET_BIT(temp,11);	//k
		if(GET_BIT(character,9)) SET_BIT(temp,13);	//m
		if(GET_BIT(character,10)) SET_BIT(temp,12);	//n
		if(GET_BIT(character,11)) SET_BIT(temp,8);	//p
		if(GET_BIT(character,12)) SET_BIT(temp,6);	//r
		if(GET_BIT(character,13)) SET_BIT(temp,5);	//s
		if(GET_BIT(character,14)) SET_BIT(temp,3);	//t
		if(GET_BIT(character,15)) SET_BIT(temp,7);	//u
	}		
	
	return temp;
}

void set_shiftregisters(uint32_t value)
{
	send_byte(value>>24);
	send_byte(value>>16);
	send_byte(value>>8);
	send_byte(value);
}

void send_byte(uint8_t value)
{
	uint8_t mask = 0x80;
	
	for(uint8_t i = 0; i < 8; i++)
	{
		
		if( (value & mask) == 0)
			PORT_595 &=~(1<<DATA_595);
		else
			PORT_595 |=(1<<DATA_595);
		
		asm volatile ("nop");
		PORT_595 |= (1<<SCK_595);
		asm volatile ("nop");
		PORT_595 &=~(1<<SCK_595);
		
		
		mask = mask >> 1;
	}
	
	
}

void blank(void)
{
	for(uint8_t x=0; x < 10; x++)
		for(uint8_t y=0; y < 2; y++)
			charbuffer[y][x]=' ';
			
			update_framebuffer();
}

void set_brightness(uint8_t percent)
{
	OCR1A = percent*10;
}

ISR(TIMER0_COMPA_vect)
{
	
	set_anode(11);
	
	set_shiftregisters(framebuffer[i]);
	PORT_595 &=~(1<<LATCH_595);
	asm volatile ("nop");
	PORT_595 |= (1<<LATCH_595);
	asm volatile ("nop");
	set_anode(i);
	i++;
	i=i%10;
}




