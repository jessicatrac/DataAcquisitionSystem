// filename ******** Main.C ************** 

//;*********************************************************************                  
//;*                       McMaster University                         *
//;*                      2DP4 Microcontrollers                        *
//;*                          Lab Section 05                           * 
//;*                  Jessica Trac Tracj2 1404451                      *
//;*********************************************************************
//;*********************************************************************
//;*                          Final Project                            *
//;*         ADC Conversion of analog signal from transducer,          *
//;*    which is communicated to PC through serial communication.      *
//;*********************************************************************
//;*****************************************************************
//;* References                                                    *
//;* Setting bus speed - Lecture 2/3 of week 8                     * 
//;* Setting baud rate - Lecture 2/3 of week 8                     *
//;* Setting serial communication - Tutorial challenge 7           *
//;* ADC configuration - Lab 4 material                            *
//;* Sampling rate calculation - Lecture 2 of week B               *
//;* Register retrieved from reference manual + data sheet         *
//;*****************************************************************


#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "SCI.h"
int i, val4, m, n;
int button_on = 1; // for interrupt 

// Prototypes
void setCLK(void); // set bus speed
void setADC(void); // set ADC = AN4
void delayms(unsigned int); // to test bus speed, 1ms delay
void sample_delayms(unsigned int); // for sampling rate 440 Hz   - did not end up using
void OutCRLF(void); // for serial communication
void brutedelay(unsigned int); // brute force delay method


void main(void) {

  setCLK();   // set bus speed to 4 MHz
  setADC();   // set ADC to AN4, 8-bit
  

  //Set Ports
  DDRJ = 0xFF;      //set all port J as output
  
/*
 * The next six assignment statements configure the Timer Input Capture                                                   
 */           
  TSCR1 = 0x90;    //Timer System Control Register 1     , 0x90 = 1001 0000 
                    // TSCR1[7] = TEN:  Timer Enable (0-disable, 1-enable)
                    // TSCR1[6] = TSWAI:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[5] = TSFRZ:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[4] = TFFCA:  Timer Fast Flag Clear All (0-normal 1-read/write clears interrupt flags)
                    // TSCR1[3] = PRT:  Precision Timer (0-legacy, 1-precision)
                    // TSCR1[2:0] not used

  TSCR2 = 0x00;    //Timer System Control Register 2
                    // TSCR2[7] = TOI: Timer Overflow Interrupt Enable (0-inhibited, 1-hardware irq when TOF=1)
                    // TSCR2[6:3] not used
                    // TSCR2[2:0] = Timer Prescaler Select: See Table22-12 of MC9S12G Family Reference Manual r1.25 (set for bus/1)
  
                    
  TIOS = 0xFE;     //Timer Input Capture or Output capture      , 0xFE = 1111 1110 so pin 0 is input-capture, all others are output-compare
                    //set TIC[0] and input (similar to DDR)
  PERT = 0x01;     //Enable Pull-Up resistor on TIC[0]

  TCTL3 = 0x00;    //TCTL3 & TCTL4 configure which edge(s) to capture   (TCTL3 is for pins 7-4, neflect)
  TCTL4 = 0x02;    //Configured for falling edge on TIC[0]              (TCTL4 pins 3-0, pin 2 is set 1 0 = falling edges captured!)

/*
 * The next one assignment statement configures the Timer Interrupt Enable                                                   
 */           
   
  TIE = 0x01;      //Timer Interrupt Enable            , 0x01 = 0000 0001 for pin 0

/*
 * The next one assignment statement configures the ESDX to catch Interrupt Requests                                                   
 */           
                        
  SCI_Init(19200);
  
	EnableInterrupts;

  DDRJ = 0x01;


  for(;;) {
  if(button_on == 0){
    PTJ ^= 0x01;
    val4 = ATDDR0; // store ADC value from channel 4 
    SCI_OutUDec(val4, 0);
    SCI_OutChar(CR); // store in excel .csv file as a new cell
    brutedelay(1); // conversion = 0.0035ms, Tx*3 = 0.52ms*3 = 1.56ms.. delay needs to be 0.79022ms, new delay fxn needed
     }
  } 
  
}


