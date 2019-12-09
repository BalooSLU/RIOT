#include "disp.h"

volatile uint8_t curr_page = 0;
static showText_t head;
volatile uint8_t nr = 0;       // letzter Eintrag in die Struktur
volatile uint8_t max_page = 0; // maximale Seitenanzahl
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
uint8_t get_currPage(void)
{
    return curr_page;
}

void init_buttons(ucg_t *ucg)
{
    gpio_init_int(GPIO22, GPIO_IN_PD, GPIO_RISING, pin_up_handler, ucg);
    gpio_init_int(GPIO21, GPIO_IN_PD, GPIO_RISING, pin_down_handler, ucg);
}
void pin_up_handler(void *arg)
{
    if (curr_page == max_page)
        return;
    curr_page += 1;
}
void pin_down_handler(void *arg)
{
    if (curr_page == 0)
        return;
    curr_page -= 1;
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
    char out[16];                    // ausgabe string
    int str_length = strlen(quelle); // string laenge
    char ending[3];                  // was ans ende angehängt wird
    uint8_t less = 0;                // menge an zu löschenden Zeichen

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
