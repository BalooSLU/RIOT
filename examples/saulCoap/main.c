/*
 * Copyright (C) 2017 Freie Universität Berlin
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
 * @brief       Example for demonstrating SAUL and the SAUL registry
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include "net/gcoap.h"
#include "saul_reg.h"
#include "shell.h"
#include "fmt.h"

//Prototypes
static uint8_t write_dev(uint8_t value);
static uint16_t read_dev(void);
static uint8_t set_dev(uint8_t value);
static ssize_t _dev_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _dev_list(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
                            size_t maxlen, coap_link_encoder_ctx_t *context);
/* CoAP resources. Must be sorted by path (ASCII order). */
static const coap_resource_t _resources[] = {
    { "/dev_read_write", COAP_GET | COAP_PUT, _dev_handler, NULL },
   // { "/dev_command", COAP_GET | COAP_PUT, _dev_handler, NULL },
    { "/dev_change", COAP_GET | COAP_PUT, _dev_list, NULL },
};

static const char *_link_params[] = {
    ";ct=0;rt=\"count\"",
    NULL
};

static gcoap_listener_t _listener = {
    &_resources[0],
    ARRAY_SIZE(_resources),
    _encode_link,
    NULL
};
/* Define Saul device*/
saul_reg_t *dev;

/* Adds link format params to resource list */
static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
                            size_t maxlen, coap_link_encoder_ctx_t *context) 
{
    ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);
    if (res > 0) {
        if (_link_params[context->link_pos]
                && (strlen(_link_params[context->link_pos]) < (maxlen - res))) {
            if (buf) {
                memcpy(buf+res, _link_params[context->link_pos],
                       strlen(_link_params[context->link_pos]));
            }
            return res + strlen(_link_params[context->link_pos]);
        }
    }

    return res;
}


int main(void)
{
    gcoap_register_listener(&_listener);

    puts("Welcome to RIOT!\n");
    puts("Type `help` for help, type `saul` to see all SAUL devices\n");
		/* Default */
    dev = saul_reg_find_nth(1);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}


static ssize_t _dev_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
  (void)ctx;

  unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));

  switch (method_flag) {
  case COAP_GET:
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    /* write the response buffer with the request count value */
    resp_len += fmt_u16_dec((char *)pdu->payload, read_dev());
    return resp_len;

  case COAP_PUT:
    /* convert the payload to an integer and update the internal
               value */
        if (pdu->payload_len <= 3) {
                char payload[4] = { 0 };
                memcpy(payload, (char *)pdu->payload, pdu->payload_len);
		if(strtoul(payload, NULL, 10)>= 257){
		 return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
		}
		uint8_t num = (uint8_t)strtoul(payload, NULL, 10);
		
		if(write_dev(num))
		{
		   return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
		}else{
		   return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);        
		}
            }
            else {
                return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
            }
   }

  return 0;
}

static ssize_t _dev_list(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
  (void)ctx;

  unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));

  switch (method_flag) {
  case COAP_GET:
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    if (pdu->payload_len >= strlen(dev->name)) {
        memcpy(pdu->payload, dev->name, strlen(dev->name));
        return resp_len + strlen(dev->name);
    }
    else {
        puts("gcoap_cli: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
  
  case COAP_PUT:
            /* convert the payload to an integer and update the internal
               value */
        if (pdu->payload_len <= 3) {
                char payload[4] = { 0 };
                memcpy(payload, (char *)pdu->payload, pdu->payload_len);
		if(strtoul(payload, NULL, 10)>= 257){
		 return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
		}
		uint8_t num = (uint8_t)strtoul(payload, NULL, 10);
		
		if(set_dev(num))
		{
		   return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
		}else{
		   return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);        
		}
            }
            else {
                return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
            }
	}

  return 0;
}


/* this function does check, if the given device is valid */
static uint8_t write_dev(uint8_t value)
{
    int dim;
    phydat_t data;

    memset(&data, 0, sizeof(data));

    dim = read_dev();
    if (dim == -1) {
        return 0;
    }

    data.val[0] = value;

    dim = saul_reg_write(dev, &data);
    if (dim <= 0) {
        if (dim == -ENOTSUP) {
            printf("error: device #%s is not writable\n", dev->name);
		return 0;
        }
        else {
            printf("error: failure to write to device #%s\n", dev->name);
		return 0;
        }
    }
    return 1;
}

/* this function does not check, if the given device is valid */
static uint16_t read_dev(void)
{
    int dim;
    phydat_t res;

    dim = saul_reg_read(dev, &res);
    if (dim <= 0) {
        printf("error: failed to read from device #%s\n", dev->name);
        return -1;
    }

    return (uint16_t)res.val[0];
}

static uint8_t set_dev(uint8_t value)
{
	if(value > 11) return 0;

   dev = NULL;
   dev = saul_reg_find_nth(value);
    if (dev == NULL) {
        puts("error: undefined device given");
        return 0;
    }
  return 1;
}
