/*
 * segmentDriver.h
 *
 * Created: 01.02.2017 23:49:18
 *  Author: Torben
 */ 


#ifndef SEGMENTDRIVER_H_
#define SEGMENTDRIVER_H_

#define RESET_595 0
#define OE_595 1
#define LATCH_595 2
#define DATA_595 3
#define SCK_595 5
#define PORT_595 PORTB
#define DDR_595 DDRB

#define ANODE_DDR DDRC
#define ANODE_PORT PORTC

typedef enum {
	up,
	down
} char_pos_t;

void init();
void update_framebuffer();
void set_brightness(uint8_t percent);
void blank(void);
extern char charbuffer[2][11];


#endif /* SEGMENTDRIVER_H_ */