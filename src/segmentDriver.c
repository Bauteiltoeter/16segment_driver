/*
 * bigDisplay.c
 *
 * Created: 29.10.2016 17:54:19
 *  Author: Torben
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/delay.h>
#define RESET_595 0
#define OE_595 1
#define LATCH_595 2
#define DATA_595 3
#define SCK_595 4
#define PORT_595 PORTB
#define DDR_595 DDRB

#define ANODE_DDR DDRC
#define ANODE_PORT PORTC

void init();
void set_shiftregisters(uint32_t value);
void send_byte(uint8_t value);
void set_anode(uint8_t anode);

int main(void)
{
	init();
	
	DDRD |=(1<<6) | (1<<7);
	PORTD	|=(1<<6) | (1<<7);
	
	set_anode(1);
	set_shiftregisters(0xAAAAAAAA);
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}

void init()
{
	DDR_595 |= (1<<RESET_595) | (1<<OE_595) | (1<<LATCH_595) | (1<<DATA_595) | (1<<SCK_595);
	
	
	PORT_595 |= (1<<RESET_595) | (1<<LATCH_595);
	PORT_595 &=~( (1<<OE_595)  | (1<<SCK_595));

	ANODE_DDR |= (0x0F);
}

void set_anode(uint8_t anode)
{
	anode = anode & 0x0F;
	ANODE_PORT = (ANODE_PORT & 0xF0) | anode;
	
}

void set_shiftregisters(uint32_t value)
{
	send_byte(value>>24);
	send_byte(value>>16);
	send_byte(value>>8);
	send_byte(value);
	
	
	
	PORT_595 &=~(1<<LATCH_595);
	_delay_ms(1);
	PORT_595 |= (1<<LATCH_595);
	_delay_ms(1);
}

void send_byte(uint8_t value)
{
	uint8_t mask = 0x80;
	
	for(uint8_t i = 0; i < 8; i++)
	{
		mask = mask >> 1;
		if(value & mask == 0)
			PORT_595 &=~mask;
		else
			PORT_595 |=mask;
		
		
		PORT_595 |= (1<<SCK_595);
		_delay_ms(1);
		PORT_595 &=~(1<<SCK_595);
		_delay_ms(1);
	}
	
	
}