/*                             
 * This is the Interrupt Service Routine for TIC channel 0 (Code Warrior has predefined the name for you as "Vtimch0"                                                    
 */           
interrupt  VectorNumber_Vtimch0 void ISR_Vtimch0(void)
{
  unsigned int temp; // do not change
  PTJ = 0x01;           // Make sure the LED isn't on to begin with, since that would be confusing  
  button_on ^= 1;    // toggle button variable to switch states

  temp = TC0;       //Refer back to TFFCA, we enabled FastFlagClear, thus by reading the Timer Capture input we automatically clear the flag, allowing another TIC interrupt
}  








// Other functions

void setCLK(void) {  // Bus Speed = 4 MHz
  CPMUCLKS = 0x80; // choose PLL = 1
  CPMUOSC = 0x00; // source clock chosen
  CPMUPROT = 0x26; // protection of clock config is enable
  // now frequency_ref = 1MHz
  CPMUSYNR = 0x0F; // VCOFRQ = 00, SYNDIN = 15 = F; 32MHz = VCOCLK = 2*1MHz*(1+SYNDIV);
  CPMUFLG = 0x00; // LOCK = 0
  // PLLCLK = 32MHz/4 = 8MHz and BUSCLK = PLLCLK/2 = 4MHz! bus speed set to 4MHz  
}

void setADC(void) {     // ADC - AN4, 8-BIT		
	ATDCTL1 = 0x0F; // 8-bit resolutoin
	ATDCTL3 = 0x88;	// right justified, one sample per sequence   
	ATDCTL4 = 0x01;	// prescaler = 1; ATD clock = 4MHz / (2 * (2 + 1)) == 1MHz      
	// ^ for above, sample + hold is set to 000 = 4 so (8bit+4+2)/4MHz -> 0.0035ms ADC conversion time 
	ATDCTL5 = 0x24;		// continuous conversion on channel 4 
}


void brutedelay(unsigned int k){   // experimentally found to be a delay of 1.14ms (?)
  unsigned int i;
  unsigned int j;

  for(i = 0; i < k; i++){
    for(j = 0; j < 68; j++){   // value of 68 was experimentally found
      PTJ = PTJ;
      PTJ = PTJ;
      PTJ = PTJ;
      PTJ = PTJ;
    }
  }
}


void sample_delayms(unsigned int j) {  // function to get 440 Hz sampling rate  ... not that ideal
  int ix; 
  TSCR1 = 0x90; // enable timer and fast timer flag clear
  TSCR2 = 0x00; // disable timer interrupt + set prescalar to 16... key part!
  TIOS |= 0x02; 
  TC1 = TCNT + 2800; // (1/4MHz)*2800 = 0.7ms
  
  for(ix=0;ix<j;ix++) {
    while(!(TFLG1_C1F));
     TC1 += 2800; 
  }
  TIOS &= ~0x01;  

}


void delayms(unsigned int k) { // function that implements 1 ms delay with 4MHz bus speed
  int ix; 
  TSCR1 = 0x90; // enable timer and fast timer flag clear
  TSCR2 = 0x00; // disable timer interrupt + set prescalar to 16... key part!
  TIOS |= 0x02; 
  TC1 = TCNT + 4000; // (1/4MHz)*4000 = 1ms
  
  for(ix=0;ix<k;ix++) {
    while(!(TFLG1_C1F));
     TC1 += 4000; 
  }
  TIOS &= ~0x01;  
}

void OutCRLF(void){  // was included in other labs w serial comm. 
  SCI_OutChar(CR);
  SCI_OutChar(LF);
  PTJ ^= 0x20;          // toggle LED D2
}

