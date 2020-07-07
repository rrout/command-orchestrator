#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __ACTION_HANDLER_H__
#define __ACTION_HANDLER_H__

#define AH_CMD_LEN      64
#define AH_CMD_DESC_LEN 128
#define AH_BUFF_LEN     256

typedef enum {
    AH_STATUS_FAIL,
    AH_STATUS_SUCCESS,
    AH_STATUS_ERROR,
    AH_STATUS_INTERNAL,
    AH_STATUS_UNKNOWN,
    AH_STATUS_MAX,
}ah_status_t;

typedef struct ah_action_data_s {
    int cmd_id;
    char cmd_desc[AH_CMD_DESC_LEN];
    char cmd[AH_CMD_LEN];
}ah_action_data_t;

typedef struct ah_reply_data_s {
    int status;
    char status_desc[AH_CMD_DESC_LEN];
}ah_reply_data_t;


/*
 * Prototype
 */
ah_status_t action_handler(const char *buf, int buf_len);
ah_status_t action_handler_reply(ah_status_t status, char *buf, int *len);

#endif //__ACTION_HANDLER_H__
