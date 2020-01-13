#include "disp.h"

uint8_t curr_page = 0;  // Aktuelle Seite die angezeigt wird
static showText_t head; // Start der linked liste
uint8_t nr = 0;         // letzter Eintrag in die Struktur maximale Position
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
#define SPEICHER 255 // anzahl speicher
#define SEPERATOR ";"
ucg_t ucg;

static showText_t showDown[SPEICHER]; // Fester Speicher
static msg_t disp_queue[DISP_QUEUE_SIZE];
void *disp_thread(void *arg)
{
    msg_t msg;
    kernel_pid_t *disp_pid = (kernel_pid_t *)arg;

    disp_init();
    disp_init_buttons(disp_pid);

    uint8_t i;
    for (i = 0; i < SPEICHER; i++)
    { // freier Speicherplatz wird mit Position 255 definiert
        showDown[i].position = SPEICHER;
    }

    puts("Starting Pages");
    disp_changePage(0);
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
        disp_changePage(curr_page);
    }
    return NULL;
}

ssize_t _dev_disp_parameter_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));
    char *title;
    char *variable;
    char temp_pay[50] = "";
    char delimiter[2] = ";";
    uint8_t temp_pos = SPEICHER;

    switch (method_flag)
    {
    case COAP_GET:
        gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
        coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
        size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

        strcpy(temp_pay, (char *)pdu->payload);
        temp_pos = getPosition(temp_pay);
        if (temp_pos != SPEICHER)
        {
            showText_t *resp = getshowText(temp_pos);
            if (pdu->payload_len >= strlen(resp->variable))
            {
                memcpy(pdu->payload, resp->variable, strlen(resp->variable));
                resp_len += strlen(resp->variable);
            }
        }
        else
        {
            char error_resp[22] = "Title not registered.";
            if (pdu->payload_len >= strlen(error_resp))
            {
                memcpy(pdu->payload, error_resp, strlen(error_resp));
                resp_len += strlen(error_resp);
            }
        }
        return resp_len;
    case COAP_PUT: //aktualisieren
                   /* update the internal variable based on the title*/
        strcpy(temp_pay, (char *)pdu->payload);
        title = strtok(temp_pay, delimiter);
        if (title == NULL)
            return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
        variable = strtok(NULL, delimiter);
        if (variable == NULL)
            return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
        temp_pos = getPosition(title);
        if (temp_pos != SPEICHER)
        {
            disp_changeVar(getshowText(temp_pos), variable);
            return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
        }
        return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
    case COAP_POST: //erzeugen
        strcpy(temp_pay, (char *)pdu->payload);
        title = strtok(temp_pay, delimiter);
        if (title == NULL)
            return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
        variable = strtok(NULL, delimiter);
        if (variable == NULL)
            return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
        showText_t *space = getnewspace();
        if (space != NULL)
        {
            disp_addParam(space, title, variable);
            return gcoap_response(pdu, buf, len, COAP_CODE_VALID);
        }
        return gcoap_response(pdu, buf, len, COAP_CODE_NOT_ACCEPTABLE);
    }
    return 0;
}
// returns position from title
uint8_t getPosition(char *title)
{
    showText_t *current = head.next;
    while (strcmp((char *)current->title, title) != 0)
    {
        if (current->next == NULL)
        {
            puts("Error: getPosition() -> wrong title");
            return SPEICHER;
        }
        current = current->next;
    }
    return current->position;
}
// gibt speicher zurueck der noch frei ist
showText_t *getnewspace(void)
{
    showText_t *current = &showDown[0];
    int i = 0;
    while (current->position != SPEICHER) // wenn position == 255 ist, dann unbenutzt
    {
        i++;
        if (i == SPEICHER)
        {
            puts("Error: getnewspace() -> No Space left");
            return NULL;
        }
        current = &showDown[i];
    }
    return current;
}
// gibt den speicher zurueck der zu der aktuellen position gehoert
showText_t *getshowText(uint8_t posi)
{
    showText_t *current = head.next;
    int i = 0;
    while (i < posi)
    {
        i++;
        if (current->next == NULL)
        {
            puts("Error: getshowText() -> wrong position");
            return NULL;
        }
        current = current->next;
    }
    return current;
}

// Parameter aus der liste Loeschen
void disp_deleteParam(showText_t *space)
{
    showText_t *current = &head;

    while (current->next != space) // iteriert durch die linked liste zum passenden eintrag
    {
        current = current->next;
        if (current->next == NULL) // wenn "space" nicht vorhanden -> fehlermeldung
        {
            puts("Error: disp_deleteParam() -> Parameter does not exist.");
            return;
        }
    }
    if (space->next != NULL) // ende der liste ignorieren
    {
        current->next = space->next;  // Parameter aus der liste loeschen
        space->next = NULL;           // loeschen der verbindung zur liste
        while (current->next != NULL) // anpassen der positionen der restlichen parameter
        {
            current = current->next; // iteriert durch die liste
            current->position -= 1;  // setzt die position um einen runter
        }
    }
    else
    {
        current->next = NULL; // Parameter aus der liste loeschen
    }
    space->position = SPEICHER;     // position == 255, bedeutet freier Speicher
    nr--;                           // loescht ein element aus der globalen Variable
    max_page = disp_get_myPage(nr); // aendert die maximale Seitenzahl
}

