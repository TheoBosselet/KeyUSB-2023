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
 *  Main source file for the MassStorage demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */
// Initialization leds //////// Moi
#define LED_DIR		DDRD
#define LED_PORT	PORTD
#define LED_BIT		6

#define LED_DIR1	DDRC
#define LED_PORT1	PORTC
#define LED_BIT1	2

#define BUTTON_DIR	DDRC
#define BUTTON_PORT PORTC
#define BUTTON_PIN	PINC
#define BUTTON_BIT	4

#define BUTTON_DIR2	 DDRC
#define BUTTON_PORT2 PORTC
#define BUTTON_PIN2	 PINC
#define BUTTON_BIT2	 5

#define CPT_MAX 100000L

#include "MassStorage.h"

/** LUFA Mass Storage Class driver interface configuration and state information. This structure is
 *  passed to all Mass Storage Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MS_Device_t Disk_MS_Interface =
	{
		.Config =
			{
				.InterfaceNumber           = INTERFACE_ID_MassStorage,
				.DataINEndpoint            =
					{
						.Address           = MASS_STORAGE_IN_EPADDR,
						.Size              = MASS_STORAGE_IO_EPSIZE,
						.Banks             = 1,
					},
				.DataOUTEndpoint           =
					{
						.Address           = MASS_STORAGE_OUT_EPADDR,
						.Size              = MASS_STORAGE_IO_EPSIZE,
						.Banks             = 1,
					},
				.TotalLUNs                 = TOTAL_LUNS,
			},
	};
//////// Moi
void init_button(void){
    BUTTON_DIR &= ~(1<<BUTTON_BIT);
    BUTTON_PORT |= (1<<BUTTON_BIT);
    BUTTON_DIR2 &= ~(1<<BUTTON_BIT2);
    BUTTON_PORT2 |= (1<<BUTTON_BIT2);
}

void vumetre(void){
    static long int compteur = CPT_MAX;
    static int k = 0;
    static unsigned char erase=0;
    if(compteur == 0){
        LED_PORT &= ~(1<<k);
        compteur = CPT_MAX;
        k++;
    }
    if(k==7 && !erase) { DataflashManager_Erase(); erase = 1; } 
    compteur --;
    }

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();
    
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();
    
    //////// Moi
    LED_DIR1 |= (1<<LED_BIT1);
    for(int k=0;k<7;k++){ LED_DIR |= (1<<k); LED_PORT |= (1<<k); }
    init_button();
    
    if (!(BUTTON_PIN & (1<<BUTTON_BIT)) && !(BUTTON_PIN2 & (1<<BUTTON_BIT2))){
    //fonctionnement de la clé si les deux boutons sont appuyés
	  for (;;)
	  {
		MS_Device_USBTask(&Disk_MS_Interface);
		USB_USBTask();
        vumetre();
        }
     }
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
	LEDs_Init();
	USB_Init();
	DataflashManager_Initialisation();

	/* Check if the Dataflash is working, abort if not */
	if (!(DataflashManager_CheckDataflashOperation()))
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		for(;;);
	}

	/* Clear Dataflash sector protections, if enabled */
	DataflashManager_ResetDataflashProtections();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= MS_Device_ConfigureEndpoints(&Disk_MS_Interface);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	MS_Device_ProcessControlRequest(&Disk_MS_Interface);
}

/** Mass Storage class driver callback function the reception of SCSI commands from the host, which must be processed.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface configuration structure being referenced
 */
bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
	bool CommandSuccess;

	LEDs_SetAllLEDs(LEDMASK_USB_BUSY);
	CommandSuccess = SCSI_DecodeSCSICommand(MSInterfaceInfo);
	LEDs_SetAllLEDs(LEDMASK_USB_READY);

	return CommandSuccess;
}

