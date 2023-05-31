/********************************
* AT45DB641E memory definitions *
*********************************/

// Constants

#define AT45DB641E_Manufacturer	0x9f
#define AT45DB641E_Status	0xd7

#define SEQ_START		1
#define SEQ_STOP		2

// Prototypes 

void AT45DB641E_page_size(volatile uint8_t *port,int cs,int size);
void AT45DB641E_cmd(volatile uint8_t *port,int cs,int cmd,uint8_t *result,int len);
unsigned char AT45DB641E_busy(volatile uint8_t *port,int cs);
void AT45DB641E_write_buffer(volatile uint8_t *port,int cs,unsigned char *data,int len,unsigned char synfin);
void AT45DB641E_write_page(volatile uint8_t *port,int cs,int address);
void AT45DB641E_read_page(volatile uint8_t *port,int cs,int address,unsigned char *data,int len,unsigned char synfin);

