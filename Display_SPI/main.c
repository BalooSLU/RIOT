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
#include <stdlib.h>
#include <string.h>
#include "thread.h"
#include "xtimer.h"
#include "periph/gpio.h"
#include "periph/spi.h"
#include "ucg.h"
#include "logo.h"

#define VAR_FONT ucg_font_7x14B_tf
#define TITLE_FONT ucg_font_timB08_hf
// Text Beispiele:
#define TEXT_UL "Temperatur:" //text left up
#define TEXT_UR "Humitity:"   //text right up
#define TEXT_DL "Time:"		  //text left down
#define TEXT_DR "State:"	  //text right down
// Display Eigenschaften
#define DISP_WIDTH 128 //display width
#define DISP_HIGHT 128 //display hight
// Text Eigenschaften
#define VAR_TEXT_HIGHT 10  //groesse der Variablenschrift
#define VAR_TEXT_PLUS 4	//unterer Bereich von "gyqj"
#define TITLE_TEXT_HIGHT 8 //groesse der Titelschrift
#define TEXT_OFFSET_W 3	//die Rahmen um die boxen erzeugen einen offset wo keine Schrift rein kann. (in der breite)
#define TEXT_OFFSET_H 3	//die Rahmen um die boxen erzeugen einen offset wo keine Schrift rein kann. (in der hoehe)
// Box Eigenschaften
#define FRAME_EGDE_RADIUS 3									   //Radius for the four edges.
#define COLS 2												   //Anzahl der boxen (in der breite)
#define LINES 2												   //Anzahl der boxen (in der hoehe)
#define QBOXES (COLS * LINES)								   //Anzahl der boxen (pro page)
#define FRAME_WIDTH (DISP_WIDTH / COLS)						   //groesse der boxen (in der breite)
#define FRAME_HIGHT (DISP_HIGHT / LINES)					   //groesse der boxen (in der hoehe)
#define VAR_TEXT_LENGTH_MAX (FRAME_WIDTH - 2 * TEXT_OFFSET_W)  // maximale Anzahl an Pixel für eine Variable (in der breite)
#define VAR_TEXT_POSITION_H (FRAME_HIGHT / 2 + VAR_TEXT_HIGHT) // hoehe wo die Variable hingeschrieben wird
// text_check() MODES
#define MODE_VARIABLE 0x33
#define MODE_TITLE 0x32

typedef struct showText
{
	char title[50];
	char variable[50];
	int position;
	struct showText *next;
} showText_t;

void drawIt(ucg_t *ucg, uint8_t inputpage, char mode);
void drawFrames(ucg_t *ucg, uint8_t inputpage);
void addParam(showText_t *space, char title[], char variable[]);
void frame(ucg_t *ucg, int line, int colum);
void title(ucg_t *ucg, char text[], int line, int colum);
void variable(ucg_t *ucg, char text[], int line, int colum);
void clearVar(ucg_t *ucg, int line, int colum);
void check_text(ucg_t *ucg, char *quelle, char *ziel, char mode);
void changePage(ucg_t *ucg, uint8_t page);
void init_disp(ucg_t *ucg);
void init_pins(ucg_t *ucg);
void pin_up_handler(void *arg);
void pin_down_handler(void *arg);

