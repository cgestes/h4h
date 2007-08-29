/**
 *  H4H project
 *  dsPIC 30F5015
 */

#include <stdlib.h>
#include "p30f5015.h"
#include "fuzzy.h"

/*****************************************************************
 * DEFINE ET MACRO
 *****************************************************************/

#define ACCEL_MIN 		0.0
#define ACCEL_MAX 		10.0
#define GYRO_MIN 		0.0
#define GYRO_MAX 		10.0
	
#define FC_NB_ENTRIES	5
#define FC_NB_SUBSETS 	4
	
#define FC_OUTPUT_MIN	0.0
#define FC_OUTPUT_MAX	10.0

/* Macros for Configuration Fuse Registers (copied from device header file):*/
_FOSC(CSW_FSCM_OFF & XT_PLL8);  /* Set up for Crystal multiplied by 8x PLL */
_FWDT(WDT_OFF);                 /* Turn off the Watch-Dog Timer.  */
_FBORPOR(MCLR_EN & PWRT_OFF);   /* Enable MCLR reset pin and turn off the power-up timers. */
_FGS(CODE_PROT_OFF);            /* Disable Code Protection */

/* Global Variables and Functions */
int main (void);
void ADC_Init(void);
void __attribute__((__interrupt__)) _ADCInterrupt(void);

unsigned int conversionResult[16];
volatile unsigned int *ptr;

/*****************************************************************
 * MAIN
 *****************************************************************/

int main (void) {
	TRISB = 0x31C; /* FIXME ajouter les autres entrées + Useless ? */ // configuration des pins 2, 3, 4, 8, 9 du PORTB en entrée
	ADC_Init();

/*****************************************************************
 * CONTRUCTION DES CONTROLEURS
 *****************************************************************/

	Reel sensor_mins[FC_NB_ENTRIES] = {GYRO_MIN /*X*/, GYRO_MIN /*Y*/, GYRO_MIN /*Z*/, ACCEL_MIN /*X*/, ACCEL_MIN /*Y*/};
	Reel sensor_maxs[FC_NB_ENTRIES] = {GYRO_MAX /*X*/, GYRO_MAX /*Y*/, GYRO_MAX /*Z*/, ACCEL_MAX /*X*/, ACCEL_MAX /*Y*/};
	unsigned int nb_subsets[FC_NB_ENTRIES] = {FC_NB_SUBSETS, FC_NB_SUBSETS, FC_NB_SUBSETS, FC_NB_SUBSETS, FC_NB_SUBSETS};

	ControleFlou *fc1 = build_ControleFlou(FC_NB_ENTRIES, sensor_mins, sensor_maxs, nb_subsets);
	ControleFlou *fc2 = build_ControleFlou(FC_NB_ENTRIES, sensor_mins, sensor_maxs, nb_subsets);
	ControleFlou *fc3 = build_ControleFlou(FC_NB_ENTRIES, sensor_mins, sensor_maxs, nb_subsets);
	ControleFlou *fc4 = build_ControleFlou(FC_NB_ENTRIES, sensor_mins, sensor_maxs, nb_subsets);

	fc1->Conclusions = (Reel*)malloc(sizeof (Reel) * fc1->NbRegle);
	fc2->Conclusions = (Reel*)malloc(sizeof (Reel) * fc2->NbRegle);
	fc3->Conclusions = (Reel*)malloc(sizeof (Reel) * fc3->NbRegle);
	fc4->Conclusions = (Reel*)malloc(sizeof (Reel) * fc4->NbRegle);

	printf("build_ControleFlou done\n");

	unsigned int i;
	for (i = 0; i < fc1->NbRegle; ++i) {
		fc1->Conclusions[i] = (float)(rand()%256);
	}
	/* FIXME : mettre des vrais signatures (NbRegle valeurs comprises entre FC_OUTPUT_MIN et FC_OUTPUT_MAX)*/

/*****************************************************************
 * TRAITEMENT DES ENTREES
 *****************************************************************/

/*
from Bigonoff :

- On choisit le canal à numériser, et on met en route le convertisseur    
- On attend Tacq
- On lance la numérisation
- On attend la fin de la numérisation
- On attend 2 Tad
- On recommence avec le canal suivant.

	Val numérisée = ((VIN - VREF-) / (VREF+ - VREF-)) * 1023)
	VIN analogique = ((Val/1023) * (VREF+ - VREF-)) + VREF 
	Temps de conversion sur N bits = Tad + N * Tad + (11-N) (2Tosc)
	Tad = Tosc * diviseur = 1,6 µs
	Temps de conversion sur 10 bits = 12 Tad
	Tacq = 2µs + Tc = 0,914895 * 10-9 * (Rinterne + Rsource) + 0,05(T° - 25°C) avec T° = 25°C

	Tacq courant : 19,7µs
	Temps de conversion courant : 19,2µs.
	Temps entre 2 numérisation successives : 3,2µs


from datasheet :

The following steps should be followed for doing an	A/D conversion:

1. Configure the A/D module:
	- Configure analog pins, voltage reference and digital I/O
	- Select A/D input channels
	- Select A/D conversion clock
	- Select A/D conversion trigger
	- Turn on A/D module
2. Configure A/D interrupt (if required):
	- Clear ADIF bit
	- Select A/D interrupt priority
3. Start sampling.
4. Wait the required acquisition time.
5. Trigger acquisition end, start conversion
6. Wait for A/D conversion to complete, by either:
	- Waiting for the A/D interrupt
7. Read A/D result buffer, clear ADIF if required.

*/


	Reel entries[FC_NB_ENTRIES] = {1.0, 2.0, 3.0, 4.0, 5.0};  /* FIXME test */

	Reel output1 = Commande(fc1, entries);
	//Reel output2 = Commande(fc2, entries);
	//Reel output3 = Commande(fc3, entries);
	//Reel output4 = Commande(fc4, entries);

	//printf("%r, %r, %r ,%r\n", output1, output2, output3, output4);
	printf("%r\n", output1);
    while (1) {
	}              				/* Loop endlessly...anytime an interrupt occurs */
                                /* the processor will vector to the interrupt and */
                                /* return back to the while(1) loop */
}



