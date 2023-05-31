/*****************************
* AT45DB641E memory handling *
*****************************/

#include <stdio.h>
#include <stdint.h>

#include "spi.h"
#include "AT45DB641E.h"

// AT45DB641E page mode
void AT45DB641E_page_size(volatile uint8_t *port,int cs,int size){
*port &= ~(1<<cs);
if(size!=256 && size!=264) return;
spi_exch(0x3D);
spi_exch(0x2A);
spi_exch(0x80);
if(size==256) spi_exch(0xA6); else spi_exch(0xA7);
*port |= (1<<cs);
}

// AT45DB641E command

void AT45DB641E_cmd(volatile uint8_t *port,int cs,int cmd,uint8_t *result,int len){
*port &= ~(1<<cs);
spi_exch(cmd);
int i;
for(i=0;i<len;i++) result[i]=spi_exch(0);
*port |= (1<<cs);
}

// AT45DB641E test busy

unsigned char AT45DB641E_busy(volatile uint8_t *port,int cs){
unsigned char status[2];
AT45DB641E_cmd(port,cs,AT45DB641E_Status,status,2);
return status[0]&0x80?0:1;
}

// AT45DB641E write into first buffer

void AT45DB641E_write_buffer(volatile uint8_t *port,int cs,unsigned char *data,int len,unsigned char synfin){
int i;
if(synfin&SEQ_START){
  *port &= ~(1<<cs);
  spi_exch(0x84);
  for(i=0;i<3;i++) spi_exch(0x00);
  }
if(data!=NULL && len>0)
  for(i=0;i<len;i++) spi_exch(data[i]);
if(synfin&SEQ_STOP){
  *port |= (1<<cs);
  while(AT45DB641E_busy(port,cs));
  }
}

// AT45DB641E write first buffer into a flash page

void AT45DB641E_write_page(volatile uint8_t *port,int cs,int address){
*port &= ~(1<<cs);
spi_exch(0x83);
spi_exch((address>>8)&0x7f);
spi_exch(address&0xff);
spi_exch(0x00);
*port |= (1<<cs);
while(AT45DB641E_busy(port,cs));
}

// AT45DB641E read

void AT45DB641E_read_page(volatile uint8_t *port,int cs,int address,unsigned char *data,int len,unsigned char synfin){
int i;
if(synfin&SEQ_START){
  *port &= ~(1<<cs);
  spi_exch(0x1B);
  spi_exch((address>>8)&0x7f);
  spi_exch(address&0xff);
  for(i=0;i<3;i++) spi_exch(0x00);
  }
if(data!=NULL && len>0)
  for(i=0;i<len;i++) data[i]=spi_exch(0x00);
if(synfin&SEQ_STOP){
  *port |= (1<<cs);
   while(AT45DB641E_busy(port,cs));
  }
}
