/*
 * Copyright (C) 2008, 2009, 2010 Kaspar Schleiser <kaspar@schleiser.de>
 * Copyright (C) 2013 INRIA
 * Copyright (C) 2013 Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Default application that shows a lot of functionality of RIOT
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "thread.h"

#include "xtimer.h"
#include "periph/gpio.h"
#include "periph/spi.h"
#include "ucg.h"

   static gpio_t pins[] = {
	[UCG_PIN_CS] = GPIO5,			// Chip select
	[UCG_PIN_CD] = GPIO14,			// Command/Data or A0 
	[UCG_PIN_RST] = GPIO2			// Reset pin
    };

   static uint32_t pins_enabled = (
	(1 << UCG_PIN_CS) +
	(1 << UCG_PIN_CD) +
	(1 << UCG_PIN_RST)
    );
#define TEXT_LU "Temperatur:"	//text left up
#define TEXT_RU	"Humidity:"	//text right up
#define TEXT_LD "Time:"		//text left down
#define TEXT_RD "State:"		//text right down
#define DISP_WIDTH 128
#define DISP_HIGHT 128
#define TITLE_HIGHT 6
#define TEXT_OFFSET_W 2
#define TEXT_OFFSET_H 4
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
		ucg_SetFont(&ucg, ucg_font_timB08_hf);	// change font 
		// Upper part                
		ucg_DrawString(&ucg, TEXT_OFFSET_W, (TITLE_HIGHT + TEXT_OFFSET_H), 0, TEXT_LU);
		ucg_DrawRFrame(&ucg, 0, 0, DISP_WIDTH/2, DISP_HIGHT/2, 3);
		ucg_DrawRFrame(&ucg, DISP_WIDTH/2, 0, DISP_WIDTH/2, DISP_HIGHT/2, 3);
		uint32_t width = (uint32_t) DISP_WIDTH - (ucg_GetStrWidth(&ucg, TEXT_RU)+TEXT_OFFSET_W);
		ucg_DrawString(&ucg, width, (TITLE_HIGHT + TEXT_OFFSET_H), 0, TEXT_RU);
		// Down part		
		ucg_DrawRFrame(&ucg, 0,DISP_HIGHT/2,DISP_WIDTH/2 , DISP_HIGHT/2, 3);
		ucg_DrawRFrame(&ucg, (DISP_WIDTH/2), DISP_HIGHT/2, DISP_WIDTH/2, DISP_HIGHT/2, 3);
	
		ucg_DrawString(&ucg, TEXT_OFFSET_W, ((DISP_HIGHT/2) + TITLE_HIGHT + TEXT_OFFSET_H), 0, TEXT_LD);

		width = (uint32_t) DISP_WIDTH - (ucg_GetStrWidth(&ucg, TEXT_RD) + TEXT_OFFSET_W);
		ucg_DrawString(&ucg, width, ((DISP_HIGHT/2) + TITLE_HIGHT + TEXT_OFFSET_H), 0, TEXT_RD);
       		/* show screen in next iteration */
        	screen += 1;
	 	break;
            case 1:
		ucg_SetFont(&ucg, ucg_font_helvB12_tf);	// change font
                ucg_SetColor(&ucg, 0, 63, 167, 136);
                ucg_DrawString(&ucg, TEXT_OFFSET_W+ 10, 50, 0, "35C");
                break;
        }



        /* sleep a little */
        xtimer_sleep(1);
    }

    return 0;
}
