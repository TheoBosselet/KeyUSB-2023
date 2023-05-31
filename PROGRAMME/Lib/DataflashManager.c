/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Functions to manage the physical Dataflash media, including reading and writing of
 *  blocks of data. These functions are called by the SCSI layer when data must be stored
 *  or retrieved to/from the physical storage media. If a different media is used (such
 *  as a SD card or EEPROM), functions similar to these will need to be generated.
 */

#define  INCLUDE_FROM_DATAFLASHMANAGER_C
#include "DataflashManager.h"
/** Initialise the Dataflash memory. */
#include "memoir.h"
#include "spi.h"
#include "AT45DB641E.h"

void DataflashManager_Initialisation(void) // a nous de definir localisation memoire et les inisialiser a 256
{
MEM1_DDR |= (1<<MEM1_PIN);
MEM2_DDR |= (1<<MEM2_PIN);
MEM3_DDR |= (1<<MEM3_PIN);
MEM4_DDR |= (1<<MEM4_PIN);

MEM1_PORT |= (1<<MEM1_PIN);
MEM2_PORT |= (1<<MEM2_PIN);
MEM3_PORT |= (1<<MEM3_PIN);
MEM4_PORT |= (1<<MEM4_PIN);

MEM_RESET_DDR |= (1<<MEM_RESET_PIN);
MEM_RESET_PORT |= (1<<MEM_RESET_PIN);
spi_init();

AT45DB641E_page_size(&MEM1_PORT,MEM1_PIN,256);
AT45DB641E_page_size(&MEM2_PORT,MEM2_PIN,256);
AT45DB641E_page_size(&MEM3_PORT,MEM3_PIN,256);
AT45DB641E_page_size(&MEM4_PORT,MEM4_PIN,256);
}

/** Writes blocks (OS blocks, not Dataflash pages) to the storage medium, the board Dataflash IC(s), from
 *  the pre-selected data OUT endpoint. This routine reads in OS sized blocks from the endpoint and writes
 *  them to the Dataflash in Dataflash page sized blocks.
 *
 *  \param[in] MSInterfaceInfo  Pointer to a structure containing a Mass Storage Class configuration and state
 *  \param[in] BlockAddress  Data block starting address for the write sequence
 *  \param[in] TotalBlocks   Number of blocks of data to write
 */
void DataflashManager_WriteBlocks(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo,
                                  const uint32_t BlockAddress,
                                  uint16_t TotalBlocks)
{
	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady()) return;

	int i,j;
	for(i=0;i<TotalBlocks;i++){ 
		for(j=0;j<VIRTUAL_MEMORY_BLOCK_SIZE;j++){
			/* Check if the endpoint is currently empty */
			if (!(Endpoint_IsReadWriteAllowed()))
			{
				/* Clear the current endpoint bank */
				Endpoint_ClearOUT();
				/* Wait until the host has sent another packet */
				if (Endpoint_WaitUntilReady()) return;
			}
			/* Read one byte from USB host */
			uint8_t byte=Endpoint_Read_8();
			/* Store byte in memory at address BlockAddress+i*VIRTUAL_MEMORY_BLOCK_SIZE+j */
			//// A NOUS DE FAIRE // i paire mem 1 et 2 et impaire mem 3 4 ducoup on ecrit sur 1 et 2 si y a que 512 / ici on ecrit et danss la prochaine fonction on va lire
			if ((BlockAddress+i) % 2 == 0){
                // Memoire 1
                if(VIRTUAL_MEMORY_BLOCK_SIZE/2 > j ){
                    if (j==0){ AT45DB641E_write_buffer(&MEM1_PORT,MEM1_PIN,&byte,1,SEQ_START);}
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2-1){  
                        AT45DB641E_write_buffer(&MEM1_PORT,MEM1_PIN,&byte,1,SEQ_STOP); 
                        AT45DB641E_write_page(&MEM1_PORT,MEM1_PIN,(BlockAddress+i)/2);
                    }
                    else { AT45DB641E_write_buffer(&MEM1_PORT,MEM1_PIN,&byte,1,0);}
                }
                // Memoire 2 on commence a ecrire a 256
                else if (j < VIRTUAL_MEMORY_BLOCK_SIZE) {
                    if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2){ AT45DB641E_write_buffer(&MEM2_PORT,MEM2_PIN,&byte,1,SEQ_START);}
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE-1){  
                        AT45DB641E_write_buffer(&MEM2_PORT,MEM2_PIN,&byte,1,SEQ_STOP); 
                        AT45DB641E_write_page(&MEM2_PORT,MEM2_PIN,(BlockAddress+i)/2);
                        
                    }
                    else { AT45DB641E_write_buffer(&MEM2_PORT,MEM2_PIN,&byte,1,0);}
                }
            }
			else {
                // Memoire 3
                if(VIRTUAL_MEMORY_BLOCK_SIZE/2 > j ){
                    if (j==0){ AT45DB641E_write_buffer(&MEM3_PORT,MEM3_PIN,&byte,1,SEQ_START);}
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2 -1){  
                        AT45DB641E_write_buffer(&MEM3_PORT,MEM3_PIN,&byte,1,SEQ_STOP); 
                        AT45DB641E_write_page(&MEM3_PORT,MEM3_PIN,(BlockAddress+i-1)/2);
                        
                    }
                    else { AT45DB641E_write_buffer(&MEM3_PORT,MEM3_PIN,&byte,1,0);}
                }
                // Memoire 4 on commence a ecrire a 256
                else if (j < VIRTUAL_MEMORY_BLOCK_SIZE) {
                    if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2){ AT45DB641E_write_buffer(&MEM4_PORT,MEM4_PIN,&byte,1,SEQ_START);}
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE-1){  
                        AT45DB641E_write_buffer(&MEM4_PORT,MEM4_PIN,&byte,1,SEQ_STOP); 
                        AT45DB641E_write_page(&MEM4_PORT,MEM4_PIN,(BlockAddress+i-1)/2);
                        
                    }
                    else { AT45DB641E_write_buffer(&MEM4_PORT,MEM4_PIN,&byte,1,0);}
                
                }
            }
			if (MSInterfaceInfo->State.IsMassStoreReset) return;
		}

	}
	/* If the endpoint is empty, clear it ready for the next packet from the host */
	if (!(Endpoint_IsReadWriteAllowed())) Endpoint_ClearOUT();
}


