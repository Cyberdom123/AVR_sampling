# AVR_sampling
This is a small application for sampling analog data using internal atmega328p ADC  and sending them through UART to the computer.

Code description:
``` C
/* initializes ADC */
void ADC_init();
/* initializes UART */
void UART_init();
/* initializes TIMER1 */
void TIMER1_init(unsigned int prescaler_value=1024, uint16_t cmr=1561);
/* switches between adc channels (from 0 to 5) */
void ADC_switch(int chanel);
/* send char via USART */
void UART_putc(unsigned char data);
/* interrupt from adc after measurement */
ISR(ADC_vect);
/* interrupt from the timer after the measure counts down */
ISR(TIMER_OVF_vect);
/* interrupt after completion of character sending */
ISR(USART_RX_vect);
```
