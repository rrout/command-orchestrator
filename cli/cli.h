#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __CLI_H__
#define __CLI_H__

#define CLI_BUF_LEN         5000
#define CLI_CMD_LEN         64
#define CLI_CMD_DESC_LEN    64
#define CLI_IPLEN           16
#define CLI_PATH_LEN        64
#define CLI_PROMPT          "CLI> "

#define CLI_LOOPBACK_IP     "127.0.0.1"
#define CLI_SERVER_LOCAL_IP "192.168.0.1"

#define CLI_SERVER_PATH     "/tmp/cli.sock"
#define CLI_SERVER_IP       CLI_LOOPBACK_IP
#define CLI_SERVER_PORT     9000
#define CLI_SERVER_TYPE     CLI_SERVER_TYPE_TCP

extern int cli_debug;
#define DEBUG_PRINT(msg, ...) \
    if( cli_debug == 1) \
    { \
        printf("ORCH:%s(%d): " msg, __func__, __LINE__, ##__VA_ARGS__);  \
    }

typedef enum {
    CLI_SERVER_TYPE_UNIX,
    CLI_SERVER_TYPE_TCP,
    CLI_SERVER_TYPE_MAX,
}cli_client_type_t;

typedef struct cli_client_param_s {
    int sockfd;
    cli_client_type_t type;
    char ipv4_addr[CLI_IPLEN];
    char unix_path[CLI_PATH_LEN];
    int port;
}cli_client_param_t;

typedef struct cli_cmd_node_s {
    char cmd[CLI_CMD_LEN];
    char cmd_desc[CLI_CMD_DESC_LEN];
}cli_cmd_node_t;

typedef enum {
    CLI_STATUS_FAIL,
    CLI_STATUS_SUCCESS,
    CLI_STATUS_ERROR,
    CLI_STATUS_MAX,
}cli_status_t;

cli_status_t cli_client_init(cli_client_param_t *client);
cli_status_t cli_handle_client(cli_client_param_t *client);
cli_status_t cli_create_client(cli_client_param_t *client);
cli_status_t cli_close_client(cli_client_param_t *client);
cli_status_t cli_exit_client(cli_client_param_t *client);
int  cli_parse_cmd(cli_cmd_node_t *cmd_node, const char *cmd);
cli_status_t cli_print_cmd_node(cli_cmd_node_t *cli_node);
cli_status_t cli_start();
cli_cmd_node_t g_cli_cmd_node[] = {
    {
        "help",
        "Command Help"
    },
    {
        "quit",
        "Quit Cli Session"
    },
    {
        "showr",
        "Show Running List"
    },
    {   "showp",
        "Show Pending List"
    },
    {
        "showc",
        "Show Completed List"
    },
    {
        "show",
        "Show Progress"
    },
    {
        "cleanup",
        "Cleanup Completed List"
    },
    {
        "exec0",
        "Execute Command 0 ()"
    },
    {
        "exec1",
        "Execute Command 1 ()"
    },
    {
        "exec2",
        "Execute Command 2 ()"
    },
    {
        "exec3",
        "Execute Command 3 ()"
    },
    {
        "exec4",
        "Execute Command 4 ()"
    },
    {
        "exec5",
        "Execute Command 5 ()"
    },
    {
        "exml<num>",
        "Execute xml Command like <exml3>"
    },
};

#endif //__CLI_H__
