#include <stdio.h>
#include <avr\io.h>
#include <avr\interrupt.h>
#include <util\delay.h>

void Config32MHzClock(void);

void SpiInit(void);
char SpiRead();
void SpiWrite(char data);
char SpiWriteRead(char data);


int main(void)
{
  char i=0;
  char result=0;
  char cmdPayload[6]={0x40,0,0,0,0,0x95};
volatile  char respPayload[3]={0,0,0};

  Config32MHzClock();

  PORTB.DIR |= (1<<4); // accel range select
  PORTB.OUT &= ~(1<<4);
  SpiInit();


// set MOSI, CS hi
PORTD.OUT |= (1<<5) | (1<<4);

// write 80 clocks to put in spi mode
SpiWrite(0xff);
SpiWrite(0xff);
SpiWrite(0xff);
SpiWrite(0xff);
SpiWrite(0xff);
SpiWrite(0xff);
SpiWrite(0xff);
SpiWrite(0xff);
SpiWrite(0xff);
SpiWrite(0xff);
//while(1)
//{
  PORTD.OUT &= ~(1<<4);  // lower ss line indicating start of transfer


  for(i=0;i<6;i++)
	  SpiWrite(cmdPayload[i]);

  _delay_ms(1);

  for(i=0;i<3;i++)
      respPayload[i]=SpiRead();

  PORTD.OUT |= (1<<4); // release CS

  _delay_ms(10);

  cmdPayload[0]=0x41; // CMD1 (init)
  respPayload[0]=0;
  respPayload[1]=0;
  respPayload[2]=0;

  PORTD.OUT &= ~(1<<4);  // lower ss line indicating start of transfer


  for(i=0;i<6;i++)
	  SpiWrite(cmdPayload[i]);

  _delay_ms(1);

  for(i=0;i<3;i++)
      respPayload[i]=SpiRead();

// try command 

};


// initialize SPI interface on PORTD:4-7
// PORTD:4 - SS   (active low)
// PORTD:5 - MOSI
// PORTD:6 - MISO
// PORTD:7 - SCK
void SpiInit(void)
{
  PORTD.DIR = 0xB0;  // configure MOSI, SS, CLK as outputs on PORTD
  // enable SPI master mode, CLK/64 (@32MHz=>500KHz)
  SPID.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV128_gc;
};

void SpiWrite(char data)
{
//  PORTD.OUT &= ~(1<<4);  // lower ss line indicating start of transfer
  SPID.DATA = data;      // initiate write
  // wait for transfer complete
  while(!(SPID.STATUS & (1<<7)));
//  PORTD.OUT |= (1<<4);   // raise ss line indicating end of transfer
};

char SpiRead(void)
{

  // write 0xff and read back results clocked into data buffer
  SpiWrite(0xff);
  return SPID.DATA;
};

char SpiWriteRead(char data)
{
  SpiWrite(data);
  return SPID.DATA;
};


void Config32MHzClock(void)
{
  CCP = CCP_IOREG_gc; //Security Signature to modify clock 
  // initialize clock source to be 32MHz internal oscillator (no PLL)
  OSC.CTRL = OSC_RC32MEN_bm; // enable internal 32MHz oscillator
  while(!(OSC.STATUS & OSC_RC32MRDY_bm)); // wait for oscillator ready
  CCP = CCP_IOREG_gc; //Security Signature to modify clock 
  CLK.CTRL = 0x01; //select sysclock 32MHz osc
};

