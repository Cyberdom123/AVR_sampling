#include<avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BAUD 500000
#include <util/setbaud.h>

volatile uint8_t ones;
volatile uint8_t tens;
volatile uint8_t hund;
volatile uint8_t thou;

volatile uint16_t CMR_value; 

volatile unsigned int CrossChannel=0;
volatile unsigned int ADC_in=1;

void ADC_stop(){
    ADCSRA &=
            ~(1<<ADEN)& //Stop ADC 
            ~(1<<ADSC);
}

void ADC_start(){
    ADCSRA |=
            (1<<ADEN)| //Start ADC 
            (1<<ADSC);       
}

void TIMER1_init(unsigned int prescaler_value=1024, uint16_t CMR=1561){
    ADC_stop();
    //Timer stop 
    TCCR1B &= ~(1<<CS12)&
              ~(1<<CS11)&
              ~(1<<CS10);

    //Disable interrupts          
    TIMSK1 &= ~(1<<TOIE1);

    if(CrossChannel == 1){
        CMR = CMR/2; //If CrossChannel is on, double freq
    }
    //Set cunter inital value
    CMR_value = 65536-CMR;
    TCNT1 = CMR_value;

    //Set prescaler
    switch(prescaler_value){
        case 1024:
            TCCR1B |= (1<<CS12)|(1<<CS10);
            break;
        case 256:
            TCCR1B |= (1<<CS12);
            break;
        case 64:
            TCCR1B |= (1<<CS11)|(1<<CS10);
            break;
        case 8:
            TCCR1B |= (1<<CS11);
            break;
        case 1:
            TCCR1B |= (1<<CS10);
            break;
        default:
            break;
    }
    //Enable overflow interrupt
    TIMSK1 |= (1<<TOIE1);
}

void ADC_init(){
    ADCSRA |= 
            (1<<ADATE)| //ADC auto trigger
            (1<<ADIE)| //ADC interrupt enabled
            (3<<ADPS0); //Prescaler set to 8

    //Trigger source select
    ADCSRB |= (1<<ADTS2)|(1<<ADTS1); //Counter interrupt trigger
    ADMUX |= 
            (1<<MUX0)| //ADC1
            (1<<REFS0)| //Internal 1.1V Voltage Reference
            (1<<REFS1);

    ADC_stop();
}

void ADC_switch(int Chanel){
    ADCSRA &= ~(1<<ADEN);

    ADMUX &= ~(1<<MUX0)& //Clear ADC chanel register
             ~(1<<MUX1)&
             ~(1<<MUX2)&
             ~(1<<MUX3);
    ADMUX |= (Chanel<<MUX0); //Set ADC chanel

    ADCSRA |= (1<<ADEN);           
}

void UART_init(){
    UBRR0H = UBRRH_VALUE; //Set baud rate
    UBRR0L = UBRRL_VALUE;

    UCSR0A |= (1<<U2X0); //Double the speed (baud x2)
    UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00); //8bit character size    
    UCSR0B |= 
            (1<<RXEN0) | (1<<TXEN0)| //Enable Transmitter, Reciever
            (1<<RXCIE0); // RX Complete Interrupt Enable
}

void UART_putc(unsigned char Data)
{
    //Wait for transmit buffer to be empty
    loop_until_bit_is_set(UCSR0A, UDRE0);

    //Load data into transmit register
    UDR0 = Data;
}

ISR(TIMER1_OVF_vect)
{
    TCNT1 = CMR_value; //Set timer val, after overflow
}

ISR(USART_RX_vect) {
    while(!(UCSR0A&(1<<RXC0))){}; //Loop until transmit buffer will be empty
    unsigned char received = UDR0; //Get data

    switch(received){
        case'0':
            ADC_stop();
            break;
        case'1':
            CrossChannel = 0;
            ADC_switch(1);
            break;
        case'2':
            CrossChannel = 1;
            break;
        case'3':
            TIMER1_init(1024, 1561);
            break;
        case'4':
            TIMER1_init(1024, 780);
            break;
        case'5':
            TIMER1_init(1024, 389);
            break;
        case'6':
            TIMER1_init(1024, 155);
            break;
        case'7':
            TIMER1_init(1024, 77);
            break;
        case'8':
            TIMER1_init(256, 61);
            break;
        case'9':
            TIMER1_init(256, 30);
            break;
        case'a':
            TIMER1_init(1024, 3);
            break;
        case'b':
            TIMER1_init(64, 24);
            break;
        case'c':
            TIMER1_init(8, 99);
            break;
        case'd':
            TIMER1_init(8, 19);
            break;
        case'e':
            break;
        case'f':
            ADC_start();
            break;
        default:
            break;
    }
}

ISR(ADC_vect){
    ADCSRA &= ~(1<<ADEN);
    uint16_t adc_reading = ADC;

    ones = adc_reading % 10;
    adc_reading = adc_reading/10;
    tens = adc_reading % 10;
    adc_reading = adc_reading/10;
    hund = adc_reading % 10;
    thou = adc_reading/10;

    ones += 0x30; //ASCI conversion
    tens += 0x30;
    hund += 0x30;
    thou += 0x30;

    UART_putc(thou);
    UART_putc(hund);
    UART_putc(tens);
    UART_putc(ones);
    UART_putc('\n');

    if(CrossChannel == 1){
        switch (ADC_in){ 
        case 2:
            ADC_switch(2);
            ADC_in = 1;
            break;
        case 1:
            UART_putc('*');
            ADC_switch(1);
            ADC_in = 2;
            break;
        default:
            break;
        }
    }
    ADCSRA |= (1<<ADEN);
}

int main(){
    UART_init();
    TIMER1_init();
    ADC_init();
    sei();

    while(1){

    }
}
