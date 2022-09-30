/*
----------------------------------------------------------------------

	AdAstra - Real Time Kernel

	Alain Chebrou

	hd44780.c	Simple support routines for HD44780 based LCD 1602 and 1604 (4x16 and 4x20)
				Use 3 command pins and 4 data pins placed anywhere
				Thread safe port output (use atomic GPIO access)
				Use LCD BUSY flag for the fastest operation
				Support custom character generator setting (CGRAM)
				Support 5x8 font only
				Tested on STM32F103 and STM32F429, ready for H7, L4, L4+

	When		Who	What
	11/05/20	ac	Creation
	22/05/12	ac	Cosmetic changes to remove compiler warning

----------------------------------------------------------------------
*/

#include	"aa.h"

#include	"hd44780.h"

//--------------------------------------------------------------------------------
//	LCD parameters

// Define only 1 display, according to your LCD
#define		HD_1602		1	// 2 lines 16 or 20 characters
#define		HD_1604_16	0	// 4 lines 16 characters
#define		HD_1604_20	0	// 4 lines 20 characters

// For 1602: 2 line display
#if (HD_1602 == 1)
	#define	HD_LN		2u		// Line count
	#define	HD_LMASK	0x01u	// Mask for line number
	#define	HD_L1		0x00u	// Start offset of line 1
	#define	HD_L2		0x40u	// Start offset of line 2

	static const uint8_t	hdOffsets [HD_LN] = { HD_L1, HD_L2 } ;
#endif

// For 1604_16: 4x16 display
#if (HD_1604_16 == 1)
	#define	HD_LN		4u		// Line count
	#define	HD_LMASK	0x03u	// Mask for line number
	#define	HD_L1		0x00u	// Start offset of line 1
	#define	HD_L2		0x40u	// Start offset of line 2
	#define	HD_L1		0x10u	// Start offset of line 1
	#define	HD_L1		0x50u	// Start offset of line 1

	static const uint8_t	hdOffsets [HD_LN] = { HD_L1, HD_L2, HD_L3, HD_L4  } ;
#endif

// For 1604_20: 4x20 display
#if (HD_1604_16 == 1)
	#define	HD_LN		4u		// Line count
	#define	HD_LMASK	0x03u	// Mask for line number
	#define	HD_L1		0x00u	// Start offset of line 1
	#define	HD_L2		0x40u	// Start offset of line 2
	#define	HD_L1		0x27u	// Start offset of line 1
	#define	HD_L1		0x67u	// Start offset of line 1

	static const uint8_t	hdOffsets [HD_LN] = { HD_L1, HD_L2, HD_L3, HD_L4  } ;
#endif


//--------------------------------------------------------------------------------
// Define pins, and macros to set these pins
// Modify these definitions according to your wiring

#if defined (STM32F1)

#define	RS_PORT		GPIOA
#define	RS_PIN		10

#define	RW_PORT		GPIOC
#define	RW_PIN		4

#define	E_PORT		GPIOB
#define	E_PIN		5

#define	D4_PORT		GPIOB
#define	D4_PIN		13

#define	D5_PORT		GPIOB
#define	D5_PIN		15

#define	D6_PORT		GPIOB
#define	D6_PIN		14

#define	D7_PORT		GPIOB
#define	D7_PIN		10

#else

#define	RS_PORT		GPIOE
#define	RS_PIN		2

#define	RW_PORT		GPIOE
#define	RW_PIN		4

#define	E_PORT		GPIOE
#define	E_PIN		5

#define	D4_PORT		GPIOE
#define	D4_PIN		6

#define	D5_PORT		GPIOE
#define	D5_PIN		3

#define	D6_PORT		GPIOF
#define	D6_PIN		8

#define	D7_PORT		GPIOF
#define	D7_PIN		7

#endif	// STM32F1

//--------------------------------------------------------------------------------
// Some STM32 families have a BRR register, others do not.
// On some families BRR and BSRR are 16 bits registers, but they are defined as one 32 bits register. Sigh...
// Some families allows GPIO register access as 16 or bits, others as 32 bits only.
// What a mess..
//	F1   : BSRR and BRR 32 bits only
//	F4   : BSRR is a 2x16 bits register, access as 16 or 32 bits
//	H7   : BSRR is a 2x16 bits register, access as 16 or 32 bits
//	L4(+): BSRR and BRR, 16 or 32 bits access (The most flexible)
//	BSRR 32 bits is common to all these families.

