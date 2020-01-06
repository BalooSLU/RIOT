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
static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
							size_t maxlen, coap_link_encoder_ctx_t *context);
extern ssize_t _dev_disp_parameter_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);

static char disp_stack[THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF];
/* CoAP resources. Must be sorted by path (ASCII order). */
static const coap_resource_t _resources_disp[] = {
	{"/disp_read_write_Parameter", COAP_GET | COAP_PUT | COAP_POST, _dev_disp_parameter_handler, NULL},
};
static const char *_link_params[] = {
	";ct=0;rt=\"count\"",
	NULL};

static gcoap_listener_t _listener = {
	&_resources_disp[0],
	ARRAY_SIZE(_resources_disp),
	_encode_link,
	NULL};
/* Adds link format params to resource list */
static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
							size_t maxlen, coap_link_encoder_ctx_t *context)
{
	ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);
	if (res > 0)
	{
		if (_link_params[context->link_pos] && (strlen(_link_params[context->link_pos]) < (maxlen - res)))
		{
			if (buf)
			{
				memcpy(buf + res, _link_params[context->link_pos],
					   strlen(_link_params[context->link_pos]));
			}
			return res + strlen(_link_params[context->link_pos]);
		}
	}

	return res;
}

int main(void)
{
	static kernel_pid_t disp_pid;
	gcoap_register_listener(&_listener);

	disp_pid = thread_create(disp_stack, sizeof(disp_stack),
							 THREAD_PRIORITY_MAIN - 1, 0, disp_thread, &disp_pid, "disp");
	while (1)
	{
	}
	return 0;
}