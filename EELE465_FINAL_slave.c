#include <msp430.h> 


/**
 * main.c
 */
#define STOP 1600
#define SHIFT 100

int data_in = 0;
int speed = 4;
int slvAddr = 69;
int ADC_Value = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

// I2C Port Configuration
    UCB0CTLW0 |= UCSWRST;               // But eUSCI_B1 into software reset

    UCB0CTLW0 |= UCMODE_3;              // Choose I2C mode
    UCB0CTLW0 &= ~UCMST;                // Choose to be Slave
    UCB0CTLW0 &= ~UCTR;                 // Set to READ mode

    UCB0I2COA0 |= UCOAEN;               // Own Address enable
    UCB0I2COA0 |= slvAddr;
    UCB0I2COA0 |= UCGCEN;               // call/response enable

//----- Config ports --------

    P1SEL1 &= ~BIT3;                    // P1.3 as SCL
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;                    // P1.2 as SDA
    P1SEL0 |= BIT2;

    UCB0CTLW0 &= ~UCSWRST;              // Remove eUSCI_B0 from software reset

//------ ADC ------
    P1SEL1 |= BIT4;                     // ADC input
    P1SEL0 |= BIT4;

    ADCCTL0 &= ~ADCSHT;
    ADCCTL0 |= ADCSHT_2;                // Conversion Cycles = 16
    ADCCTL0 |= ADCON;                   // Turn ADC on
    ADCCTL1 |= ADCSSEL_2;               // ADC Clock src = SMCLK
    ADCCTL1 |= ADCSHP;                  // sample signal source = sampling timer
    ADCCTL2 &= ~ADCRES;                 // Clear ADCRES from def. of ADCRES=01
    ADCCTL2 |= ADCRES_2;                // Resolution = 12-bit
    ADCMCTL0 |= ADCINCH_4;              // ADC Input Channel = A4 (P1.4)

    P1DIR |= BIT5;
    P1OUT &= ~BIT5;

//  -----Servo------
    // Setup timer B0
    TB0CTL |= TBCLR;                    // Clears timer B0
    TB0CTL |= TBSSEL__SMCLK;            // Choose SMCLK as src
    TB0CTL |= MC__UP;                   // Put in UP mode
    TB0CCR0 = 21000;                    // CCR0
    TB0CCR1 = STOP;                     // CCR1 for no movement
    TB0CCTL0 |= CCIE;                   // IRQ for CCR0
    TB0CCTL1 |= CCIE;
    __enable_interrupt();

    TB0CCTL0 &= ~CCIFG;                 // Clear flg
    TB0CCTL1 &= ~CCIFG;

    P1DIR |= BIT1;
    P1OUT |= BIT1;

    PM5CTL0 &= ~LOCKLPM5;               // Turn on I/O

    ADCIE |= ADCIE0;                    // ADC Conv Complete IRQ
    UCB0IE |= UCRXIE0;                  // Enable Rx buffer interrupt
   __enable_interrupt();

    while(1){
       if(ADC_Value > 7){
           P1OUT |= BIT5;
           switch(speed){               // Servo rotation speed depends on I2C input
           case 1:
               TB0CCR1 = (STOP - 250);
               break;
           case 2:
               TB0CCR1 = (STOP - 150);
               break;
           case 3:
               TB0CCR1 = (STOP - 50);
               break;
           case 4:
               TB0CCR1 = (STOP);
               break;
           case 5:
               TB0CCR1 = (STOP + 50);
               break;
           case 6:
               TB0CCR1 = (STOP + 150);
               break;
           case 7:
               TB0CCR1 = (STOP + 250);
               break;
           default:
               TB0CCR1 = STOP;
               break;
           }
       }
       else{
           P1OUT &= ~BIT5;
           TB0CCR1 = STOP;
       }
    }
    return 0;
}


// ------------------ ISR subroutines --------------

#pragma vector = EUSCI_B0_VECTOR
__interrupt void slaveisr(void){
    UCB0IE &= ~UCRXIE0;                     // Pause interrupts for duration of ISR
    data_in = UCB0RXBUF;

//--------------- Slave functionality -----------------------
    speed = data_in;
//-----------------------------------------------------------
    ADCCTL0 |= ADCENC | ADCSC;              // Enable and start conversion
   __bis_SR_register(GIE | LPM0_bits);
    UCB0IE |= UCRXIE0;                      // Allow read interrupts again

}

#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR(void){
    ADC_Value = ADCMEM0;
    __bic_SR_register_on_exit(LPM0_bits);
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void pwm_high(void)
{
    P1OUT |= BIT1;                      // HIGH bit
    TB0CCTL0 &= ~CCIFG;                 // clear flag
}

#pragma vector = TIMER0_B1_VECTOR
__interrupt void pwm_low(void)
{
    P1OUT &= ~BIT1;                     // CLEAR PWM
    TB0CCTL1 &= ~CCIFG;                 // clear flag
}