// All this is resolved into constants at compile time to get the minimal code.

#define	rsSet()		RS_PORT->BSRR = (1u << RS_PIN)
#define	rsClear()	RS_PORT->BSRR = (1u << (RS_PIN + 16))

#define	rwSet()		RW_PORT->BSRR = (1u << RW_PIN)
#define	rwClear()	RW_PORT->BSRR = (1u << (RW_PIN + 16))

#define	eSet()		E_PORT->BSRR  = (1u << E_PIN)
#define	eClear()	E_PORT->BSRR  = (1u << (E_PIN + 16))

#define	d4Set()		D4_PORT->BSRR = (1u << D4_PIN)
#define	d4Clear()	D4_PORT->BSRR = (1u << (D4_PIN + 16))

#define	d5Set()		D5_PORT->BSRR = (1u << D5_PIN)
#define	d5Clear()	D5_PORT->BSRR = (1u << (D5_PIN + 16))

#define	d6Set()		D6_PORT->BSRR = (1u << D6_PIN)
#define	d6Clear()	D6_PORT->BSRR = (1u << (D6_PIN + 16))

#define	d7Set()		D7_PORT->BSRR = (1u << D7_PIN)
#define	d7Clear()	D7_PORT->BSRR = (1u << (D7_PIN + 16))

//--------------------------------------------------------------------------------
#if defined (STM32F1) || defined (STM32L1)
// For STM32F1xx MCU

// Functions to initialize the GPIO and switch D7 pin mode
// Must be adapted to your MCU

#define		MODE_IN		0x04u
#define		MODE_OUT	0x02u
#define		MODE_MASK	0x0Fu

//--------------------------------------------------------------------------------
// Configure GPIO pins to output

static void hdGpioInit (GPIO_TypeDef * pGpio, uint32_t pin)
{
	// Enable port clock (without pointer arithmetic...)
	RCC->APB2ENR |= 1u << ((((uint32_t) pGpio - (uint32_t) GPIOA) / ((uint32_t) GPIOB - (uint32_t) GPIOA)) + RCC_APB2ENR_IOPAEN_Pos) ;

	// Initialize GPIO as output
	if (pin < 8)
	{
		pGpio->CRL = (pGpio->CRL & ~(MODE_MASK << (pin * 4))) | (MODE_OUT << (pin * 4)) ;
	}
	else
	{
		pGpio->CRH = (pGpio->CRH & ~(MODE_MASK << ((pin - 8) * 4))) | (MODE_OUT << ((pin - 8) * 4)) ;
	}
	pGpio->BRR = 1u << pin ;	// Pin output low
}

//--------------------------------------------------------------------------------
// To switch D7 to input, allows to check the HD44780 busy flag.
// BEWARE: not atomic

__ALWAYS_STATIC_INLINE	void	d7IsInput (void)
{
	#if (D7_PIN < 8)
	{
		D7_PORT->CRL = (D7_PORT->CRL & ~(MODE_MASK << (D7_PIN * 4))) | (MODE_IN << (D7_PIN * 4)) ;
	}
	#else
	{
		D7_PORT->CRH = (D7_PORT->CRH & ~(MODE_MASK << ((D7_PIN - 8) * 4))) | (MODE_IN << ((D7_PIN - 8) * 4)) ;
	}
	#endif
}

//--------------------------------------------------------------------------------
// To switch D7 to output
// BEWARE: not atomic

__ALWAYS_STATIC_INLINE	void	d7IsOutput (void)
{
	#if (D7_PIN < 8)
	{
		D7_PORT->CRL = (D7_PORT->CRL & ~(MODE_MASK << (D7_PIN * 4))) | (MODE_OUT << (D7_PIN * 4)) ;
	}
	#else
	{
		D7_PORT->CRH = (D7_PORT->CRH & ~(MODE_MASK << ((D7_PIN - 8) * 4))) | (MODE_OUT << ((D7_PIN - 8) * 4)) ;
	}
	#endif
}

//--------------------------------------------------------------------------------
#elif defined (STM32F4)  ||  defined (STM32L4)  ||  defined (STM32H7)
//--------------------------------------------------------------------------------

#define		IO_SPEED	0x01u	// Medium
#define		MODE_IN		0x00u	// Input
#define		MODE_OUT	0x01u	// Output
#define		MODE_MASK	0x03u

