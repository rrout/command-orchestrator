#include <stdio.h>
#include <stdlib.h>

#include "orchestrator.h"

int sendall(int s, char *buf, int *len);
int orch_create_server(orch_handler_addr_t *server);
orch_status_t orch_cli_handler(int sd, char *buf, int buf_len);
orch_status_t orch_cli_handler_reply(orch_status_t status, char *buf, int *len);
int orch_cli_show_status(char *command, char *disp_buf, int *disp_len);
int orch_show_rlist(char *command, char *disp_buf, int *disp_len);
int orch_show_plist(char *command, char *disp_buf, int *disp_len);
int orch_show_clist(char *command, char *disp_buf, int *disp_len);
int orch_show_help(char *command, char *disp_buf, int *disp_len);
int orch_cli_exec_cmd(char *command, char *disp_buf, int *disp_len);
orch_status_t orch_cli_exec_dispatch_cmd(int cmd_num);
