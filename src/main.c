/*
 * main.c
 *
 * Created: 01.02.2017 23:36:41
 *  Author: Torben
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "segmentDriver.h"
#include <stdio.h>
#include "uart.h"
#define UART_BAUD_RATE 115200

void process_byte(uint8_t byte);
void parse_data(uint8_t* data, uint8_t length);

typedef enum {
	wait_initialisation,
	wait_id,
	ready,
    receiving
} com_status_t;

typedef enum {
	id_assign = 0x00,
	data_transmit = 0x01
} commandwords;

typedef enum {
	character_update = 0x01,
	brightness_update = 0x02
	
} datatypes;

int main(void)
{
	init();
	

	

	_delay_ms(500);
	//sprintf(charbuffer[0],"ABCDEFGHIJ");
	//sprintf(charbuffer[1],"KLMNOPQRST");
	update_framebuffer();
	
	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
	set_brightness(100);

	blank();
    sprintf(charbuffer[0],"Waiting 0O");
	sprintf(charbuffer[1],"for Init");
	update_framebuffer();

    while(1)
    {

		
	
		
		unsigned int c = uart_getc();
		if (! ( c & UART_NO_DATA) )
		{	
			process_byte(c);
		}			
    }
}

void process_byte(uint8_t byte)
{
	static com_status_t com_status = wait_initialisation;	
	static uint8_t receiving_counter=0;
	static uint8_t id;
	static uint8_t databuffer[256];
	static uint8_t message_id;
	static uint8_t message_length;
	
	
	switch(com_status)
	{
		case wait_initialisation:
			if(byte==id_assign)
			{

				com_status=wait_id;
			}				
		break;
		case wait_id:
			blank();
			sprintf(charbuffer[0],"Init");
			sprintf(charbuffer[1],"ID %d", byte);
			update_framebuffer();
			uart_putc(id_assign);
			id=byte;
			uart_putc(byte+1);
			com_status=ready;
		break;
		case ready: 
			if(byte==id_assign)
				com_status = wait_id;
			if(byte==data_transmit)
			{
				com_status = receiving;
				receiving_counter=0;
			}				
		break;
		case receiving:
			if(receiving_counter==0)
			{
				message_id=byte;
			}
			if(receiving_counter==1)
			{
				message_length=byte;
			}
			if(receiving_counter>1)
			{
				databuffer[receiving_counter-2]=byte;
			}
			receiving_counter++;
			
			if(receiving_counter >= 2+message_length)
			{
				if(id==message_id)
				{
					uart_putc(data_transmit);
					uart_putc(id);
					uart_putc(1);
					uart_putc(11);
					
					parse_data(databuffer,message_length);
				}
				else
				{
					uart_putc(data_transmit);
					uart_putc(message_id);
					uart_putc(message_length);
					for(uint8_t i=0; i < message_length; i++)
					{
						uart_putc(databuffer[i]);
					}
				}
				com_status=ready;
			}
			
		break;

	}
	
}

void parse_data(uint8_t* data, uint8_t length)
{
	switch (data[0])
	{
		case character_update:
			for(uint8_t i=0; i < length-1; i++)
				charbuffer[ (i>9) ? 1:0 ] [i%10]  = data[i+1];
				
			update_framebuffer();	
		break;
		case brightness_update: 
			set_brightness(data[1]);
		break;
	}

}		