/////// MOI
void DataflashManager_Erase(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo,
                                 const uint32_t BlockAddress,
                                 uint16_t TotalBlocks)
{
    int j;
    //Memoire1
    for(j=0;j<VIRTUAL_MEMORY_BLOCK_SIZE/2;j++){
                    if (j==0){ AT45DB641E_write_buffer(&MEM1_PORT,MEM1_PIN,0x00,1,SEQ_START);}
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2-1){  
                        AT45DB641E_write_buffer(&MEM1_PORT,MEM1_PIN,0x00,1,SEQ_STOP);
                    }
                    else { AT45DB641E_write_buffer(&MEM1_PORT,MEM1_PIN,0x00,1,0);}
                }
            
    for(j=0;j<16;j++){
        AT45DB641E_write_page(&MEM1_PORT,MEM1_PIN,j);}

    
    //Memoire2
    for(j=0;j<VIRTUAL_MEMORY_BLOCK_SIZE/2;j++){
                    if (j==0){ AT45DB641E_write_buffer(&MEM2_PORT,MEM2_PIN,0x00,1,SEQ_START);}
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2-1){  
                        AT45DB641E_write_buffer(&MEM2_PORT,MEM2_PIN,0x00,1,SEQ_STOP);
                    }
                    else { AT45DB641E_write_buffer(&MEM2_PORT,MEM2_PIN,0x00,1,0);}
                }
            
    for(j=0;j<16;j++){
        AT45DB641E_write_page(&MEM2_PORT,MEM2_PIN,j);}
    //Memoire3
    for(j=0;j<VIRTUAL_MEMORY_BLOCK_SIZE/2;j++){
                    if (j==0){ AT45DB641E_write_buffer(&MEM3_PORT,MEM3_PIN,0x00,1,SEQ_START);}
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2-1){  
                        AT45DB641E_write_buffer(&MEM3_PORT,MEM3_PIN,0x00,1,SEQ_STOP);
                    }
                    else { AT45DB641E_write_buffer(&MEM3_PORT,MEM3_PIN,0x00,1,0);}
                }
             
    for(j=0;j<16;j++){
        AT45DB641E_write_page(&MEM3_PORT,MEM3_PIN,j);}
    //Memoire4
    for(j=0;j<VIRTUAL_MEMORY_BLOCK_SIZE/2;j++){
                    if (j==0){ AT45DB641E_write_buffer(&MEM4_PORT,MEM4_PIN,0x00,1,SEQ_START);}
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2-1){  
                        AT45DB641E_write_buffer(&MEM4_PORT,MEM4_PIN,0x00,1,SEQ_STOP);
                    }
                    else { AT45DB641E_write_buffer(&MEM4_PORT,MEM4_PIN,0x00,1,0);}
                }
    for(j=0;j<16;j++){
        AT45DB641E_write_page(&MEM4_PORT,MEM4_PIN,j);}
}