/*****************************************************************
 * ADC_Init() is used to configure A/D
 *****************************************************************/

void ADC_Init(void) {

/************************************************************
 * ADCON1 Register
 ************************************************************/

/*
	ADCON1bits.DONE   :1;	// A/D Conversion Status bit
	ADCON1bits.SAMP   :1;	// A/D Sample Enable bit
	ADCON1bits.ASAM   :1;	// A/D Sample Auto-Start bit
	ADCON1bits.SIMSAM :1;	// Simultaneous Sample Select bit (only applicable when CHPS = 01 or 1x)
	ADCON1bits.SSRC   :3;	// Conversion Trigger Source Select bits
	ADCON1bits.FORM   :2;	// Data Output Format bits
	ADCON1bits.ADSIDL :1;	// Stop in Idle Mode bit
	ADCON1bits.ADON   :1;   // A/D Operating Mode bit
*/

    ADCON1bits.FORM = 1; 	// signed integer
    ADCON1bits.SSRC = 7; 	// auto convert
	ADCON1bits.SIMSAM = 0; 	// 
    ADCON1bits.ASAM = 0; 	// Sampling begins when SAMP bit set
	//All other bits to their default state

/************************************************************
 * ADCON2 Register
 ************************************************************/

/*
	ADCON2bits.ALTS   :1;	// Alternate Input Sample Mode Select bit
	ADCON2bits.BUFM   :1;	// Buffer Mode Select bit
	ADCON2bits.SMPI   :4;	// Sample/Convert Sequences Per Interrupt Selection bits
	ADCON2bits.BUFS   :1;	// Buffer Fill Status bit
	ADCON2bits.CHPS   :2;	// Selects Channels Utilized bits
	ADCON2bits.CSCNA  :1;	// Scan Input Selections for CH0+ S/H Input for MUX A Input Multiplexer Setting bit
	ADCON2bits.VCFG   :3;	// 000 (VDD-VSS)
*/

	ADCON2bits.VCFG = 0;	// VrefH = VDD, VrefL = VSS
	ADCON2bits.CHPS = 0;	// Converts CH0, CH1, CH2 and CH3 (req. ADCON1<SIMSAM> = 1)
    ADCON2bits.SMPI = 15;	// Interrupts at the completion of conversion for each 16th sample/convert sequence /* FIXME */
	ADCON2bits.BUFM = 0;	// Buffer configured as one 16-word buffer ADCBUF(15...0.) /* Useless ? */
	// All other bits to their default state

/************************************************************
 * ADCON3 Register
 ************************************************************/

/*
	ADCON3bits.ADCS   :6;	// A/D Conversion Clock Select bits
	ADCON3bits.ADRC   :1;	// A/D Conversion Clock Source bit
	ADCON3bits.SAMC   :5;	// Auto-Sample Time bits
*/

    //We will set up the ADRC as the A/D conversion clock
    //so that the A/D converter can operate when the device is in
    //SLEEP mode. Also, 1 Tad period is allocated for sampling time.
    //The conversion rate for the ADRC oscillator is depends on whether
    //the device is a dsPIC30F or dsPIC33F device and also whether the
    //A/D module is a 10-bit or 12-bit A/D converter.
    //Please refer to the device Datasheet for "ADRC" conversion rate.

	ADCON3bits.ADCS = 9;	// TCY/2 • (ADCS<5:0> + 1) = TCY/2 /* FIXME */
    ADCON3bits.SAMC = 1; 	// 1 TAD /* FIXME */
    ADCON3bits.ADRC = 0;	// A/D internal RC clock


/************************************************************
 * ADCHS Register
 ************************************************************/

    //Set up A/D Channel Select Register to convert AN4 on Mux A input
    //of CH0 S/H amplifiers

    ADCHS = 0x0004; /* FIXME */

/************************************************************
 * ADCSSL Register
 ************************************************************/

    //Channel Scanning is disabled. All bits left to their default state

    ADCSSL = 0x0000;

/************************************************************
 * ADPCFG Register
 ************************************************************/

    //Set up channels AN2 3 4 8 9 as analog input and configure rest as digital

	//ADPCFG = 0x31C; /* FIXME */
    ADPCFG = 0xFFFF;
    ADPCFGbits.PCFG2 = 0;
	ADPCFGbits.PCFG3 = 0;
	ADPCFGbits.PCFG4 = 0;
	ADPCFGbits.PCFG8 = 0;
	ADPCFGbits.PCFG9 = 0;

/************************************************************
 * INIT
 ************************************************************/

    //Clear the A/D interrupt flag bit

    IFS0bits.ADIF = 0;

    //Set the A/D interrupt enable bit

    IEC0bits.ADIE = 1;

    //Turn on the A/D converter
    //This is typically done after configuring other registers

    ADCON1bits.ADON = 1;
	ADCON1bits.SAMP = 1;
}