// Variablen-Inhalt aendern
void disp_changeVar(showText_t *space, char *new_var)
{
    strcpy(space->variable, new_var);
    //refreshing the current page after updating variables
    if (disp_get_currPage() == disp_get_myPage(space->position))
    {
        disp_refresh();
    }
}
// Aktualisieren der Seite
void disp_refresh(void)
{
    disp_drawIt(curr_page, MODE_VARIABLE);
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
void disp_changePage(uint8_t page)
{
    if (page <= max_page)
    {
        ucg_ClearScreen(&ucg);
        disp_drawFrames(page);
        disp_drawIt(page, MODE_TITLE);
        disp_drawIt(page, MODE_VARIABLE);
    }
}
// initialisiert das Display
void disp_init(void)
{
    ucg_SetPins(&ucg, pins, pins_enabled);
    ucg_SetDevice(&ucg, SPI_DEV(0));

    ucg_Init(&ucg, ucg_dev_st7735_18x128x160, ucg_ext_st7735_18, ucg_com_riotos_hw_spi);

    /* initialize the display */
    puts("Initializing display.");
    ucg_SetFontMode(&ucg, UCG_FONT_MODE_TRANSPARENT);

    ucg_ClearScreen(&ucg);
    ucg_SetFontPosBaseline(&ucg);
}
// initialisiert die Knoepfe
void disp_init_buttons(kernel_pid_t *disp_pid)
{
    gpio_init_int(GPIO22, GPIO_IN_PD, GPIO_RISING, disp_pin_up_handler, disp_pid);
    gpio_init_int(GPIO21, GPIO_IN_PD, GPIO_RISING, disp_pin_down_handler, disp_pid);
}

void disp_drawFrames(uint8_t inputpage)
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
                disp_frame(l, c);
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
            disp_frame(l, c);
        }
    }
}

void disp_drawIt(uint8_t inputpage, char mode)
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
                    disp_title(current->title, l, c);
                    break;
                case MODE_VARIABLE:
                    disp_variable(current->variable, l, c);
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
                disp_title(current->title, l, c);
                break;
            case MODE_VARIABLE:
                disp_variable(current->variable, l, c);
                break;
            default:
                return;
            }
            current = current->next; // iterating over the list
        }
    }
}

void disp_frame(int line, int colum)
{
    if (line > LINES || colum > COLS)
        return;
    ucg_SetColor(&ucg, 0, 189, 32, 43);
    // draw frame
    ucg_DrawRFrame(&ucg, colum * FRAME_WIDTH, line * FRAME_HIGHT, FRAME_WIDTH, FRAME_HIGHT, FRAME_EGDE_RADIUS);
}
// Title/Variable Position example
/*      --------------------
			*  	- 0.0	-	0.1-
			*	--------------------
			*  	- 1.0	-	1.1-
			*	--------------------
			*/

void disp_title(char text[], int line, int colum)
{
    if (line > LINES || colum > COLS)
        return;
    ucg_SetColor(&ucg, 0, 189, 32, 43);
    // change font
    ucg_SetFont(&ucg, TITLE_FONT);
    // test text
    char checked_text[] = " ";
    disp_check_text(text, checked_text, MODE_TITLE);
    // draw text
    ucg_DrawString(&ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + (TITLE_TEXT_HIGHT + TEXT_OFFSET_H), 0, checked_text);
}

void disp_variable(char text[], int line, int colum)
{
    if (line > LINES || colum > COLS)
        return;
    //clear area
    disp_clearVar(line, colum);
    // change Color
    ucg_SetColor(&ucg, 0, 63, 167, 136);
    // change font
    ucg_SetFont(&ucg, VAR_FONT);
    // test text
    char checked_text[] = " ";
    disp_check_text(text, checked_text, MODE_VARIABLE);
    ucg_DrawString(&ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + VAR_TEXT_POSITION_H, 0, checked_text);
}

void disp_clearVar(int line, int colum)
{
    if (line > LINES || colum > COLS)
        return;
    ucg_SetColor(&ucg, 0, 0, 0, 0);
    ucg_DrawBox(&ucg, colum * FRAME_WIDTH + TEXT_OFFSET_W, line * FRAME_HIGHT + (VAR_TEXT_POSITION_H - VAR_TEXT_HIGHT), VAR_TEXT_LENGTH_MAX, (VAR_TEXT_HIGHT + VAR_TEXT_PLUS));
}

void disp_check_text(char *quelle, char *ziel, char mode)
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
    while (ucg_GetStrWidth(&ucg, out) > FRAME_WIDTH - TEXT_OFFSET_W)
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
