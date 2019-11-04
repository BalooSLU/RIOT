/*
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @brief       SPI Display first design for the basestation with static values
 *
 * @author      Martin Pyka
 * @}
 */

#include <stdio.h>
#include <string.h>
#include "thread.h"
#include "xtimer.h"
#include "periph/gpio.h"
#include "periph/spi.h"
#include "ucg.h"
#include "logo.h"

#define TEXT_UL "Temp:"		//text left up
#define TEXT_UR	"Humi:"		//text right up
#define TEXT_DL "Time:"		//text left down
#define TEXT_DR "State:"	//text right down
#define DISP_WIDTH 128		//display width
#define DISP_HIGHT 128		//display hight
#define VAR_TEXT_HIGHT 12	//groesse der Variablenschrift
#define TITLE_TEXT_HIGHT 8	//groesse der Titelschrift
#define FRAME_EGDE_RADIUS 3	//Radius for the four edges.
#define TEXT_OFFSET_W 3		//die Rahmen um die boxen erzeugen einen offset wo keine Schrift rein kann. (in der breite)
#define TEXT_OFFSET_H 3		//die Rahmen um die boxen erzeugen einen offset wo keine Schrift rein kann. (in der hoehe)
#define COLS 2
#define LINES 2
#define FRAME_WIDTH (DISP_WIDTH / COLS)
#define FRAME_HIGHT (DISP_HIGHT / LINES)
#define VAR_TEXT_LENGTH_MAX (FRAME_WIDTH - 2 *TEXT_OFFSET_W)   
#define VAR_TEXT_POSITION_H (FRAME_HIGHT/2 + VAR_TEXT_HIGHT)

// text_check MODES
#define MODE_VARIABLE 1
#define MODE_TITLE    2

void frame(ucg_t* ucg, int line, int colum);
void title(ucg_t* ucg, char text[], int line, int colum);
void variable(ucg_t* ucg, char text[], int line, int colum);
void clearVar(ucg_t* ucg, int line, int colum);
void check_text(ucg_t* ucg, char *quelle, char *ziel);

static gpio_t pins[] = {
	[UCG_PIN_CS] = GPIO5,			// Chip select
	[UCG_PIN_CD] = GPIO14,			// Command/Data or A0 
	[UCG_PIN_RST] = GPIO2			// Reset pin
};
			// Pin out for ESP-WROOM-32
			/* 	Diplay 	-	MCU
			*-----------------------------
			*	LED	-	3V3
			*  	SCK	-	D18
			*  	SDA	-	D23
			*  	A0	-	D14
			*  	RESET	-	D2
			*  	CS	-	D5
			*  	GND	-	GND
			*  	VCC	-	VIN
			*/
static uint32_t pins_enabled = (
	(1 << UCG_PIN_CS) +
	(1 << UCG_PIN_CD) +
	(1 << UCG_PIN_RST)
 );

int main(void)
{
    uint32_t screen = 0;
    ucg_t ucg;


    ucg_SetPins(&ucg, pins, pins_enabled);
    ucg_SetDevice(&ucg, SPI_DEV(0));
    
    ucg_Init(&ucg, ucg_dev_st7735_18x128x160, ucg_ext_st7735_18, ucg_com_riotos_hw_spi);
 
/* initialize the display */
    puts("Initializing display.");

    ucg_SetFontMode(&ucg, UCG_FONT_MODE_TRANSPARENT);

    /* start drawing in a loop */
    puts("Drawing on Display.");

    ucg_ClearScreen(&ucg);
    ucg_SetFontPosBaseline(&ucg);

    while (1) {
       
        switch (screen) {
            case 0:
		ucg_ClearScreen(&ucg);
		// Start up Display
		frame(&ucg, 0, 0);
		frame(&ucg, 0, 1);
		frame(&ucg, 1, 0);
		frame(&ucg, 1, 1);
		title(&ucg, TEXT_UL, 0, 0);
		title(&ucg, TEXT_UR, 0, 1);
		title(&ucg, TEXT_DL, 1, 0);
		title(&ucg, TEXT_DR, 1, 1);
       		/* show screen in next iteration */
        	screen += 1;
	 	break;
            case 1:
		variable(&ucg, "1234567889", 0,0);
		variable(&ucg, "2", 0,1);
		variable(&ucg, "3", 1,0);
		variable(&ucg, "4", 1,1);
		screen = 0;		
                break;
	    case 2:
		break;
	     case 3:
		//Riot Logo test only
		ucg_ClearScreen(&ucg);
		for (int y = 0; y < 48; y++) {
                    for (int x = 0; x < 96; x++) {
                        uint32_t offset = (x + (y * 96)) * 3;

                        ucg_SetColor(&ucg, 0, logo[offset + 2], logo[offset + 1],logo[offset + 0] );
                        ucg_DrawPixel(&ucg, x, y);
                    }
		}
		screen = 0;
		break;
        }



        /* sleep a little */
        xtimer_sleep(1);
    }

    return 0;
}
//1. Draw Frame 
void frame(ucg_t* ucg, int line, int colum)
{	
	ucg_SetColor(ucg, 0, 189, 32, 43);
	// draw frame
	ucg_DrawRFrame(ucg, colum * FRAME_WIDTH,  line * FRAME_HIGHT, FRAME_WIDTH, FRAME_HIGHT, FRAME_EGDE_RADIUS);
}
			// Text Position
			/*      --------------------
			*  	- 0.0	-	0.1-
			*	--------------------
			*  	- 1.0	-	1.1-
			*	--------------------
			*/
//2. Title Text 
void title(ucg_t* ucg, char text[], int line, int colum)
{	
	ucg_SetColor(ucg, 0, 189, 32, 43);	
	// change font
	ucg_SetFont(ucg, ucg_font_timB08_hf);		
	// draw text
	ucg_DrawString(ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + (TITLE_TEXT_HIGHT + TEXT_OFFSET_H), 0, text);
}

			// Variable Position
			/*      --------------------
			*  	- 0.0	-	0.1-
			*	--------------------
			*  	- 1.0	-	1.1-
			*	--------------------
			*/
//*3. Variable Text
void variable(ucg_t* ucg, char text[], int line, int colum)
{	
	//clear area
	clearVar(ucg, line, colum);	
	// change Color	
	ucg_SetColor(ucg, 0, 63, 167, 136);	
	// change font
	ucg_SetFont(ucg, ucg_font_helvB12_tf);
	char checked_text[] = " ";
	check_text(ucg, text, checked_text);
	ucg_DrawString(ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + VAR_TEXT_POSITION_H, 0, checked_text);
}

void clearVar(ucg_t* ucg, int line, int colum)
{
	ucg_SetColor(ucg, 0, 0, 0, 0);
	ucg_DrawBox(ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + (VAR_TEXT_POSITION_H - VAR_TEXT_HIGHT), VAR_TEXT_LENGTH_MAX, VAR_TEXT_HIGHT);
}

void check_text(ucg_t* ucg, char *quelle, char *ziel)
{	
	char out[16];
	strcpy(out, quelle);
	
	int str_length = strlen(quelle);
	while (ucg_GetStrWidth(ucg, out) >= FRAME_WIDTH)
	{	
		if(str_length > 1)
		{	
			str_length--;
			out[str_length] = '\0';
			strcat(out, ".");
			puts(out);
		}else break;
	}
 	strcpy(ziel, out);	
} 