static showText_t head;
volatile uint8_t nr = 0;	   // letzter Eintrag in die Struktur
volatile uint8_t max_page = 0; // maximale Seitenanzahl
volatile uint8_t curr_page = 0;
static gpio_t pins[] = {
	[UCG_PIN_CS] = GPIO5,  // Chip select
	[UCG_PIN_CD] = GPIO14, // Command/Data or A0
	[UCG_PIN_RST] = GPIO2  // Reset pin
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
static uint32_t pins_enabled = ((1 << UCG_PIN_CS) +
								(1 << UCG_PIN_CD) +
								(1 << UCG_PIN_RST));

int main(void)
{
	static showText_t showDown[28];
	ucg_t ucg;
	uint8_t localPage = curr_page;

	init_disp(&ucg);
	init_pins(&ucg);

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
	puts("Starting Pages");

	changePage(&ucg, localPage);
	while (1)
	{
		if (localPage != curr_page)
		{
			localPage = curr_page;
			changePage(&ucg, localPage);
		}
	}
	return 0;
}

void init_pins(ucg_t *ucg)
{
	gpio_init_int(GPIO22, GPIO_IN_PD, GPIO_RISING, pin_up_handler, ucg);
	gpio_init_int(GPIO21, GPIO_IN_PD, GPIO_RISING, pin_down_handler, ucg);
}
void pin_up_handler(void *arg)
{
	puts("Interrupt Up");
	if (curr_page == max_page)
		return;
	curr_page += 1;
	printf("%d \n", curr_page);
}
void pin_down_handler(void *arg)
{
	puts("Interrupt Down");
	if (curr_page == 0)
		return;
	curr_page -= 1;
	printf("%d \n", curr_page);
}
// funktion um die Struktur richtig zu fuellen.
void addParam(showText_t *space, char title[], char variable[])
{
	showText_t *current = &head;

	while (current->next != NULL) // iteriert durch die linked liste zum letzten eintrag
	{
		current = current->next;
	}
	if (current == space) // stopen von schleifen
		return;

	current->next = space; // fuegt neue structur zum ende der liste

	// ** Leerzeichen muessen abgefangen werden
	strcpy(space->title, title);
	strcpy(space->variable, variable);

	if (nr >= QBOXES) // Parameter passt nicht auf Seite null
	{
		max_page = (uint8_t)(nr / QBOXES);
	}
	else // Parameter passt auf Seite null
	{
		max_page = 0;
	}
	nr++;
}

void drawFrames(ucg_t *ucg, uint8_t inputpage)
{
	uint8_t l = 0;
	uint8_t c = 0;

	if (inputpage > max_page)
		return;
	if (nr == 0)
		return;
	if (inputpage < max_page) // full Page
	{
		for (l = 0; l < LINES; l++)
		{
			for (c = 0; c < COLS; c++)
			{
				frame(ucg, l, c);
			}
		}
	}
	else if (inputpage == max_page) // teils full
	{
		uint8_t position = (uint8_t)((nr - 1) % QBOXES); // nr == actual position
		for (int p = 0; p <= position; p++)
		{
			l = (uint8_t)(p / COLS);
			c = (uint8_t)(p % COLS);
			frame(ucg, l, c);
		}
	}
}

void drawIt(ucg_t *ucg, uint8_t inputpage, char mode)
{
	showText_t *current = head.next; // start hier
	uint8_t l = 0;
	uint8_t c = 0;
	int startlist = 0;
	int i = 0;

	if (nr == 0)
		return;
	if (inputpage > max_page)
		return;
	startlist = (inputpage * QBOXES); // start der liste
	for (i = 0; i < startlist; i++)
	{
		current = current->next; // iterating over the list
	}
	if (inputpage < max_page) // full Page
	{
		puts("Full Page");
		for (l = 0; l < LINES; l++)
		{
			for (c = 0; c < COLS; c++)
			{
				switch (mode)
				{
				case MODE_TITLE:
					title(ucg, current->title, l, c);
					break;
				case MODE_VARIABLE:
					variable(ucg, current->variable, l, c);
					break;
				default:
					return;
				}
				current = current->next; // iterating over the list
			}
		}
	}
	else if (inputpage == max_page) // teil Page
	{
		puts("Teils Page");
		uint8_t position = (uint8_t)((nr - 1) % QBOXES); // nr == max position
		for (int p = 0; p <= position; p++)
		{
			l = (uint8_t)(p / COLS);
			c = (uint8_t)(p % COLS);

			switch (mode)
			{
			case MODE_TITLE:
				title(ucg, current->title, l, c);
				break;
			case MODE_VARIABLE:
				variable(ucg, current->variable, l, c);
				break;
			default:
				return;
			}
			current = current->next; // iterating over the list
		}
	}
}

void changePage(ucg_t *ucg, uint8_t page)
{
	if (page <= max_page)
	{
		ucg_ClearScreen(ucg);
		drawFrames(ucg, page);
		drawIt(ucg, page, MODE_TITLE);
		drawIt(ucg, page, MODE_VARIABLE);
	}
}

void init_disp(ucg_t *ucg)
{
	ucg_SetPins(ucg, pins, pins_enabled);
	ucg_SetDevice(ucg, SPI_DEV(0));

	ucg_Init(ucg, ucg_dev_st7735_18x128x160, ucg_ext_st7735_18, ucg_com_riotos_hw_spi);

	/* initialize the display */
	puts("Initializing display.");
	ucg_SetFontMode(ucg, UCG_FONT_MODE_TRANSPARENT);

	ucg_ClearScreen(ucg);
	ucg_SetFontPosBaseline(ucg);
}

void frame(ucg_t *ucg, int line, int colum)
{
	if (line > LINES || colum > COLS)
		return;
	ucg_SetColor(ucg, 0, 189, 32, 43);
	// draw frame
	ucg_DrawRFrame(ucg, colum * FRAME_WIDTH, line * FRAME_HIGHT, FRAME_WIDTH, FRAME_HIGHT, FRAME_EGDE_RADIUS);
}
// Text Position example
/*      --------------------
			*  	- 0.0	-	0.1-
			*	--------------------
			*  	- 1.0	-	1.1-
			*	--------------------
			*/

void title(ucg_t *ucg, char text[], int line, int colum)
{
	if (line > LINES || colum > COLS)
		return;
	ucg_SetColor(ucg, 0, 189, 32, 43);
	// change font
	ucg_SetFont(ucg, TITLE_FONT);
	// test text
	char checked_text[] = " ";
	check_text(ucg, text, checked_text, MODE_TITLE);
	// draw text
	ucg_DrawString(ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + (TITLE_TEXT_HIGHT + TEXT_OFFSET_H), 0, checked_text);
}

// Variable Position example
/*      --------------------
			*  	- 0.0	-	0.1-
			*	--------------------
			*  	- 1.0	-	1.1-
			*	--------------------
			*/

void variable(ucg_t *ucg, char text[], int line, int colum)
{
	if (line > LINES || colum > COLS)
		return;
	//clear area
	clearVar(ucg, line, colum);
	// change Color
	ucg_SetColor(ucg, 0, 63, 167, 136);
	// change font
	ucg_SetFont(ucg, VAR_FONT);
	// test text
	char checked_text[] = " ";
	check_text(ucg, text, checked_text, MODE_VARIABLE);
	ucg_DrawString(ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + VAR_TEXT_POSITION_H, 0, checked_text);
}

void clearVar(ucg_t *ucg, int line, int colum)
{
	if (line > LINES || colum > COLS)
		return;
	ucg_SetColor(ucg, 0, 0, 0, 0);
	ucg_DrawBox(ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + (VAR_TEXT_POSITION_H - VAR_TEXT_HIGHT), VAR_TEXT_LENGTH_MAX, (VAR_TEXT_HIGHT + VAR_TEXT_PLUS));
}

void check_text(ucg_t *ucg, char *quelle, char *ziel, char mode)
{
	char out[16];					 // ausgabe string
	int str_length = strlen(quelle); // string laenge
	char ending[3];					 // was ans ende angehängt wird
	uint8_t less = 0;				 // menge an zu löschenden Zeichen

	strcpy(out, quelle);
	switch (mode)
	{
	case MODE_VARIABLE:
		strcpy(ending, ".");
		less = 1;
		break;
	case MODE_TITLE:
		strcpy(ending, " .:");
		less = 2;
		break;
	default:
		strcpy(ending, "");
		break;
	}
	while (ucg_GetStrWidth(ucg, out) > FRAME_WIDTH - TEXT_OFFSET_W)
	{
		if (str_length > 1)
		{
			str_length -= less;
			out[str_length] = '\0';
			strcat(out, ending);
		}
		else
			break;
	}
	strcpy(ziel, out);
}
