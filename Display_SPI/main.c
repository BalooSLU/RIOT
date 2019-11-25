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
#include "display.h"

int main(void)
{
	showText_t showDown[20];
	uint32_t screen = 0;
	ucg_t ucg;

	init_disp(&ucg);
	init_pins();
	
	addParam(&showDown[0], TEXT_UL, "1234567889");
	addParam(&showDown[1], TEXT_UR, "2");
	addParam(&showDown[2], TEXT_DL, "3");
	addParam(&showDown[3], TEXT_DR, "4");
	addParam(&showDown[4], "NextSite", "5");
	addParam(&showDown[5], "FullSide", "9");
	addParam(&showDown[6], "TestSide", "913");
	addParam(&showDown[7], "Tede", "903");
	addParam(&showDown[8], "Teide", "91");
	addParam(&showDown[9], "Tee", "91");
	addParam(&showDown[10], "Teside", "91");
	addParam(&showDown[11], "TSide", "91");
	addParam(&showDown[12], "Tide", "913");
	addParam(&showDown[13], "Teide", "913");

  while (1) {
		switch (screen) {
			case 0:
				changePage(&ucg, 0);
		    screen += 1;
		 		break;
		  case 1:
				changePage(&ucg, 1);
				screen += 1;		
		  	break;
			case 2:
				changePage(&ucg, 2);
				screen = 0;
				break;
			case 3:
					//Camera test only
				ucg_ClearScreen(&ucg);
				for (int y = 0; y < 48; y++) {
					for (int x = 0; x < 96; x++) {
						uint32_t offset = (x + (y * 96)) * 3;
						//BGR
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
