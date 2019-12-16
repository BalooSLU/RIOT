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
#include "msg.h"

// Text Beispiele:
#define TEXT_UL "Temperatur:" //text left up
#define TEXT_UR "Humitity:"   //text right up
#define TEXT_DL "Time:"		  //text left down
#define TEXT_DR "State:"	  //text right down

#define DISP_QUEUE_SIZE (8)
extern uint8_t curr_page;
static showText_t showDown[28];
static kernel_pid_t disp_pid;
static char disp_stack[THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF];
static msg_t disp_queue[DISP_QUEUE_SIZE];
static void *disp_thread(void *arg)
{
	msg_t msg;
	ucg_t *ucg = (ucg_t *)arg;

	disp_init(ucg);
	disp_init_buttons(ucg, &disp_pid);

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

	disp_changePage(ucg, 0);
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
		disp_changePage(ucg, curr_page);
	}
	return NULL;
}
int main(void)
{
	ucg_t ucg;
	disp_pid = thread_create(disp_stack, sizeof(disp_stack),
							 THREAD_PRIORITY_MAIN - 1, 0, disp_thread, &ucg, "disp");
	while (1)
	{
	}
	return 0;
}