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
 * @brief       SPI Display for the basestation with buttons
 *
 * @author      Martin Pyka
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disp.h"
#include "thread.h"
#include "xtimer.h"
#include "logo.h"

// Text Beispiele:
#define TEXT_UL "Temperatur:" //text left up
#define TEXT_UR "Humitity:"   //text right up
#define TEXT_DL "Time:"		  //text left down
#define TEXT_DR "State:"	  //text right down

int main(void)
{
	static showText_t showDown[28];
	ucg_t ucg;
	uint8_t localPage = get_currPage();

	init_disp(&ucg);
	init_buttons(&ucg);

	addParam(&showDown[0], TEXT_UL, "1234567889");
	addParam(&showDown[1], TEXT_UR, "2");
	addParam(&showDown[2], TEXT_DL, "3");
	addParam(&showDown[3], TEXT_DR, "4");
	addParam(&showDown[4], "NextSite", "5");
	addParam(&showDown[5], "FullSide", "9");
	addParam(&showDown[6], "FullSide", "2");
	addParam(&showDown[7], "qw", "3");
	addParam(&showDown[8], "FullSide", "123");
	addParam(&showDown[9], "q", "4467");
	addParam(&showDown[10], "FullSide", "33222");
	addParam(&showDown[11], "re", "12356");
	addParam(&showDown[12], "FulldfSide", "433445");
	addParam(&showDown[13], "vx", "2234567");
	addParam(&showDown[14], "kglr", "89777");
	addParam(&showDown[15], "zziioosa", "5544");
	addParam(&showDown[16], "ffddss", "3456");
	addParam(&showDown[17], "sdccvbbb", "78866");
	addParam(&showDown[18], "sdfkfgrr", "33558");
	addParam(&showDown[19], "dfgksdkfr", "2221");
	addParam(&showDown[20], "daeglfkg", "2");
	addParam(&showDown[21], "ddc", "345");
	addParam(&showDown[22], "fgf", "678");
	addParam(&showDown[23], "izih", "44664");
	addParam(&showDown[24], "isaih", "4123664");
	addParam(&showDown[25], "iasfzih", "44123664");

	puts("Starting Pages");
	int test = 0;
	changePage(&ucg, localPage);
	while (1)
	{
		if (localPage != get_currPage())
		{
			localPage = get_currPage();
			changePage(&ucg, localPage);

			char text[40];
			sprintf(text, "Upd:%d", test);
			changeVar(&ucg, &showDown[1], text);
			test %= 100;
			test++;
		}
	}
	return 0;
}