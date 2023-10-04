#include <msp430.h> 


/**
 * main.c
 */
    int clk = 0;
    int i = 0;
    //unsigned int data = 0;
    int speed = 4;
    int slvAddr = 69;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    P6DIR |= BIT0;              // Pin 6.0 = SRCLR
    P6OUT &= ~ BIT0;
    P6DIR |= BIT1;              // Pin 6.1 = SRCLK
    P6OUT &= ~ BIT1;
    P6DIR |= BIT2;              // Pin 6.2 = RCLK
    P6OUT &= ~ BIT2;
    P6DIR |= BIT3;              // Pin 6.3 = OE
    P6OUT &= ~BIT3;
    P6DIR |= BIT4;              // Pin 6.4 = SER
    P6OUT &= ~ BIT4;

    P1DIR &= ~BIT2;             // Set pin 1.2 to low input
    P1REN |= BIT2;              // Pulldown resistor
    P1OUT &= ~BIT2;

    P1IFG &= ~BIT2;             // Enable pin 1 interrupt
    P1IES &= ~BIT2;
    P1IE |= BIT2;

    P2DIR &= ~BIT7;             // Set pin 2.7 to low input
    P2REN |= BIT7;              // Pulldown resistor
    P2OUT &= ~BIT7;

    P2IFG &= ~BIT7;             // Enable pin 2 interrupt
    P2IES &= ~BIT7;
    P2IE |= BIT7;

    P3DIR &= ~BIT0;             // Set pin 2.7 to low input
    P3REN |= BIT0;              // Pulldown resistor
    P3OUT &= ~BIT0;

    P3IFG &= ~BIT0;             // Enable pin 2 interrupt
    P3IES &= ~BIT0;
    P3IE |= BIT0;

    P1DIR |= BIT1;
    P1OUT &= ~BIT1;

// I2C Port Configuration
    UCB1CTLW0 |= UCSWRST;               // But eUSCI_B1 into software reset

    UCB1CTLW0 |= UCSSEL_3;              // select SMCLK
    UCB1BRW = 10;                       // Set prescalar to 10

    UCB1CTLW0 |= UCMODE_3;              // Choose I2C mode
    UCB1CTLW0 |= UCMST;                 // Choose to be Master
    UCB1CTLW0 |= UCTR;                // Set to WRITE mode
    UCB1I2CSA |= slvAddr;


    UCB1CTLW1 |= UCASTP_2;              // Auto Stop Mode
    UCB1TBCNT = 1;

    //----- Config ports --------

    P4SEL1 &= ~BIT7;                    // P4.7 as SCL
    P4SEL0 |= BIT7;

    P4SEL1 &= ~BIT6;                    // P4.6 as SDA
    P4SEL0 |= BIT6;

    UCB1CTLW0 &= ~UCSWRST;              // Remove eUSCI_B1 from software reset
    PM5CTL0 &= ~LOCKLPM5;

    UCB1IE |= UCTXIE0;                  // Write (Tx) buffer interrupt enable
    __enable_interrupt();


    while(1){

// Repeatedly communicate with slave

        UCB1CTLW0 |= UCTXSTT;                   // Start message
        while((UCB1IFG & UCSTPIFG) == 0){}
        UCB0IFG &= ~UCSTPIFG;           // Clear stop flag
        for(i = 0; i < 100; i++){}


        convert(speed);
    }
    return 0;
}

//--------- ISRs--------------

#pragma vector = EUSCI_B1_VECTOR
__interrupt void sendSpeed(void){
    UCB1TXBUF = speed;
}

#pragma vector = PORT1_VECTOR
__interrupt void upPress(void){
    P1IE &= ~BIT2;
    P1OUT &= ~BIT2;
    for(i = 0; i < 5000; i++){}           // Debouncer delay
    if(speed < 7){
        speed += 1;
    }
    for(i = 0; i < 5000; i++){}           // Debouncer delay
    P1IFG &= 0;
    P1IE |= BIT2;
}

#pragma vector = PORT2_VECTOR
__interrupt void downPress(void){
    P2IE &= ~BIT7;
    P2OUT &= ~BIT7;                     // Set input back at LOW
    for(i = 0; i < 5000; i++){}           // Debouncer delay
    if(speed > 1){
        speed -= 1;
    }
    for(i = 0; i < 5000; i++){}           // Debouncer delay
    P2IFG &= 0;
    P2IE |= BIT7;
}

#pragma vector = PORT3_VECTOR
__interrupt void stopPress(void){
    P3IE &= ~BIT3;
    P3OUT &= ~BIT3;                     // Set input back at LOW
    for(i = 0; i < 5000; i++){}           // Debouncer delay
    speed = 4;
    for(i = 0; i < 5000; i++){}           // Debouncer delay
    P3IFG &= 0;
    P3IE |= BIT3;
}

void send(data_in){

    //unsigned char data = (char) data_in;
    //
    int i = 0;
    int j = 0;
    //int bit_cnt = 0;
    unsigned int mask = 0b10000000;
    int curBit = 0;

    P6OUT |= BIT0;                     // Clear register
    for(i = 0; i < 50; i++){}
    P6OUT |= BIT0;
    for(i = 0; i < 8; i++){
        curBit = data_in & mask;
        mask >>= 1;
        if(curBit == 0){
            P6OUT &= ~BIT4;  // Data 1
        }
        else{

            P6OUT |= BIT4;  // Data 0
           // P6OUT &= ~BIT3;
        }
        for(j = 0; j <4; j++){}
        P6OUT |= BIT1;
        //P6OUT |= BIT2;

        for(j = 0; j <10; j++){}
        //P6OUT &= ~BIT2;
        P6OUT &= ~BIT1;
        for(j = 0; j <4; j++){}
        P6OUT &= ~BIT4;


        //for(j = 0; j < 5000; j++){}
    }
    //P6OUT |= BIT3;                      // Enable output
    P6OUT &= ~BIT3;
    for(j = 0; j <4; j++){}
    P6OUT |= BIT1;
    P6OUT |= BIT2;

    for(j = 0; j <10; j++){}
    P6OUT &= ~BIT2;
    P6OUT &= ~BIT1;
    for(j = 0; j <4; j++){}
    P6OUT &= ~BIT4;
    for(i = 0; i < 10000; i++){
        for(j = 0; j< 3; j++){}
    }

    //P6OUT |= BIT3;                      // Disable Ouput
    P6OUT&= ~BIT0;                      // Clear


    return;
}

void convert(int input){
    switch(input){
    case 0x0:
        send(0b111111101);              // Zero
        break;
    case 0x1:
        send(0b01100111);               // Speed 3 down
        //send(0b001000001);
        break;
    case(0x2):
        send(0b000110111);              // Speed 2 down
        break;
    case(0x3):
        send(0b001000001);              // Speed 1 down
        break;
    case(0x4):
        send(0b111111101);              // Speed '4' is stopped
        break;
    case(0x5):
        send(0b001000001);
        break;
    case(0x6):
        send(0b000110111);
        break;
    case(0x7):
        send(0b01100111);               // Speed 3 up
        break;
    case(0x8):
        send(0b01111111);
        break;
    case(0x9):
        send(0b01101111);
        break;
    default:
        break;

    }
    return;
}
