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

#define VAR_FONT ucg_font_7x14B_tf
#define TITLE_FONT ucg_font_timB08_hf
#define TEXT_UL "Temperatur:"	//text left up
#define TEXT_UR	"Humitity:"	//text right up
#define TEXT_DL "Time:"		//text left down
#define TEXT_DR "State:"	//text right down
#define DISP_WIDTH 128		//display width
#define DISP_HIGHT 128		//display hight
#define VAR_TEXT_HIGHT 10	//groesse der Variablenschrift
#define VAR_TEXT_PLUS 4		//unterer Bereich von "gyqj"
#define TITLE_TEXT_HIGHT 8	//groesse der Titelschrift
#define FRAME_EGDE_RADIUS 3	//Radius for the four edges.
#define TEXT_OFFSET_W 3		//die Rahmen um die boxen erzeugen einen offset wo keine Schrift rein kann. (in der breite)
#define TEXT_OFFSET_H 3		//die Rahmen um die boxen erzeugen einen offset wo keine Schrift rein kann. (in der hoehe)
#define COLS 2			//Anzahl der boxen (in der breite)
#define LINES 2			//Anzahl der boxen (in der hoehe)
#define QBOXES (COLS * LINES) 	//Anzahl der boxen (pro page)
#define FRAME_WIDTH (DISP_WIDTH / COLS)	//groesse der boxen (in der breite)
#define FRAME_HIGHT (DISP_HIGHT / LINES)//groesse der boxen (in der hoehe)
#define VAR_TEXT_LENGTH_MAX (FRAME_WIDTH - 2 *TEXT_OFFSET_W) // maximale Anzahl an Pixel für eine Variable (in der breite)   
#define VAR_TEXT_POSITION_H (FRAME_HIGHT/2 + VAR_TEXT_HIGHT) // hoehe wo die Variable hingeschrieben wird

// text_check() MODES
#define MODE_VARIABLE 0x33
#define MODE_TITLE    0x32

void drawTitles(ucg_t* ucg, uint8_t inputpage);
void drawVariables(ucg_t* ucg, uint8_t inputpage);
void drawFrames(ucg_t* ucg, uint8_t inputpage);
void addParam(char title[], char variable[]);
void frame(ucg_t* ucg, int line, int colum);
void title(ucg_t* ucg, char text[], int line, int colum);
void variable(ucg_t* ucg, char text[], int line, int colum);
void clearVar(ucg_t* ucg, int line, int colum);
void check_text(ucg_t* ucg, char *quelle, char *ziel, char mode);
void changePage(uint8_t page);

typedef struct {
char title[50];
char variable[50];
int line;
int colum;
int page;
}showText;

static showText showTex[100];
static uint8_t nr = 0;		// letzter Eintrag in die Struktur 
static uint8_t max_page = 0; 	// maximale Seitenanzahl
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
    
    addParam(TEXT_UL, "1234567889");
    addParam(TEXT_UR, "2");
    addParam(TEXT_DL, "3");
    addParam(TEXT_DR, "4");
    addParam("nextSite", "5");
    while (1) {
       
        switch (screen) {
            case 0:
		//ucg_ClearScreen(&ucg);
		// Start up Display
		drawFrames(&ucg, 0);
		drawTitles(&ucg, 0);
		drawVariables(&ucg, 0);

        	screen += 1;
	 	break;
            case 1:
		ucg_ClearScreen(&ucg);
		if(max_page >= 1)
		{
			drawFrames(&ucg, 1);
			//drawTitles(&ucg, 1);
			//drawVariables(&ucg, 1);
		}
		screen += 1;		
                break;
	    case 2:
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
void addParam(char title[], char variable[])
{	
	uint8_t page = 0;				// Anzahl der Seiten
	uint8_t position = 0;				// Position auf der Seite
	
	if(nr >= QBOXES)
	{ 
		page = (uint8_t) (nr / QBOXES);		
		position = (uint8_t) (nr % QBOXES); 	
	}else
	{	page = 0;
		position = nr;
	}
	// ** Leerzeichen abfangen 
	strcpy(showTex[nr].title, title);
	strcpy(showTex[nr].variable, variable);
	if(position == 0)
	{
		showTex[nr].line = 0;
		showTex[nr].colum = 0; 
	}else
	{	
		showTex[nr].line = (uint8_t) (position / COLS);
		showTex[nr].colum = (uint8_t) ((position % COLS)-1); 
	}
	showTex[nr].page = page;
	max_page = page;
	printf("%d\n", nr);
	nr ++;
}
//** 
void drawFrames(ucg_t* ucg, uint8_t inputpage)
{	
	uint8_t line = 0;
	uint8_t cols = 0;
	
	if(inputpage < max_page)				// full Page
	{
		line = LINES;
		cols = COLS;
	}else if(inputpage == max_page)				// teils full
	{
		uint8_t position = (uint8_t) ((nr + 1) % QBOXES); //<---*
		line = (uint8_t) (position / COLS);
		cols = (uint8_t) ((position % COLS)-1); 
	}
	for(int l=0; l < line; l++)
	{
		for(int c=0; c < cols; c++)
		{
			frame(ucg, l, c);
		}
	}
}

void drawTitles(ucg_t* ucg, uint8_t inputpage)
{
	for(int l=0; l < LINES; l++)
	{
		for(int c=0; c < COLS; c++)
		{
			title(ucg,showTex[(l*COLS)+c].title, l, c);
		}
	}
}
void drawVariables(ucg_t* ucg, uint8_t inputpage)
{
	for(int l=0; l < LINES; l++)
	{
		for(int c=0; c < COLS; c++)
		{
			variable(ucg,showTex[(l*COLS)+c].variable, l, c);
		}
	}	
}
void changePage(uint8_t page)
{

}

//1. Draw Frame 
void frame(ucg_t* ucg, int line, int colum)
{	
	ucg_SetColor(ucg, 0, 189, 32, 43);
	// draw frame
	ucg_DrawRFrame(ucg, colum * FRAME_WIDTH,  line * FRAME_HIGHT, FRAME_WIDTH, FRAME_HIGHT, FRAME_EGDE_RADIUS);
}
			// Text Position example
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
//3. Variable Text
void variable(ucg_t* ucg, char text[], int line, int colum)
{	
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

void clearVar(ucg_t* ucg, int line, int colum)
{
	ucg_SetColor(ucg, 0, 0, 0, 0);
	ucg_DrawBox(ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + (VAR_TEXT_POSITION_H - VAR_TEXT_HIGHT), VAR_TEXT_LENGTH_MAX, (VAR_TEXT_HIGHT + VAR_TEXT_PLUS));
}

void check_text(ucg_t* ucg, char *quelle, char *ziel, char mode)
{	
	char out[16];				// ausgabe string
	int str_length = strlen(quelle);	// string laenge
	char ending[3];				// was ans ende angehängt wird
	uint8_t less = 0;			// menge an zu löschenden Zeichen

	strcpy(out, quelle);
	switch(mode){
		case MODE_VARIABLE:
			strcpy(ending, ".");
			less = 1; 
			break;
		case MODE_TITLE:
			strcpy(ending, " .:"); 
			less = 2;
			break;
		default:strcpy(ending, "");  break;
	}
	while (ucg_GetStrWidth(ucg, out) > FRAME_WIDTH - TEXT_OFFSET_W)
	{	
		if(str_length > 1)
		{	
			str_length -= less;
			out[str_length] = '\0';
			strcat(out, ending);
		}else break;
	}
 	strcpy(ziel, out);		
}