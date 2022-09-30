/*
----------------------------------------------------------------------

	AdAstra - Real Time Kernel

	Alain Chebrou

	hd44780.h	Simple support routines for HD44780 based LCD (1602/1604)

	When		Who	What
	11/05/20	ac	Creation

----------------------------------------------------------------------
*/

#ifndef	HD44780_H_
#define HD44780_H_
//--------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

#define	HD_SCREEN_OFF	0
#define	HD_SCREEN_ON	1

#define	HD_BLINK_OFF	0
#define	HD_BLINK_ON		1

#define	HD_CUR_OFF		0
#define	HD_CUR_ON		1

void	hdInit			(void) ;
void	hdSetPos		(uint32_t line, uint32_t col) ;
void    hdScreenOn		(uint32_t mode) ;
void    hdBlinkOn		(uint32_t mode) ;
void    hdCursorOn		(uint32_t mode) ;
void    hdClear			(void) ;
void    hdHome			(void) ;
void    hdCgram			(uint32_t index, const uint8_t * pData) ;
void	hdData			(uint8_t cc) ;
void	hdStr			(const char * pStr) ;

#ifdef __cplusplus
}
#endif
//--------------------------------------------------------------------------------
#endif // HD44780_H_
