#include <stdio.h>
#include <stdlib.h>

#include "orchestrator.h"

int sendall(int s, char *buf, int *len);
int orch_create_server(orch_handler_addr_t *server);
orch_status_t orch_cli_handler(int sd, const char *buf, int buf_len);
orch_status_t orch_cli_handler_reply(orch_status_t status, char *buf, int *len);
int orch_cli_show_status(char *disp_buf, int *disp_len);
int orch_show_rlist(char *disp_buf, int *disp_len);
int orch_show_plist(char *disp_buf, int *disp_len);
int orch_show_clist(char *disp_buf, int *disp_len);
int orch_show_help(char *disp_buf, int *disp_len);
