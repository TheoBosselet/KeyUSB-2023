/** Functions for I/O **/

#include <stdint.h>
#include <avr/io.h>

#include "io.h"

// Some constants

#define ARRAY_NULL {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
#define ARRAY_ZERO {0,0,0,0,0,0,0,0,0,0}

// Port structure declaration

volatile uint8_t *ddr[]={&DDRB,&DDRC,&DDRD};
volatile uint8_t *write[]={&PORTB,&PORTC,&PORTD};
volatile uint8_t *read[]={&PINB,&PINC,&PIND};

static struct ports_s ports={3,ddr,write,read};

// Get port number 
int ports_nb(void){ return ports.nb; }

// Input initialization

static int imask[]=ARRAY_NULL;
static int pullup[]=ARRAY_NULL;

void inputs_init(unsigned char inputs[],unsigned char pmask[]){
int nb=ports.nb;
int i;
for(i=0;i<nb;i++){ *ports.dir[i] &= ~inputs[i]; *ports.write[i] |= pmask[i]; }
for(i=0;i<nb;i++){ imask[i]=inputs[i]; pullup[i]=pmask[i]; }
}

// Output initialization

static int omask[]=ARRAY_NULL;

void outputs_init(unsigned char outputs[]){
int nb=ports.nb;
int i;
for(i=0;i<nb;i++) *ports.dir[i] |= outputs[i];
for(i=0;i<nb;i++) omask[i]=outputs[i];
}

// Get inputs

void inputs_get(unsigned char values[]){
int nb=ports.nb;
int i;
for(i=0;i<nb;i++) values[i]=0;
for(i=0;i<nb;i++)
  if(imask[i]>=0){ values[i]=(*ports.read[i]&imask[i]); values[i] ^= pullup[i]; }
}

unsigned char input_active(unsigned char input[],unsigned char state[]){
int nb=ports.nb;
int i;
int result=0;
for(i=0;i<nb;i++) result |= input[i]&state[i];
return result==0?0:1;
}

// Set output

void outputs_set(unsigned char values[]){
int nb=ports.nb;
int i;
for(i=0;i<nb;i++)
  if(omask[i]>=0){ *ports.write[i] |= (values[i]&omask[i]); *ports.write[i] &= (values[i]|~omask[i]); }
}

// Handle output list

unsigned char ostates[]=ARRAY_ZERO;

void output_set(unsigned char **list,int num){
int nb=ports.nb;
int i;
for(i=0;i<nb;i++) ostates[i] |= list[num][i];
outputs_set(ostates);
}

void output_unset(unsigned char **list,int num){
int nb=ports.nb;
int i;
for(i=0;i<nb;i++) ostates[i] &= ~list[num][i];
outputs_set(ostates);
}

void output_toogle(unsigned char **list,int num){
int nb=ports.nb;
int i;
for(i=0;i<nb;i++) ostates[i] ^= list[num][i];
outputs_set(ostates);
}