/** Reads blocks (OS blocks, not Dataflash pages) from the storage medium, the board Dataflash IC(s), into
 *  the pre-selected data IN endpoint. This routine reads in Dataflash page sized blocks from the Dataflash
 *  and writes them in OS sized blocks to the endpoint.
 *
 *  \param[in] MSInterfaceInfo  Pointer to a structure containing a Mass Storage Class configuration and state
 *  \param[in] BlockAddress  Data block starting address for the read sequence
 *  \param[in] TotalBlocks   Number of blocks of data to read
 */
void DataflashManager_ReadBlocks(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo,
                                 const uint32_t BlockAddress,
                                 uint16_t TotalBlocks)
{
	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady()) return;

	int i,j;
	for(i=0;i<TotalBlocks;i++){
		for(j=0;j<VIRTUAL_MEMORY_BLOCK_SIZE;j++){
			/* Check if the endpoint is currently empty */
			if (!(Endpoint_IsReadWriteAllowed()))
			{
				/* Clear the current endpoint bank */
				Endpoint_ClearIN();
				/* Wait until the host has sent another packet */
				if (Endpoint_WaitUntilReady()) return;
			}
			/* Retrieve byte in memory at address BlockAddress+i*VIRTUAL_MEMORY_BLOCK_SIZE+j */
			uint8_t byte;
						if ((BlockAddress+i) % 2 == 0){
                // Memoire 1
                if(VIRTUAL_MEMORY_BLOCK_SIZE/2 > j ){
                    if (j==0){ AT45DB641E_read_page(&MEM1_PORT,MEM1_PIN,(BlockAddress+i)/2,&byte,1,SEQ_START);}
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2-1){  
                        AT45DB641E_read_page(&MEM1_PORT,MEM1_PIN,(BlockAddress+i)/2,&byte,1,SEQ_STOP); 
                    }
                    else { AT45DB641E_read_page(&MEM1_PORT,MEM1_PIN,(BlockAddress+i)/2,&byte,1,0);}
                }
                // Memoire 2 on commence a ecrire a 256
                else if (j < VIRTUAL_MEMORY_BLOCK_SIZE) {
                    if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2){ AT45DB641E_read_page(&MEM2_PORT,MEM2_PIN,(BlockAddress+i)/2,&byte,1,SEQ_START);}
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE-1){  
                        AT45DB641E_read_page(&MEM2_PORT,MEM2_PIN,(BlockAddress+i)/2,&byte,1,SEQ_STOP); 
                        
                    }
                    else { AT45DB641E_read_page(&MEM2_PORT,MEM2_PIN,(BlockAddress+i)/2,&byte,1,0);}
                }
            }
			else {
                // Memoire 3
                if(VIRTUAL_MEMORY_BLOCK_SIZE/2 > j ){
                    if (j==0){ AT45DB641E_read_page(&MEM3_PORT,MEM3_PIN,(BlockAddress+i-1)/2,&byte,1,SEQ_START);}
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2 -1){  AT45DB641E_read_page(&MEM3_PORT,MEM3_PIN,(BlockAddress+i-1)/2,&byte,1,SEQ_STOP); }
                    else { AT45DB641E_read_page(&MEM3_PORT,MEM3_PIN,(BlockAddress+i-1)/2,&byte,1,0);}
                }
                // Memoire 4 on commence a ecrire a 256
                else if (j < VIRTUAL_MEMORY_BLOCK_SIZE) {
                    if (j==VIRTUAL_MEMORY_BLOCK_SIZE/2){ AT45DB641E_read_page(&MEM4_PORT,MEM4_PIN,(BlockAddress+i-1)/2,&byte,1,SEQ_START); }
                    else if (j==VIRTUAL_MEMORY_BLOCK_SIZE-1){  AT45DB641E_read_page(&MEM4_PORT,MEM4_PIN,(BlockAddress+i-1)/2,&byte,1,SEQ_STOP); }
                    else { AT45DB641E_read_page(&MEM4_PORT,MEM4_PIN,(BlockAddress+i-1)/2,&byte,1,0); }
                }
            }
			/* Send byte to USB host */
			Endpoint_Write_8(byte);
			if (MSInterfaceInfo->State.IsMassStoreReset) return;
		}
	}
	/* If the endpoint is full, send its contents to the host */
	if (!(Endpoint_IsReadWriteAllowed())) Endpoint_ClearIN();
}

/** Disables the Dataflash memory write protection bits on the board Dataflash ICs, if enabled. */
void DataflashManager_ResetDataflashProtections(void)
{
}

/** Performs a simple test on the attached Dataflash IC(s) to ensure that they are working.
 *
 *  \return Boolean \c true if all media chips are working, \c false otherwise
 */
bool DataflashManager_CheckDataflashOperation(void)
{
	return true;
}

