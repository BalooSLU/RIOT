#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <string.h>
#include "periph/gpio.h"
#include "periph/spi.h"
#include "ucg.h"
#include "thread.h"

#define VAR_FONT ucg_font_7x14B_tf
#define TITLE_FONT ucg_font_timB08_hf

// Display Eigenschaften
#define DISP_WIDTH 128 //display width
#define DISP_HIGHT 128 //display hight
// Text Eigenschaften
#define VAR_TEXT_HIGHT 10  //groesse der Variablenschrift
#define VAR_TEXT_PLUS 4    //unterer Bereich von "gyqj"
#define TITLE_TEXT_HIGHT 8 //groesse der Titelschrift
#define TEXT_OFFSET_W 3    //die Rahmen um die boxen erzeugen einen offset wo keine Schrift rein kann. (in der breite)
#define TEXT_OFFSET_H 3    //die Rahmen um die boxen erzeugen einen offset wo keine Schrift rein kann. (in der hoehe)
// Box Eigenschaften
#define FRAME_EGDE_RADIUS 3                                    //Radius for the four edges.
#define COLS 2                                                 //Anzahl der boxen (in der breite)
#define LINES 2                                                //Anzahl der boxen (in der hoehe)
#define QBOXES (COLS * LINES)                                  //Anzahl der boxen (pro page)
#define FRAME_WIDTH (DISP_WIDTH / COLS)                        //groesse der boxen (in der breite)
#define FRAME_HIGHT (DISP_HIGHT / LINES)                       //groesse der boxen (in der hoehe)
#define VAR_TEXT_LENGTH_MAX (FRAME_WIDTH - 2 * TEXT_OFFSET_W)  // maximale Anzahl an Pixel für eine Variable (in der breite)
#define VAR_TEXT_POSITION_H (FRAME_HIGHT / 2 + VAR_TEXT_HIGHT) // hoehe wo die Variable hingeschrieben wird
// text_check() MODES
#define MODE_VARIABLE 0x33
#define MODE_TITLE 0x32

typedef struct showText
{
    char title[50];
    char variable[50];
    uint8_t position;
    struct showText *next;
} showText_t;

// User Layer
void *disp_thread(void *arg);
void disp_refresh(ucg_t *ucg);
void disp_init(ucg_t *ucg);
void disp_init_buttons(ucg_t *ucg, kernel_pid_t *disp_pid);
void disp_changePage(ucg_t *ucg, uint8_t page);
uint8_t disp_get_currPage(void);
uint8_t disp_get_myPage(uint8_t position);
void disp_changeVar(ucg_t *ucg, showText_t *space, char *new_var);
void disp_addParam(showText_t *space, char title[], char variable[]);
// 2. Layer
void disp_pin_up_handler(void *arg);
void disp_pin_down_handler(void *arg);
void disp_drawIt(ucg_t *ucg, uint8_t inputpage, char mode);
void disp_drawFrames(ucg_t *ucg, uint8_t inputpage);
// 1. Layer
void disp_check_text(ucg_t *ucg, char *quelle, char *ziel, char mode);
void disp_frame(ucg_t *ucg, int line, int colum);
void disp_title(ucg_t *ucg, char *text, int line, int colum);
void disp_variable(ucg_t *ucg, char *text, int line, int colum);
void disp_clearVar(ucg_t *ucg, int line, int colum);

#endif