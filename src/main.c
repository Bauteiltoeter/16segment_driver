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
uint8_t calc_checksum(uint8_t* data, uint8_t length);

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
    sprintf(charbuffer[0],"Waiting ");
	sprintf(charbuffer[1],"for Init");
	update_framebuffer();

    int i=0;
    while(1)
    {

		
	
		
		unsigned int c = uart_getc();


		if ( c & UART_NO_DATA )
        {
            /* 
             * no data available from UART 
             */
        }
        else
        {
            /*
             * new data available from UART
             * check for Frame or Overrun error
             */
            if ( c & UART_FRAME_ERROR )
            {
                /* Framing Error detected, i.e no stop bit detected */
				sprintf(charbuffer[0],"Frameerror");
                update_framebuffer();
            }
            if ( c & UART_OVERRUN_ERROR )
            {
                /* 
                 * Overrun, a character already present in the UART UDR register was 
                 * not read by the interrupt handler before the next character arrived,
                 * one or more received characters have been dropped
                 */
                sprintf(charbuffer[0],"Overrun");
                update_framebuffer();
            }
            if ( c & UART_BUFFER_OVERFLOW )
            {
                /* 
                 * We are not reading the receive buffer fast enough,
                 * one or more received character have been dropped 
                 */
                uart_puts_P("Buffer overflow error: ");
				blank();
				sprintf(charbuffer[0],"Boverflow");
				
                update_framebuffer();
            }
            /* 
             * send received character back
             */
            //uart_putc( (unsigned char)c );


			

			process_byte(c);
        }









		
		/*if (! ( c & UART_NO_DATA) )
		{	
            if(c & UART_FRAME_ERROR)
            {
                blank();

                sprintf(charbuffer[0],"Frameerror");
                update_framebuffer();
            } else if(c & UART_PARITY_ERROR) {
                blank();
                sprintf(charbuffer[0],"Parity");
                update_framebuffer();
            } else {
                i++;
                process_byte(c);
                 blank();

                sprintf(charbuffer[0],"Byte %d",i);

                sprintf(charbuffer[1],"%d      ", c);
                update_framebuffer();
            }



        }
        else if( c & UART_FRAME_ERROR)
        {

        }
        else if( c & UART_PARITY_ERROR)
        {

        }*/
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
					/*uart_putc(data_transmit);
					uart_putc(id);
					uart_putc(1);
                    if(databuffer[message_length-1]==calc_checksum(databuffer,message_length-1))
                    {*/
                    //    uart_putc(0);
                        parse_data(databuffer,message_length);
                    /*}
                    else
                    {
                        uart_putc(1);
                    }*/
					
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
            for(uint8_t i=0; i < length-2; i++)
				charbuffer[ (i>9) ? 1:0 ] [i%10]  = data[i+1];
				
			update_framebuffer();	
		break;
		case brightness_update: 
			set_brightness(data[1]);
		break;
	}

}		

uint8_t calc_checksum(uint8_t* data, uint8_t length)
{
    uint8_t temp=0;
    for(uint16_t i=0; i < length; i++)
    {
        temp^=data[i];
    }
    return temp;
}