//--------------------------------------------------------------------------------
// Configure GPIO pins to output

static void hdGpioInit (GPIO_TypeDef * pGpio, uint32_t pin)
{
	uint32_t	temp ;

	// Enable port clock (without pointer arithmetic...)
#if defined (STM32F4)
	RCC->AHB1ENR |= 1u << ((((uint32_t) pGpio - (uint32_t) GPIOA) / ((uint32_t) GPIOB - (uint32_t) GPIOA)) + RCC_AHB1ENR_GPIOAEN_Pos) ;
#elif defined (STM32H7)
	RCC->AHB4ENR |= 1u << ((((uint32_t) pGpio - (uint32_t) GPIOA) / ((uint32_t) GPIOB - (uint32_t) GPIOA)) + RCC_AHB4ENR_GPIOAEN_Pos) ;
#elif defined (STM32L4)
	//	On STM32L4+ the pins PG[15:2] are powered by VDDIO2, and need to be connected before use...
	#if defined(PWR_CR2_IOSV)
		if ((pGpio == GPIOG)  &&  (pin > 1u))
		{
			volatile uint32_t	reg ;

			RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN ;
			reg = RCC->APB1ENR1 ;	// Delay after an RCC peripheral clock enabling
			(void) reg ;

			PWR->CR2 |= PWR_CR2_IOSV ;
		}
	#endif
	RCC->AHB2ENR |= 1u << ((((uint32_t) pGpio - (uint32_t) GPIOA) / ((uint32_t) GPIOB - (uint32_t) GPIOA)) + RCC_AHB2ENR_GPIOAEN_Pos) ;
#else
	#error "MCU Not defined"
#endif

	// Configure the IO Speed : medium
	temp = pGpio->OSPEEDR ;
	temp &= ~(GPIO_OSPEEDR_OSPEED0 << (pin * 2u)) ;
	temp |= IO_SPEED << (pin * 2u) ;
	pGpio->OSPEEDR = temp ;

	// Configure the IO Output Type : PP
	temp = pGpio->OTYPER ;
	temp &= ~(GPIO_OTYPER_OT0 << pin) ;
	pGpio->OTYPER = temp ;

	// No PU nor PD
	temp = pGpio->PUPDR ;
	temp &= ~(GPIO_PUPDR_PUPD0 << (pin * 2u)) ;
	pGpio->PUPDR = temp ;

	// Configure Alternate function 0
	temp = pGpio->AFR[pin >> 3u] ;
	temp &= ~((uint32_t) 0x0Fu << ((pin & 0x07u) * 4u)) ;
	pGpio->AFR[pin >> 3u] = temp ;

	// Configure to output
	temp = pGpio->MODER ;
	temp &= ~(GPIO_MODER_MODE0 << (pin * 2u)) ;
	temp |= MODE_OUT << (pin * 2u) ;
	pGpio->MODER = temp ;

	pGpio->BSRR = 1u << (pin + 16) ;	// Pin output low
}

//--------------------------------------------------------------------------------
// To switch D7 to input, allows to check the HD44780 busy flag.
// BEWARE: not atomic

__ALWAYS_STATIC_INLINE	void	d7IsInput (void)
{
	D7_PORT->MODER = (D7_PORT->MODER & ~(MODE_MASK << (D7_PIN * 2))) | (MODE_IN << (D7_PIN * 2)) ;
}

//--------------------------------------------------------------------------------
// To switch D7 to output
// BEWARE: not atomic

__ALWAYS_STATIC_INLINE	void	d7IsOutput (void)
{
	D7_PORT->MODER = (D7_PORT->MODER & ~(MODE_MASK << (D7_PIN * 2))) | (MODE_OUT << (D7_PIN * 2)) ;
}

#else
	#error "MCU Not defined"
#endif

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

static	uint8_t hdCtrl ;	// Local copy of Display On/Off Control Register
							// Allow to individually set cursor/blink/screen on-off

//--------------------------------------------------------------------------------
// Wait for LCD busy flag 0

static	void	hdWaitForBusy ()
{
	uint32_t	flag ;

	// Switch D7 to input
	d7IsInput () ;

	// Set to command read
	rsClear () ;
	rwSet () ;

	do
	{
		// Read high nibble
		bspDelayUs (1) ;
		eSet () ;
		bspDelayUs (1) ;
		flag = D7_PORT->IDR & (1 << D7_PIN) ;
		eClear () ;

		// Read low nibble
		bspDelayUs (1) ;
		eSet () ;
		bspDelayUs (1) ;
		eClear () ;
	} while (flag != 0) ;

	// Set default state: write mode and D7 as output
	rwClear () ;
	d7IsOutput () ;
}

