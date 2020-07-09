/************************************************************************
* FILENAME :        orch_cli.h             DESIGN REF: RR-01
* DESCRIPTION :
*       File for Orchistator Cli handling  routines.
* NOTES :
*       These functions are a part of Open Software
*       Copyright GPL-3.0  2020.  All rights reserved.
* AUTHOR    : Rashmi Ranjan Rout        START DATE : 08-JUL-2020
* CHANGES   :
* REF NO  VERSION DATE        WHO         DETAIL
* RR-01   V.0.1   08-JUL-2020 Rashmi      Initil Version
*
************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "orchestrator.h"
#ifndef __ORCH_CLI_H__
#define __ORCH_CLI_H__

#define ORCH_XML_FILE_PATH      "/tmp/orch_xml/"
#define ORCH_XML_FILE_NAME_LEN  100

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

#endif //__ORCH_CLI_H__



