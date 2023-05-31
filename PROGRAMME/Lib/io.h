/** Definitions for I/O **/

// Structures 

struct ports_s {
  int nb;
  volatile uint8_t **dir;
  volatile uint8_t **write;
  volatile uint8_t **read;
  };

// Prototypes

int ports_nb(void);
void inputs_init(unsigned char inputs[],unsigned char pullups[]);
void outputs_init(unsigned char outputs[]);
void inputs_get(unsigned char values[]);
unsigned char input_active(unsigned char input[],unsigned char state[]);
void outputs_set(unsigned char values[]);
void output_set(unsigned char **list,int num);
void output_unset(unsigned char **list,int num);
void output_toogle(unsigned char **list,int num);