/************************************************************
 * _ADCInterrupt
 ************************************************************/

//_ADCInterrupt() is the A/D interrupt service routine (ISR).
//The routine must have global scope in order to be an ISR.
//The ISR name is chosen from the device linker script.
//If the device is in SLEEP mode, the A/D interrupt wakes
//the device up. The device vectors to the A/D ISR upon wake-up.
void __attribute__((__interrupt__)) _ADCInterrupt(void)
{
        int i = 0;
        ptr = &ADCBUF0;
        while (i < 16) {
                conversionResult[i++] = *ptr++;
        }

        //Clear the A/D Interrupt flag bit or else the CPU will
        //keep vectoring back to the ISR
        IFS0bits.ADIF = 0;
}

/*
EOF
*/




/************************************* BATTLEGROUND ****************************************
		SENSORS:
		 acceleroX (PA7) 	RB8
		 gyroscopeX (PA8) 	RB9
		 gyroscopeY (PA9) 	RB2
		 acceleroY (PA10) 	RB3
		 gyroscopeZ (PA11) 	RB4
		SERIAL:
		 T1IN 				RF3
		 R1OUT 				RF2
		ICSP:
		 ICSP4 				RB7
		 ICSP5 				RB6
		OUTPUT:
		 moteur1 (PA0) 		PWM1L
		 moteur2 (PA1) 		PWM2L
		 moteur3 (PA6) 		PWM3L
		 moteur3 (PA7) 		PWM4L
		RETOUR:
		 moteur1 (PA2) 		RB10
		 moteur2 (PA3) 		RB11
		 moteur3 (PA4) 		RB12
		 moteur4 (PA5) 		RB13

		RB0			
		RB1
		RB2					gyroscopeY
		RB3					acceleroY
		RB4					gyroscopeZ
		RB5
		RB6					ICSP5
		RB7					ICSP4
		RB8					acceleroX
		RB9					gyroscopeX
		RB10				retour1
		RB11				retour2
		RB12				retour3
		RB13				retour4
		RB14
		RB15

	TRISBbits.TRISB9 = 1; // gyroX
	TRISBbits.TRISB2 = 1; // gyroY
	TRISBbits.TRISB4 = 1; // gyroZ
	TRISBbits.TRISB8 = 1; // accelX
	TRISBbits.TRISB3 = 1; // accelY

************************************************************************************/