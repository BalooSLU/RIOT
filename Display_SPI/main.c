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

static char disp_stack[THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF];

int main(void)
{
	static kernel_pid_t disp_pid;
	disp_pid = thread_create(disp_stack, sizeof(disp_stack),
							 THREAD_PRIORITY_MAIN - 1, 0, disp_thread, &disp_pid, "disp");
	while (1)
	{
	}
	return 0;
}