//--------------------------------------------------------------------------------
// Generate E pulse

static void ePulse (void)
{
	bspDelayUs (1) ;	// For data setup time
	eSet () ;
	bspDelayUs (1) ;
	eClear () ;
}

//--------------------------------------------------------------------------------
// Set low nibble of data to the data output port

static void setDataPort (uint8_t data)
{
	if ((data & 0x08u) == 0u)
	{
		d7Clear () ;
	}
	else
	{
		d7Set () ;
	}

	if ((data & 0x04u) == 0u)
	{
		d6Clear () ;
	}
	else
	{
		d6Set () ;
	}

	if ((data & 0x02u) == 0u)
	{
		d5Clear () ;
	}
	else
	{
		d5Set () ;
	}

	if ((data & 0x01u) == 0u)
	{
		d4Clear () ;
	}
	else
	{
		d4Set () ;
	}
}

//--------------------------------------------------------------------------------
// Send data to LCD

void hdData (uint8_t cc)
{
	setDataPort (cc >> 4) ;	// get upper nibble
	rsSet () ;				// set LCD to data mode
	ePulse () ;				// E strobe

	setDataPort (cc) ;		// get down nibble
	ePulse () ;				// E strobe

	hdWaitForBusy () ;
}

//--------------------------------------------------------------------------------
// Send command to LCD

static	void hdCmd (uint8_t cc)
{
	setDataPort (cc >> 4) ;		// Get upper nibble
	rsClear () ;				// Set LCD to instruction mode
	ePulse () ;					// E strobe

	setDataPort (cc) ;			// Get lower nibble
	ePulse () ;					// E strobe

	hdWaitForBusy () ;
}

//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Set cursor position
// Beware : no check on line and column values

void	hdSetPos (uint32_t line, uint32_t col)
{
	uint8_t	pos ;

	pos = (uint8_t) (hdOffsets [line & HD_LMASK] + col) ;
	hdCmd (0x80u | pos) ;
}

//--------------------------------------------------------------------------------
// Set LCD on (mode=HD_SCREEN_ON) or off (mode=HD_SCREEN_OFF)

void    hdScreenOn (uint32_t mode)
{
	hdCtrl &= (uint8_t) ~0x04u ;
	if (mode != HD_SCREEN_OFF)
	{
		hdCtrl |= 0x04u ;
	}
	hdCmd (hdCtrl) ;
}

//--------------------------------------------------------------------------------
// Set cursor blink (mode=HD_BINK_ON) or off (mode=HD_BINK_OFF)

void    hdBlinkOn (uint32_t mode)
{
	hdCtrl &= (uint8_t) ~0x01u ;
	if (mode != HD_BLINK_OFF)
	{
		hdCtrl |= 0x01u ;
	}
	hdCmd (hdCtrl) ;
}

//--------------------------------------------------------------------------------
// Set cursor on (mode=HD_CUR_ON) or off (mode=HD_CUR_OFF)

void    hdCursorOn (uint32_t mode)
{
	hdCtrl &= (uint8_t) ~0x02u ;
	if (mode != HD_CUR_OFF)
	{
		hdCtrl |= 0x02u ;
	}
	hdCmd (hdCtrl) ;
}

//--------------------------------------------------------------------------------
// LCD clear

void    hdClear (void)
{
	hdCmd (1u) ;
}

//--------------------------------------------------------------------------------
// Return home, slow because also resets the shift

void    hdHome		(void)
{
	hdCmd (2u) ;
}

//--------------------------------------------------------------------------------
// Configure character pattern generator
// Index: 0..7
// pData: pointer to 8 bytes

void    hdCgram		(uint32_t index, const uint8_t * pData)
{
	uint32_t	ii ;

	hdCmd ((uint8_t) (0x40u + (index * 8u))) ;	// Set CGRAM address
	for (ii = 0u ; ii < 8u ; ii++)
	{
		hdData (pData [ii]) ;
	}
	hdCmd (0x80u) ;	// Set DDRAM address: home
}

//--------------------------------------------------------------------------------
// Display a 0 terminated string

void	hdStr (const char * pStr)
{
	char cc ;

    while ((cc = *pStr++) != 0u)
    {
        hdData (cc) ;
    }
}

