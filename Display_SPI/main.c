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
#include "shell.h"
#include "shell_commands.h"

#include "xtimer.h"
#include "periph/gpio.h"
#include "periph/spi.h"
#include "ucg.h"

   static gpio_t pins[] = {
	[UCG_PIN_CS] = SPI0_CS0,
	[UCG_PIN_CD] = GPIO14,
	[UCG_PIN_RST] = GPIO2
    };

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
    ucg_SetFont(&ucg, ucg_font_helvB12_tf);

    /* start drawing in a loop */
    puts("Drawing on screen.");

    while (1) {
        ucg_ClearScreen(&ucg);

        switch (screen) {
            case 0:
                ucg_SetColor(&ucg, 0, 189, 32, 43);
                ucg_DrawString(&ucg, 0, 20, 0, "THIS");
                break;
            case 1:
                ucg_SetColor(&ucg, 0, 63, 167, 136);
                ucg_DrawString(&ucg, 0, 20, 0, "IS");
                break;
        }


        /* show screen in next iteration */
        screen = (screen + 1) % 2;

        /* sleep a little */
        xtimer_sleep(1);
    }

    return 0;
}