#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <signal.h>

#include "cli.h"

char *prompt = CLI_PROMPT;
extern cli_cmd_node_t g_cli_cmd_node[];
cli_client_param_t g_cli_client;
int cli_debug = 0;

void cli_signal_handler(int signum)
{
    if (signum == SIGPIPE) {
        printf("Server Connection Closed\n");
        printf("Restart Cli Again\n");
    } else if (signum == SIGINT) {
        printf("Exiting Cli.....\n");
        cli_exit_client(&g_cli_client);
    } else {
        printf("Unknown Signal\n");
        cli_exit_client(&g_cli_client);
    }
}

cli_status_t cli_client_init(cli_client_param_t *client)
{
    if(!client)
        return CLI_STATUS_ERROR;

    client->type = CLI_SERVER_TYPE;
    client->port = CLI_SERVER_PORT;
    strcpy(client->ipv4_addr, CLI_SERVER_IP);
    strcpy(client->unix_path, CLI_SERVER_PATH);
    DEBUG_PRINT("Init Cli Client..... Done!\n");

    return CLI_STATUS_SUCCESS;
}

cli_status_t cli_handle_client(cli_client_param_t *client)
{
    char buff[CLI_BUF_LEN] = {0};
    int blen;
    int cmd_len = 0;

    if (!client)
        return CLI_STATUS_ERROR;

    DEBUG_PRINT("Starting Cli Client Handler\n");

    while (1)
    {
        printf("%s", prompt);
        fflush(stdin);
        //scanf("%s",buff);
        fgets(buff, CLI_BUF_LEN, stdin);
        buff[strcspn(buff, "\r\n")] = '\0';
        if (strlen(buff) == 0 || strlen(buff) == 0)
            continue;
        fflush(stdin);
        if (strncmp(buff, "quit", strlen("quit")) == 0) {
            printf("Exiting Command Line Interface...........\n");
            cli_close_client(client);
            DEBUG_PRINT("Exiting Command Line Interface...........\n");
            return CLI_STATUS_SUCCESS;
        }
        if (strncmp(buff, "help", strlen("help")) == 0) {
            cli_print_cmd_node(g_cli_cmd_node);
            continue;
        }
        cmd_len = cli_parse_cmd(&g_cli_cmd_node, buff);
        if (cmd_len != 0) {
            blen = cmd_len;
            if(client->sockfd != 0) {
                DEBUG_PRINT("Writing Sock  Cmd:%s len:%d\n", buff, blen);
                write(client->sockfd, buff, blen);
            } else {
                printf ("Send Socket is not in init or closed\n");
                continue;
            }
        } else {
            continue;
        }
        if(client->sockfd != 0) {
            DEBUG_PRINT("Waiting Read on Socket\n");
            blen = read(client->sockfd, buff, sizeof(buff));
            if(blen > 0) {
                DEBUG_PRINT("Read returned with len:%d\n", blen);
                printf("%s\n", buff);
            }
        } else {
            printf ("Receive Socket is not in init or closed\n");
            continue;
        }
    }
    printf("Exiting Command Handler\n");
    return CLI_STATUS_SUCCESS;
}

cli_status_t cli_create_client(cli_client_param_t *client)
{
    int sockfd;
    struct sockaddr_in servaddr;
    struct sockaddr_un unixaddr;
    if (!client)
        return CLI_STATUS_ERROR;
    if (client->type == CLI_SERVER_TYPE_TCP) {
        if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        {
            perror("Cannot create socket");
        }
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(client->ipv4_addr);
        servaddr.sin_port = htons(client->port);
        client->sockfd = sockfd;

        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
            printf("connection with the server failed...\n");
            cli_close_client(client);
            return CLI_STATUS_FAIL;
        } else {
            printf("connected to the server.. (%s:%d)\n", client->ipv4_addr, client->port);
        }
        client->sockfd = sockfd;
    } else if (client->type == CLI_SERVER_TYPE_UNIX) {
        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        unixaddr.sun_family = AF_UNIX;
        strcpy(unixaddr.sun_path, client->unix_path);
        client->sockfd = sockfd;

        if (connect(sockfd, (struct sockaddr *)&unixaddr, sizeof(unixaddr)) != 0) {
            printf("connection with the server failed...\n");
            cli_close_client(client);
            return CLI_STATUS_FAIL;
        } else {
             printf("connected to the server..(%s:%d)\n", client->unix_path, client->port);
        }
    } else {
        printf("Invalid Connection param....Return\n");
        return CLI_STATUS_ERROR;
    }
    //cli_handle_client(client);
    return CLI_STATUS_SUCCESS;
}

