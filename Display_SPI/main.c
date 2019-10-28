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

#define TEXT_UL "Temperatur:"	//text left up
#define TEXT_UR	"Humidity:"	//text right up
#define TEXT_DL "Time:"		//text left down
#define TEXT_DR "State:"	//text right down
#define DISP_WIDTH 128		//display width
#define DISP_HIGHT 128		//display hight
#define TITLE_HIGHT 6		//groesse der Titelschrift
#define TEXT_OFFSET_W 2		//die Rahmen um die boxen erzeugen einen offset wo keine Schrift rein kann. (in der breite)
#define TEXT_OFFSET_H 4		//die Rahmen um die boxen erzeugen einen offset wo keine Schrift rein kann. (in der hoehe)

void title_ul(ucg_t* ucg, char text[]);
void title_ur(ucg_t* ucg, char text[]);
void title_dl(ucg_t* ucg, char text[]);
void title_dr(ucg_t* ucg, char text[]);

void variable_ul(ucg_t* ucg, char text[]);
void variable_ur(ucg_t* ucg, char text[]);
void variable_dl(ucg_t* ucg, char text[]);
void variable_dr(ucg_t* ucg, char text[]);

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
		// Start up Display
		 ucg_SetColor(&ucg, 0, 189, 32, 43);

		title_ul(&ucg, TEXT_UL);
		//title_ur(&ucg, TEXT_UR);
		//title_dl(&ucg, TEXT_DL);
		title_dr(&ucg, TEXT_DR);
       		/* show screen in next iteration */
        	screen += 1;
	 	break;
            case 1:
		// Start up Display
		// change Color	
		ucg_SetColor(&ucg, 0, 63, 167, 136);
		variable_ul(&ucg, "35 C");
		screen += 1;		
                break;
	    case 2:
		variable_ul(&ucg, " ");
		screen = 0;
        }



        /* sleep a little */
        xtimer_sleep(1);
    }

    return 0;
}
			// Text Position
			/*      -------------------
			*  	- 1.	-	2.-
			*	-------------------
			*  	- 3.	-	4.-
			*	-------------------
			*/
//1. Title Upper Left
void title_ul(ucg_t* ucg, char text[])
{	
	// change font
	ucg_SetFont(ucg, ucg_font_timB08_hf);	
	// draw frame
	ucg_DrawRFrame(ucg, 0, 0, DISP_WIDTH/2, DISP_HIGHT/2, 3);
	// draw text
	ucg_DrawString(ucg, TEXT_OFFSET_W, (TITLE_HIGHT + TEXT_OFFSET_H), 0, text);
}

//2. Title Upper Right
void title_ur(ucg_t* ucg, char text[])
{	
	// change font
	ucg_SetFont(ucg, ucg_font_timB08_hf);	
	// draw frame
	ucg_DrawRFrame(ucg, DISP_WIDTH/2, 0, DISP_WIDTH/2, DISP_HIGHT/2, 3);
	// draw text
	uint32_t position = (uint32_t) DISP_WIDTH - (ucg_GetStrWidth(ucg, text)+TEXT_OFFSET_W);
	ucg_DrawString(ucg, position, (TITLE_HIGHT + TEXT_OFFSET_H), 0, text);	

}

//3. Title Down Left
void title_dl(ucg_t* ucg, char text[])
{	
	// change font 
	ucg_SetFont(ucg, ucg_font_timB08_hf);	
	// draw frame
	ucg_DrawRFrame(ucg, 0,DISP_HIGHT/2,DISP_WIDTH/2 , DISP_HIGHT/2, 3);
	// draw text
	ucg_DrawString(ucg, TEXT_OFFSET_W, ((DISP_HIGHT/2) + TITLE_HIGHT + TEXT_OFFSET_H), 0, text);
}
//4. Title Down Right
void title_dr(ucg_t* ucg, char text[])
{	
	// change font 
	ucg_SetFont(ucg, ucg_font_timB08_hf);	
	// draw frame	
	ucg_DrawRFrame(ucg, (DISP_WIDTH/2), DISP_HIGHT/2, DISP_WIDTH/2, DISP_HIGHT/2, 3);
	// draw text	
	uint32_t position = (uint32_t) DISP_WIDTH - (ucg_GetStrWidth(ucg, text) + TEXT_OFFSET_W);
	ucg_DrawString(ucg, position, ((DISP_HIGHT/2) + TITLE_HIGHT + TEXT_OFFSET_H), 0, text);
}


			// Variable Position
			/*      -------------------
			*  	- 1.1	-      2.1-
			*	-------------------
			*  	- 3.1	-      4.1-
			*	-------------------
			*/
//1.1 Variable Upper Left
void variable_ul(ucg_t* ucg, char text[])
{	
	if(text
	// change font
	ucg_SetFont(ucg, ucg_font_helvB12_tf);	
	// draw text
	ucg_DrawString(ucg, TEXT_OFFSET_W+ 10, 50, 0, text);


}
//2.1 Variable Upper Right
void variable_ur(ucg_t* ucg, char text[])
{	
	// change font
	ucg_SetFont(ucg, ucg_font_timB08_hf);	
	// draw frame
	ucg_DrawRFrame(ucg, DISP_WIDTH/2, 0, DISP_WIDTH/2, DISP_HIGHT/2, 3);
	// draw text
	uint32_t width = (uint32_t) DISP_WIDTH - (ucg_GetStrWidth(ucg, text)+TEXT_OFFSET_W);
	ucg_DrawString(ucg, width, (TITLE_HIGHT + TEXT_OFFSET_H), 0, text);	

}

//3.1 Variable Down Left
void variable_dl(ucg_t* ucg, char text[])
{	
	// change font 
	ucg_SetFont(ucg, ucg_font_timB08_hf);	
	// draw frame
	ucg_DrawRFrame(ucg, 0,DISP_HIGHT/2,DISP_WIDTH/2 , DISP_HIGHT/2, 3);
	// draw text
	ucg_DrawString(ucg, TEXT_OFFSET_W, ((DISP_HIGHT/2) + TITLE_HIGHT + TEXT_OFFSET_H), 0, text);
}
//4.1 Variable Down Right
void variable_dr(ucg_t* ucg, char text[])
{	
	// change font 
	ucg_SetFont(ucg, ucg_font_timB08_hf);	
	// draw frame	
	ucg_DrawRFrame(ucg, (DISP_WIDTH/2), DISP_HIGHT/2, DISP_WIDTH/2, DISP_HIGHT/2, 3);
	// draw text	
	uint32_t width = (uint32_t) DISP_WIDTH - (ucg_GetStrWidth(ucg, text) + TEXT_OFFSET_W);
	ucg_DrawString(ucg, width, ((DISP_HIGHT/2) + TITLE_HIGHT + TEXT_OFFSET_H), 0, text);
}
