extern unsigned int _etext;
extern unsigned int _data;
extern unsigned int _edata;
extern unsigned int _bss;
extern unsigned int _ebss;
extern void SystemInit(void);
extern void main(void);

void __nop() {
	asm("nop");
}

#define DUMMY __attribute__ ((weak, alias ("irq_handler_dummy")))

void irq_handler_reset() {
  unsigned int *src, *dst;

  src = &_etext;
  dst = &_data;
  while (dst < &_edata)
    *dst++ = *src++;

  dst = &_bss;
  while (dst < &_ebss)
    *dst++ = 0;

	SystemInit();
	main();
	while(1);
}

void irq_handler_dummy(void)
{
  	while (1);
}

DUMMY void irq_handler_nmi(void);
DUMMY void irq_handler_hard_fault(void);
DUMMY void irq_handler_sv_call(void);
DUMMY void irq_handler_pend_sv(void);
DUMMY void irq_handler_sys_tick(void);
DUMMY void TMR0_IRQHandler(void);
DUMMY void GPIO_IRQHandler(void);
DUMMY void SLAVE_IRQHandler(void);
DUMMY void SPI0_IRQHandler(void);
DUMMY void BB_IRQHandler(void);
DUMMY void LLE_IRQHandler(void);
DUMMY void USB_IRQHandler(void);
DUMMY void ETH_IRQHandler(void);
DUMMY void TMR1_IRQHandler(void);
DUMMY void TMR2_IRQHandler(void);
DUMMY void UART0_IRQHandler(void);
DUMMY void UART1_IRQHandler(void);
DUMMY void RTC_IRQHandler(void);
DUMMY void ADC_IRQHandler(void);
DUMMY void SPI1_IRQHandler(void);
DUMMY void LED_IRQHandler(void);
DUMMY void TMR3_IRQHandler(void);
DUMMY void UART2_IRQHandler(void);
DUMMY void UART3_IRQHandler(void);
DUMMY void WDT_IRQHandler(void);

__attribute__ ((used, section(".vectors")))
void (* const vectors[])(void) =
{
	0x20008000,
	irq_handler_reset,
	irq_handler_nmi,               // 2 - NMI
	irq_handler_hard_fault,        // 3 - Hard Fault
	0,                             // 4 - Reserved
	0,                             // 5 - Reserved
	0,                             // 6 - Reserved
	0,                             // 7 - Reserved
	0,                             // 8 - Reserved
	0,                             // 9 - Reserved
	0,                             // 10 - Reserved
	irq_handler_sv_call,           // 11 - SVCall
	0,                             // 12 - Reserved
	0,                             // 13 - Reserved
	irq_handler_pend_sv,           // 14 - PendSV
	irq_handler_sys_tick,          // 15 - SysTick
	TMR0_IRQHandler,             // 0:  TMR0
	GPIO_IRQHandler,             // 1:  GPIO
	SLAVE_IRQHandler,            // 2:  SLAVE
	SPI0_IRQHandler,             // 3:  SPI0
	BB_IRQHandler,               // 4:  BB
	LLE_IRQHandler,              // 5:  LLE
	USB_IRQHandler,              // 6:  USB
	ETH_IRQHandler,              // 7:  ETH
	TMR1_IRQHandler,             // 8:  TMR1
	TMR2_IRQHandler,             // 9:  TMR2
	UART0_IRQHandler,            // 0:  UART0
	UART1_IRQHandler,            // 1:  UART1
	RTC_IRQHandler,              // 2:  RTC
	ADC_IRQHandler,              // 3:  ADC
	SPI1_IRQHandler,             // 4:  SPI1
	LED_IRQHandler,              // 5:  LED
	TMR3_IRQHandler,             // 6:  TMR3 
	UART2_IRQHandler,            // 7:  UART2
	UART3_IRQHandler,            // 8:  UART3
	WDT_IRQHandler,              // 9:  WDT
};
