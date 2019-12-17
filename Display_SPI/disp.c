#include "disp.h"

uint8_t curr_page = 0;  // Aktuelle Seite die angezeigt wird
static showText_t head; // Start der linked liste
uint8_t nr = 0;         // letzter Eintrag in die Struktur
uint8_t max_page = 0;   // maximale Seitenanzahl
static gpio_t pins[] = {
    [UCG_PIN_CS] = GPIO5,  // Chip select
    [UCG_PIN_CD] = GPIO14, // Command/Data or A0
    [UCG_PIN_RST] = GPIO2  // Reset pin
};
// Pin out for ESP-WROOM-32
/*          	Diplay 	-	MCU
			*-----------------------------
			*	LED	-	3V3
			*  	SCK	-	D18
			*  	SDA	-	D23
			*  	A0	-	D14
			*  	RESET	-	D2
			*  	CS	-	D5
			*  	GND	-	GND
			*  	VCC	-	VIN
            * Up_Botton - D22
            * Down_Bott - D21
			*/
static uint32_t pins_enabled = ((1 << UCG_PIN_CS) +
                                (1 << UCG_PIN_CD) +
                                (1 << UCG_PIN_RST));

#define DISP_QUEUE_SIZE (8)
// Text Beispiele:
#define TEXT_UL "Temperatur:" //text left up
#define TEXT_UR "Humitity:"   //text right up
#define TEXT_DL "Time:"       //text left down
#define TEXT_DR "State:"      //text right down

static showText_t showDown[28];
static msg_t disp_queue[DISP_QUEUE_SIZE];
void *disp_thread(void *arg)
{
    ucg_t ucg;
    msg_t msg;
    kernel_pid_t *disp_pid = (kernel_pid_t *)arg;

    disp_init(&ucg);
    disp_init_buttons(&ucg, disp_pid);

    disp_addParam(&showDown[0], TEXT_UL, "1234567889");
    disp_addParam(&showDown[1], TEXT_UR, "2");
    disp_addParam(&showDown[2], TEXT_DL, "3");
    disp_addParam(&showDown[3], TEXT_DR, "4");
    disp_addParam(&showDown[4], "NextSite", "5");
    disp_addParam(&showDown[5], "FullSide", "9");
    disp_addParam(&showDown[6], "FullSide", "2");
    disp_addParam(&showDown[7], "qw", "3");
    disp_addParam(&showDown[8], "FullSide", "123");
    disp_addParam(&showDown[9], "q", "4467");
    disp_addParam(&showDown[10], "FullSide", "33222");
    disp_addParam(&showDown[11], "re", "12356");
    disp_addParam(&showDown[12], "FulldfSide", "433445");
    disp_addParam(&showDown[13], "vx", "2234567");
    disp_addParam(&showDown[14], "kglr", "89777");
    disp_addParam(&showDown[15], "zziioosa", "5544");
    disp_addParam(&showDown[16], "ffddss", "3456");
    disp_addParam(&showDown[17], "sdccvbbb", "78866");
    disp_addParam(&showDown[18], "sdfkfgrr", "33558");
    disp_addParam(&showDown[19], "dfgksdkfr", "2221");
    disp_addParam(&showDown[20], "daeglfkg", "2");
    disp_addParam(&showDown[21], "ddc", "345");
    disp_addParam(&showDown[22], "fgf", "678");
    disp_addParam(&showDown[23], "izih", "44664");
    disp_addParam(&showDown[24], "isaih", "4123664");
    disp_addParam(&showDown[25], "iasfzih", "44123664");

    puts("Starting Pages");

    disp_changePage(&ucg, 0);
    msg_init_queue(disp_queue, DISP_QUEUE_SIZE);
    while (1)
    {
        msg_receive(&msg);
        if (msg.content.value == 0)
        {
            curr_page -= 1;
        }
        else if (msg.content.value == 1)
        {
            curr_page += 1;
        }
        disp_changePage(&ucg, curr_page);
    }
    return NULL;
}

// Variablen-Inhalt aendern
void disp_changeVar(ucg_t *ucg, showText_t *space, char *new_var)
{
    strcpy(space->variable, new_var);
    //refreshing the current page after updating variables
    if (disp_get_currPage() == disp_get_myPage(space->position))
    {
        disp_refresh(ucg);
    }
}
// Aktualisieren der Seite
void disp_refresh(ucg_t *ucg)
{
    disp_drawIt(ucg, curr_page, MODE_VARIABLE);
}
// Gibt die aktuelle Seite zurueck
uint8_t disp_get_currPage(void)
{
    return curr_page;
}
// Gibt die Seite die zu der Position gehoert zurueck
uint8_t disp_get_myPage(uint8_t position)
{
    if (position >= QBOXES) // Parameter passt nicht auf Seite null
    {
        return (uint8_t)(position / QBOXES);
    }
    else // Parameter passt auf Seite null
    {
        return 0;
    }
}
// Eine Seite vor Handler
void disp_pin_up_handler(void *arg)
{
    kernel_pid_t *pid = (kernel_pid_t *)arg;
    msg_t msg;

    if (curr_page == max_page)
        return;
    msg.content.value = 1;
    if (msg_try_send(&msg, *pid) == 0)
    {
        printf("Receiver queue full.\n");
    }
}
// Eine Seite zurueck Handler
void disp_pin_down_handler(void *arg)
{
    kernel_pid_t *pid = (kernel_pid_t *)arg;
    msg_t msg;

    if (curr_page == 0)
        return;
    msg.content.value = 0;
    if (msg_try_send(&msg, *pid) == 0)
    {
        printf("Receiver queue full.\n");
    }
}