cli_status_t cli_close_client(cli_client_param_t *client)
{
    if (!client)
        return CLI_STATUS_ERROR;
    if (client->sockfd == 0)
        return CLI_STATUS_ERROR;

    if (client->sockfd != 0)
        close(client->sockfd);
    client->sockfd = 0;

    return CLI_STATUS_SUCCESS;
}

cli_status_t cli_exit_client(cli_client_param_t *client)
{
    if (!client)
        return CLI_STATUS_ERROR;
    if (client->sockfd == 0)
        return CLI_STATUS_ERROR;
    cli_close_client(client);
    exit(1);
}

int cli_parse_cmd(cli_cmd_node_t *cmd_node, const char *cmd)
{
    int i, cmd_len = 0;
    int cmd_arr_len = sizeof(g_cli_cmd_node)/sizeof(cli_cmd_node_t);
    char *command = NULL;
    if(!cmd || !cmd_node)
        return 0;

    DEBUG_PRINT("Parsing Cli Cmd:%s in Cmd Node:%p\n", cmd, cmd_node);
    command = cmd;
    for( i = 0 ; i < cmd_arr_len ; i++ )
    {
        command = cmd_node[i].cmd;
        /* this is a special case for exmlX commands - will improve later */ //TODO
        if(!strncmp(cmd, "exml", strlen("exml"))) {
            cmd_len = strlen(cmd);
            DEBUG_PRINT("Parsing Special xml Cli Cmd:%s Len:%d .. Success\n", cmd, cmd_len);
            return cmd_len;
        }
        if(strncmp(cmd, command, strlen(command)) == 0)
        {
            cmd_len = strlen(command);
            DEBUG_PRINT("Parsing Cli Cmd:%s Len:%d .. Success\n", command, cmd_len);
            return cmd_len;
        }
    }
    printf("No Such Command  --  %s\n", cmd);
    cli_print_cmd_node(cmd_node);

    return 0;
}

cli_status_t cli_print_cmd_node(cli_cmd_node_t *cli_node)
{
    int i;
    if(!cli_node)
        return CLI_STATUS_ERROR;

    int cmd_arr_len = sizeof(g_cli_cmd_node)/sizeof(cli_cmd_node_t);
    printf("Command          Description\n");
    printf("---------------- -----------------------------------\n");
    for( i = 0 ; i < cmd_arr_len ; i++ )
    {
        printf("%-17s%s\n", cli_node[i].cmd, cli_node[i].cmd_desc);
    }
    return CLI_STATUS_SUCCESS;
}

cli_status_t cli_start()
{
    cli_status_t res;
    res = cli_client_init(&g_cli_client);
    if (res != CLI_STATUS_SUCCESS)
        return CLI_STATUS_FAIL;
    res = cli_create_client(&g_cli_client);
    if (res != CLI_STATUS_SUCCESS)
        return CLI_STATUS_FAIL;
    res = cli_handle_client(&g_cli_client);
    if (res != CLI_STATUS_SUCCESS)
        return CLI_STATUS_FAIL;
    cli_close_client(&g_cli_client);

    return CLI_STATUS_SUCCESS;
}

int main()
{
    signal(SIGINT, cli_signal_handler);
    signal(SIGPIPE, cli_signal_handler);
    cli_start();
    return CLI_STATUS_SUCCESS;
}





