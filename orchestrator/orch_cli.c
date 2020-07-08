#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "orch_cli.h"
#include "orchestrator.h"

orch_status_t orch_cli_handler(int sd, char *buf, int buf_len)
{
    char display_buf[50000];
    int disp_len = 0;
    if (!buf)
        return ORCH_STATUS_ERROR;
    DEBUG_PRINT("BUF=%s  :len=%d\n", buf, buf_len);
    buf[buf_len] = '\0';
    DEBUG_PRINT("STRINGED BUF=%s  :strlen=%d\n", buf, strlen(buf));
    DEBUG_PRINT("Exec Cmd: %s\n", buf);

    if (!strncmp(buf, "help", strlen("help"))) {
        orch_show_help(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (!strncmp(buf, "showr", strlen("showr"))) {
        orch_show_rlist(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (!strncmp(buf, "showp", strlen("showp"))) {
        orch_show_plist(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (!strncmp(buf, "showc", strlen("showc"))) {
        orch_show_clist(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (!strncmp(buf, "show", strlen("show"))) {
        orch_cli_show_status(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (!strncmp(buf, "show", strlen("show"))) {
        strcpy(buf,"Not Implemented Yet\n ");
    } else if (!strncmp(buf, "exec0", strlen("exec0"))) {
        orch_cli_exec_cmd(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (!strncmp(buf, "exec1", strlen("exec1"))) {
        orch_cli_exec_cmd(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (!strncmp(buf, "exec2", strlen("exec2"))) {
        orch_cli_exec_cmd(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (!strncmp(buf, "exec3", strlen("exec3"))) {
        orch_cli_exec_cmd(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (!strncmp(buf, "exec4", strlen("exec4"))) {
        orch_cli_exec_cmd(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (!strncmp(buf, "exec5", strlen("exec5"))) {
        orch_cli_exec_cmd(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else if (strstr(buf, "show") != NULL){
        strcpy(buf,"Display Show\n ");
         orch_dump_all_list();
        orch_cli_show_status(buf, display_buf, &disp_len);
        sendall(sd, display_buf, &disp_len);
    } else {
        strcpy(buf,"Unknown Cmd-Use >help\n ");
        sendall(sd, display_buf, &disp_len);
    }
    return ORCH_STATUS_SUCCESS;
}

orch_status_t orch_cli_handler_reply(orch_status_t status, char *buf, int *len)
{
    
}
int orch_show_help(char *command, char *disp_buf, int *disp_len)
{
    int len = 0;
    char *buf = disp_buf;
    len += sprintf(disp_buf+len, "=================CMD HELP===========\n");
    len += sprintf(disp_buf+len, "help          :Help on Commands\n");
    len += sprintf(disp_buf+len, "show          :Show Current Status\n");
    len += sprintf(disp_buf+len, "showp         :Show Pending Status\n");
    len += sprintf(disp_buf+len, "showr         :Show Running Status\n");
    len += sprintf(disp_buf+len, "showc         :Show Completed Status\n");

    *disp_len = len;
    return ORCH_STATUS_SUCCESS;
}

int orch_show_rlist(char *command, char *disp_buf, int *disp_len)
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
        len += sprintf(disp_buf+len, "CMD [ %-3d | %-10s | %-30s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "=================STATUS END===========\n");
    *disp_len = len;
}

int orch_show_plist(char *command, char *disp_buf, int *disp_len)
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
        len += sprintf(disp_buf+len, "CMD [ %-3d | %-10s | %-30s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "=================STATUS END===========\n");
    *disp_len = len;
}

int orch_show_clist(char *command, char *disp_buf, int *disp_len)
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
        len += sprintf(disp_buf+len, "CMD [ %-3d | %-10s | %-30s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "=================STATUS END===========\n");
    *disp_len = len;
}

int orch_cli_show_status(char *command, char *disp_buf, int *disp_len)
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
        len += sprintf(disp_buf+len, "CMD [ %-3d | %-10s | %-30s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "PENDING:\n");
    for( p = pend_list ; p != NULL ; p = p->next )
    {
        cmd = p->cmd;
        len += sprintf(disp_buf+len, "CMD [ %-3d | %-10s | %-30s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "COMPLETED:\n");
    for( p = done_list ; p != NULL ; p = p->next )
    {
        cmd = p->cmd;
        len += sprintf(disp_buf+len, "CMD [ %-3d | %-10s | %-30s ]\n", cmd->cmd_id, cmd->cmd_exe, cmd->cmd_desc);
    }
    len += sprintf(disp_buf+len, "=================STATUS END===========\n");
    *disp_len = len;
}

orch_status_t orch_cli_exec_dispatch_cmd(int cmd_num)
{
    orch_cmd_t *cmd = NULL;
    cmd = (orch_cmd_t *)malloc(sizeof(orch_cmd_t));
    memset(cmd, 0, sizeof(orch_cmd_t));
    orch_gen_cmd(cmd_num, cmd);
    DEBUG_PRINT("Cli Generated cmd[%d] = %s\n", cmd->cmd_id, cmd->cmd_exe);
    DEBUG_PRINT("Dispatched from Cli\n");
    orch_dispatch(cmd);

    return ORCH_STATUS_SUCCESS;
}

int orch_cli_exec_cmd(char *command, char *disp_buf, int *disp_len)
{
    int len = 0;
    char *buf = disp_buf;
    int cmd_num;
    if (!command)
        return ORCH_STATUS_ERROR;
    DEBUG_PRINT("Invoked Command dispatcher Cli with Cmd:%s\n", command);
    if (!strncmp(command, "exec0", strlen("exec0"))) {
        cmd_num = 0;
    } else if (!strncmp(command, "exec1", strlen("exec1"))) {
        cmd_num = 1;
    } else if (!strncmp(command, "exec2", strlen("exec2"))) {
        cmd_num = 2;
    } else if (!strncmp(command, "exec3", strlen("exec3"))) {
        cmd_num = 3;
    } else if (!strncmp(command, "exec4", strlen("exec4"))) {
        cmd_num = 4;
    } else if (!strncmp(command, "exec5", strlen("exec5"))) {
        cmd_num = 5;
    } else {
        printf("Unknown Command\n");
        DEBUG_PRINT("Unknown Cmd\n");
        return ORCH_STATUS_FAIL;
    }

    orch_cli_exec_dispatch_cmd(cmd_num);
    len += sprintf(disp_buf+len, "Dispatched Cmd: %s\n", command);
    *disp_len = len;
    return ORCH_STATUS_SUCCESS;
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


