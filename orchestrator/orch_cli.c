#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "orch_cli.h"
#include "orchestrator.h"

orch_status_t orch_cli_handler(int sd, const char *buf, int buf_len)
{
    char display_buf[50000];
    int disp_len = 0;
    if (!buf)
        return ORCH_STATUS_ERROR;
    DEBUG_PRINT("BUF=%s  :len=%d\n", buf, buf_len);

    if (strcmp(buf,"help") == 0) {
        strcpy(buf,"show_r\nshow_p\nshow_c\n ");
    } else if (strcmp(buf,"show_p") == 0){
        strcpy(buf,"Not Implemented Yet\n ");
    } else if (strstr(buf, "show_r") != NULL){
        orch_show_rlist(display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (strstr(buf, "show_p") != NULL){
        orch_show_plist(display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (strstr(buf, "show_c") != NULL){
        orch_show_clist(display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (strstr(buf, "help") != NULL){
        orch_show_help(display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (strstr(buf, "show") != NULL){
        strcpy(buf,"Display Show\n ");
         orch_dump_all_list();
        orch_cli_show_status(display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else {
        strcpy(buf,"Unknown Cmd-Use >help\n ");
    }
    return ORCH_STATUS_SUCCESS;
}

orch_status_t orch_cli_handler_reply(orch_status_t status, char *buf, int *len)
{
    
}
int orch_show_help(char *disp_buf, int *disp_len)
{
    int len = 0;
    char *buf = disp_buf;
    len += sprintf(disp_buf+len, "=================CMD HELP===========\n");
    len += sprintf(disp_buf+len, "help          :Help on Commands\n");
    len += sprintf(disp_buf+len, "show          :Show Current Status\n");
    len += sprintf(disp_buf+len, "show_p        :Show Pending Status\n");
    len += sprintf(disp_buf+len, "show_r        :Show Running Status\n");
    len += sprintf(disp_buf+len, "show_c        :Show Completed Status\n");

    *disp_len = len;
    return ORCH_STATUS_SUCCESS;
}

int orch_show_rlist(char *disp_buf, int *disp_len)
{
    int len = 0;
    char *buf = disp_buf;
    orch_cmd_list_t *p = NULL;
    orch_cmd_t *cmd = NULL;
    len += sprintf(disp_buf+len, "=================RUNNING STATUS===========\n");
    len += sprintf(disp_buf+len, "RUNNING:\n");
    for( p = run_list ; p != NULL ; p = p->next )
    {
        cmd = p->cmd;
        len += sprintf(disp_buf+len, "CMD [ %d | %s | %s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "=================STATUS END===========\n");
    *disp_len = len;
}

int orch_show_plist(char *disp_buf, int *disp_len)
{
    int len = 0;
    char *buf = disp_buf;
    orch_cmd_list_t *p = NULL;
    orch_cmd_t *cmd = NULL;
    len += sprintf(disp_buf+len, "=================PENDING STATUS===========\n");
    len += sprintf(disp_buf+len, "PENDING:\n");
    for( p = pend_list ; p != NULL ; p = p->next )
    {
        cmd = p->cmd;
        len += sprintf(disp_buf+len, "CMD [ %d | %s | %s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "=================STATUS END===========\n");
    *disp_len = len;
}

int orch_show_clist(char *disp_buf, int *disp_len)
{
    int len = 0;
    char *buf = disp_buf;
    orch_cmd_list_t *p = NULL;
    orch_cmd_t *cmd = NULL;
    len += sprintf(disp_buf+len, "=================COMPLETE STATUS===========\n");
    len += sprintf(disp_buf+len, "CIMPLETE:\n");
    for( p = done_list ; p != NULL ; p = p->next )
    {
        cmd = p->cmd;
        len += sprintf(disp_buf+len, "CMD [ %d | %s | %s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "=================STATUS END===========\n");
    *disp_len = len;
}

int orch_cli_show_status(char *disp_buf, int *disp_len)
{
    int len = 0;
    char *buf = disp_buf;
    orch_cmd_list_t *p = NULL;
    orch_cmd_t *cmd = NULL;

    len += sprintf(disp_buf+len, "=================STATUS===========\n");
    len += sprintf(disp_buf+len, "RUNNING:\n");
    for( p = run_list ; p != NULL ; p = p->next )
    {
        cmd = p->cmd;
        len += sprintf(disp_buf+len, "CMD [ %d | %s | %s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "PENDING:\n");
    for( p = pend_list ; p != NULL ; p = p->next )
    {
        cmd = p->cmd;
        len += sprintf(disp_buf+len, "CMD [ %d | %s | %s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "COMPLETED:\n");
    for( p = done_list ; p != NULL ; p = p->next )
    {
        cmd = p->cmd;
        len += sprintf(disp_buf+len, "CMD [ %d | %s | %s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "=================STATUS END===========\n");
    *disp_len = len;
}


int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 onm failure, 0 on success
}