//--------------------------------------------------------------------------------
// Initialize the LCD device

void hdInit (void)
{
	// Initialize GPIO ports
	hdGpioInit (RS_PORT, RS_PIN) ;
	hdGpioInit (RW_PORT, RW_PIN) ;
	hdGpioInit (E_PORT, E_PIN) ;
	hdGpioInit (D4_PORT, D4_PIN) ;
	hdGpioInit (D5_PORT, D5_PIN) ;
	hdGpioInit (D6_PORT, D6_PIN) ;
	hdGpioInit (D7_PORT, D7_PIN) ;

	// Reset HD44780
	rsClear () ;
	rwClear () ;
	bspDelayUs (40000) ;	// Min 40ms

	setDataPort (0x03u) ;
	ePulse () ;
	bspDelayUs (6000) ;		// Min 5ms

	setDataPort (0x03u) ;
	ePulse () ;
	bspDelayUs (200) ;		// Min 100 us

	setDataPort (0x03u) ;
	ePulse () ;
	bspDelayUs (200) ;

	setDataPort (0x02u) ;	// Enable 4-Bit Mode
	ePulse () ;
	hdWaitForBusy () ;

	hdCmd (0x28) ;			// Set Interface Length: 4 bits, 2 lines, 5x7 font
	hdCmd (0x08) ;			// Display off
	hdCmd (0x01) ;			// Clear display
	hdCtrl = 0x0Cu ;		// Display on, cursor off, no blink
	hdCmd (hdCtrl) ;
	hdCmd (0x06) ;			// Entry mode: Move cursor after each data
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//	HD44780 test

#include	"aa.h"
#include	"aaprintf.h"

// Define 4 characters for the CGRAM

static const	uint8_t	c0 [8] = { 0x00, 0x01, 0x02, 0x04, 0x04, 0x08, 0x10, 0x00 } ;
static const	uint8_t	c1 [8] = { 0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00 } ;
static const	uint8_t	c2 [8] = { 0x00, 0x10, 0x08, 0x04, 0x04, 0x02, 0x01, 0x00 } ;
static const	uint8_t	c3 [8] = { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00 } ;

void	hd44780Test ()
{
	uint32_t	cc ;

	while (1)
	{
		aaPrintf ("i   Init\n") ;
		aaPrintf ("z   Clear screen\n") ;
		aaPrintf ("1   Set position line 1\n") ;
		aaPrintf ("2   Set position line 2\n") ;
		aaPrintf ("s   Display string\n") ;
		aaPrintf ("O/o Screen On/Off\n") ;
		aaPrintf ("C/c Cursor On/off\n") ;
		aaPrintf ("B/n Blink On/off\n") ;
		aaPrintf ("g   Configure CGRAM\n") ;
		aaPrintf ("q   quit\n") ;
		aaPrintf ("Select: ") ;
		cc = aaGetChar () ;
		aaPutChar ((char) cc) ;
		aaPutChar ('\n') ;

		if (cc == 'q')
		{
			break ;
		}

		switch (cc)
		{
			case 'i':
				hdInit () ;
				break ;

			case 'z':
				hdClear () ;
				break ;

			case '1':
				hdSetPos (0, 0) ;
				break ;

			case '2':
				hdSetPos (1, 0) ;
				break ;

			case 'O':
				hdScreenOn (1) ;
				break ;

			case 'o':
				hdScreenOn (0) ;
				break ;

			case 'C':
				hdCursorOn (1) ;
				break ;

			case 'c':
				hdCursorOn (0) ;
				break ;

			case 'B':
				hdBlinkOn (1) ;
				break ;

			case 'b':
				hdBlinkOn (0) ;
				break ;

			case 's':
				hdStr ("  AdAstra-RTK") ;
				break ;

			case 'g':
				hdCgram (0, c0) ;
				hdCgram (1, c1) ;
				hdCgram (2, c2) ;
				hdCgram (3, c3) ;
				cc = 0 ;
				while (aaCheckGetChar () == 0u)
				{
					hdSetPos (0, 7) ;
					hdData ((uint8_t) cc) ;
					cc = (cc + 1) & 0x03u ;
					aaTaskDelay (bspGetTickRate () / 4) ;
				}
				(void) aaGetChar () ;
				break ;

			default:
				break ;
		}
	}
}

//--------------------------------------------------------------------------------