// funktion um die Struktur richtig zu fuellen.
void disp_addParam(showText_t *space, char title[], char variable[])
{
    showText_t *current = &head;

    while (current->next != NULL) // iteriert durch die linked liste zum letzten eintrag
    {
        current = current->next;
    }
    if (current == space) // stopen von schleifen
        return;

    current->next = space; // fuegt neue structur zum ende der liste

    strcpy(space->title, title);
    strcpy(space->variable, variable);
    space->position = nr;

    max_page = disp_get_myPage(nr);
    nr++;
}
// funktion um die Seite zu wechseln
void disp_changePage(ucg_t *ucg, uint8_t page)
{
    if (page <= max_page)
    {
        ucg_ClearScreen(ucg);
        disp_drawFrames(ucg, page);
        disp_drawIt(ucg, page, MODE_TITLE);
        disp_drawIt(ucg, page, MODE_VARIABLE);
    }
}
// initialisiert das Display
void disp_init(ucg_t *ucg)
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
// initialisiert die Knoepfe
void disp_init_buttons(ucg_t *ucg, kernel_pid_t *disp_pid)
{
    gpio_init_int(GPIO22, GPIO_IN_PD, GPIO_RISING, disp_pin_up_handler, disp_pid);
    gpio_init_int(GPIO21, GPIO_IN_PD, GPIO_RISING, disp_pin_down_handler, disp_pid);
}

void disp_drawFrames(ucg_t *ucg, uint8_t inputpage)
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
                disp_frame(ucg, l, c);
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
            disp_frame(ucg, l, c);
        }
    }
}

void disp_drawIt(ucg_t *ucg, uint8_t inputpage, char mode)
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
        for (l = 0; l < LINES; l++)
        {
            for (c = 0; c < COLS; c++)
            {
                switch (mode)
                {
                case MODE_TITLE:
                    disp_title(ucg, current->title, l, c);
                    break;
                case MODE_VARIABLE:
                    disp_variable(ucg, current->variable, l, c);
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
        uint8_t position = (uint8_t)((nr - 1) % QBOXES); // nr == max position
        for (int p = 0; p <= position; p++)
        {
            l = (uint8_t)(p / COLS);
            c = (uint8_t)(p % COLS);

            switch (mode)
            {
            case MODE_TITLE:
                disp_title(ucg, current->title, l, c);
                break;
            case MODE_VARIABLE:
                disp_variable(ucg, current->variable, l, c);
                break;
            default:
                return;
            }
            current = current->next; // iterating over the list
        }
    }
}

void disp_frame(ucg_t *ucg, int line, int colum)
{
    if (line > LINES || colum > COLS)
        return;
    ucg_SetColor(ucg, 0, 189, 32, 43);
    // draw frame
    ucg_DrawRFrame(ucg, colum * FRAME_WIDTH, line * FRAME_HIGHT, FRAME_WIDTH, FRAME_HIGHT, FRAME_EGDE_RADIUS);
}
// Title/Variable Position example
/*      --------------------
			*  	- 0.0	-	0.1-
			*	--------------------
			*  	- 1.0	-	1.1-
			*	--------------------
			*/

void disp_title(ucg_t *ucg, char text[], int line, int colum)
{
    if (line > LINES || colum > COLS)
        return;
    ucg_SetColor(ucg, 0, 189, 32, 43);
    // change font
    ucg_SetFont(ucg, TITLE_FONT);
    // test text
    char checked_text[] = " ";
    disp_check_text(ucg, text, checked_text, MODE_TITLE);
    // draw text
    ucg_DrawString(ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + (TITLE_TEXT_HIGHT + TEXT_OFFSET_H), 0, checked_text);
}

void disp_variable(ucg_t *ucg, char text[], int line, int colum)
{
    if (line > LINES || colum > COLS)
        return;
    //clear area
    disp_clearVar(ucg, line, colum);
    // change Color
    ucg_SetColor(ucg, 0, 63, 167, 136);
    // change font
    ucg_SetFont(ucg, VAR_FONT);
    // test text
    char checked_text[] = " ";
    disp_check_text(ucg, text, checked_text, MODE_VARIABLE);
    ucg_DrawString(ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + VAR_TEXT_POSITION_H, 0, checked_text);
}

void disp_clearVar(ucg_t *ucg, int line, int colum)
{
    if (line > LINES || colum > COLS)
        return;
    ucg_SetColor(ucg, 0, 0, 0, 0);
    ucg_DrawBox(ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + (VAR_TEXT_POSITION_H - VAR_TEXT_HIGHT), VAR_TEXT_LENGTH_MAX, (VAR_TEXT_HIGHT + VAR_TEXT_PLUS));
}

void disp_check_text(ucg_t *ucg, char *quelle, char *ziel, char mode)
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
