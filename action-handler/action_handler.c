#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "action_handler.h"

ah_status_t action_handler(const char *buf, int buf_len)
{
    ah_action_data_t act_data = {0};
    if (!buf)
        return AH_STATUS_ERROR;

    memcpy(&act_data, buf, sizeof(act_data));
    printf("============== ACTION DATA ==============\n");
    printf("CMD-ID   = %d\n", act_data.cmd_id);
    printf("CMD      = %s\n", act_data.cmd);
    printf("CMD DESC = %s\n", act_data.cmd_desc);
    printf("============== ACTION EXEC ==============\n");
    system(act_data.cmd);
    printf("============= ACTION STATUS =============\n");
    printf("CMD RESP = %s\n", "AH_SUCCESS");
    return AH_STATUS_SUCCESS;
}

ah_status_t action_handler_reply(ah_status_t status, char *buf, int *len)
{
    ah_reply_data_t reply_data = {0};
    if (!buf)
        return AH_STATUS_ERROR;
    if (*len <= sizeof(reply_data))
        return AH_STATUS_ERROR;

    switch (status)
    {
        case AH_STATUS_SUCCESS:
        {
            reply_data.status = AH_STATUS_SUCCESS;
            strcpy(&reply_data.status_desc, "Command exec success");
        }
        break;
        case AH_STATUS_FAIL:
        case AH_STATUS_ERROR:
        {
            reply_data.status = AH_STATUS_FAIL;
            strcpy(&reply_data.status_desc, "Command exec failed");
        }
        break;
        default:
        {
            reply_data.status = AH_STATUS_UNKNOWN;
            strcpy(&reply_data.status_desc, "Unknown error");
        }
    }
    memcpy(buf, &reply_data, sizeof(reply_data));
    *len = sizeof(reply_data);
    return AH_STATUS_SUCCESS;
}

