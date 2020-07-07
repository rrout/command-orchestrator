#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

#include "orchestrator.h"

orch_cmd_list_t *run_list = NULL;
orch_cmd_list_t *done_list = NULL;
orch_cmd_list_t *pend_list = NULL;

pthread_mutex_t run_list_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t done_list_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pend_list_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t worker_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t worker_lock = PTHREAD_MUTEX_INITIALIZER;


orch_handler_addr_t g_handler;
orch_handler_addr_t g_cli_server;
pthread_t g_worker_thread;
pthread_t g_cli_thread;
int orch_debug = 1;

void orch_init()
{
    g_handler.handler_type = ORCH_HANDLER_TYPE_TCP;
    strcpy(g_handler.ipv4_addr, "127.0.0.1");
    g_handler.port = 8888;

    g_cli_server.handler_type = ORCH_HANDLER_TYPE_TCP;
    strcpy(g_cli_server.ipv4_addr, "127.0.0.1");
    g_cli_server.port =9000;
}

orch_status_t orch_check_handler_server_connectivity()
{
    orch_status_t res;
    orch_handler_connection_t con = {0};
    res = orch_create_handler_connection(&g_handler, &con);
    orch_close_handler_connection(&con);
    if (res != ORCH_STATUS_SUCCESS)
        return ORCH_STATUS_FAIL;

    return ORCH_STATUS_SUCCESS;
}


void orch_parse_cmd(char *file, orch_cmd_t *cmd)
{

}

void orch_gen_cmd(int cmdtype, orch_cmd_t *cmd)
{
    if (!cmd)
        return;

    switch(cmdtype)
    {
        case 1:
            {
                cmd->cmd_id = 1;
                strcpy(cmd->cmd,"CMD 1");
                strcpy(cmd->cmd_exe,"ls");
                strcpy(cmd->cmd_desc, "List");
                cmd->exec_wait = 10;
                cmd->dependency[0] = 3;
                cmd->dependency_count = 1;
            }
            break;
        case 2:
            {
                cmd->cmd_id = 2;
                strcpy(cmd->cmd,"CMD 2");
                strcpy(cmd->cmd_exe,"pwd");
                strcpy(cmd->cmd_desc, "Present Working Directory");
                cmd->exec_wait = 10;
                cmd->dependency[0] = 1;
                cmd->dependency_count = 1;
            }
            break;
        case 3:
            {
                cmd->cmd_id = 3;
                strcpy(cmd->cmd,"CMD 3");
                strcpy(cmd->cmd_exe,"date");
                strcpy(cmd->cmd_desc, "Print Date");
                cmd->exec_wait = 10;
                cmd->dependency_count = 0;
            }
            break;
        case 4:
            {
                cmd->cmd_id = 4;
                strcpy(cmd->cmd,"CMD 4");
                strcpy(cmd->cmd_exe,"ps");
                strcpy(cmd->cmd_desc, "Process Status");
                cmd->exec_wait = 10;
                cmd->dependency[0] = 1;
                cmd->dependency[1] = 2;
                cmd->dependency_count = 2;
            }
            break;
        default:
            {
                cmd->cmd_id = 0;
                strcpy(cmd->cmd,"CMD 0");
                strcpy(cmd->cmd_exe,"ls -altr");
                strcpy(cmd->cmd_desc, "List Time Ascending");
                cmd->exec_wait = 10;
                cmd->dependency_count = 0;
            }
            break;
    }
}

orch_cmd_list_t * orch_add_list(orch_cmd_list_t *list, orch_cmd_t *cmd)
{
    orch_cmd_list_t *node = NULL;
    if (!cmd)
         return list;

    node = (orch_cmd_list_t *) malloc (sizeof(orch_cmd_list_t));
    node->cmd = cmd;
    node->next = list;
    return node;
}

orch_cmd_list_t * orch_del_list(orch_cmd_list_t *list, orch_cmd_t *cmd)
{
    orch_cmd_list_t *p = NULL, *q = NULL;
    if (!list || !cmd)
        return list;
    if((list->cmd->cmd_id == cmd->cmd_id) && (list->cmd->dependency == cmd->dependency)) {
        p = list;
        list = list->next;
        free (p);
        return list;
    }
    p = q = list;
    while (p)
    {
        if ((p->cmd->cmd_id == cmd->cmd_id) && (p->cmd->dependency == cmd->dependency)) {
            q->next = p->next;
            free(p);
            return list;
        }
        q = p;
        p = p->next;
    }
    return list;
}

orch_cmd_list_t *orch_dump_list(orch_cmd_list_t *list)
{
    orch_cmd_list_t *p;
    orch_cmd_t *cmd;
    DEBUG_PRINT("Dump List\n");
    for( p = list ; p != NULL ; p = p->next )
    {
        cmd = p->cmd;
        printf("Cmd=%s, cmdId=%d, dp=%d, dpc=%d\n", cmd->cmd_exe, cmd->cmd_id,cmd->dependency_count, cmd->dependency_count_cur);
    }
}

orch_cmd_list_t *orch_destroy_list(orch_cmd_list_t *list)
{
    orch_cmd_list_t *p = NULL, *q = NULL;
    if (!list)
        return list;
    p = list;
    while (p)
    {
        q = p;
        p = p->next;
        free(q);
    }
    return NULL;
}

orch_status_t orch_add_pend_list(orch_cmd_t *cmd)
{
    orch_cmd_list_t *list = pend_list;
    DEBUG_PRINT("Add Cmd %s to Pending List\n",cmd->cmd_exe);
    if (!cmd)
        return ORCH_STATUS_ERROR;
    pend_list = orch_add_list(list, cmd);
    orch_dump_list(list);
    return ORCH_STATUS_SUCCESS;
}

orch_cmd_list_t * orch_del_pend_list(orch_cmd_t *cmd)
{
    orch_cmd_list_t *list = pend_list;
    if (!list || !cmd)
        return ORCH_STATUS_ERROR;
    pend_list = orch_del_list(list, cmd);
    return ORCH_STATUS_SUCCESS;
}

orch_status_t orch_add_run_list(orch_cmd_t *cmd)
{
    orch_cmd_list_t *list = run_list;
    if (!cmd)
        return ORCH_STATUS_ERROR;
    run_list = orch_add_list(list, cmd);
    return ORCH_STATUS_SUCCESS;
}

orch_cmd_list_t * orch_del_run_list(orch_cmd_t *cmd)
{
    orch_cmd_list_t *list = run_list;
    if (!list || !cmd)
        return ORCH_STATUS_ERROR;
    run_list = orch_del_list(list, cmd);
    return ORCH_STATUS_SUCCESS;
}

orch_status_t orch_add_done_list(orch_cmd_t *cmd)
{
    orch_cmd_list_t *list = done_list;
    if (!cmd)
        return ORCH_STATUS_ERROR;
    done_list = orch_add_list(list, cmd);
    return ORCH_STATUS_SUCCESS;
}

orch_cmd_list_t * orch_del_done_list(orch_cmd_t *cmd)
{
    orch_cmd_list_t *list = done_list;
    if (!list || !cmd)
        return ORCH_STATUS_ERROR;
    done_list = orch_del_list(list, cmd);
    return ORCH_STATUS_SUCCESS;
}

orch_status_t orch_del_pend_list_done_items()
{
    orch_cmd_list_t *list = pend_list;
    orch_cmd_list_t *p = pend_list;
    orch_cmd_list_t *tmp_list = NULL;
    for( p = pend_list ; p != NULL ; p = p->next )
    {
        if ((p->cmd->dependency_count_cur != 0) || (p->cmd->run_now != 1)) {
            tmp_list = orch_add_list(tmp_list, p->cmd);
        }
    }
    orch_dump_list(pend_list);
    orch_dump_list(tmp_list);

    pend_list = orch_destroy_list(pend_list);

    for( p = tmp_list ; p != NULL ; p = p->next )
    {
        pend_list = orch_add_list(pend_list, p->cmd);
    }

    tmp_list = orch_destroy_list(tmp_list);
    orch_dump_list(pend_list);
/*
    while (p)
    {
        if((p->cmd->dependency_count_cur == 0) && (p->cmd->run_now == 1)) {
            tmp = p;
            free(tmp);
            p = p->next;
        }
    }
*/

    return ORCH_STATUS_SUCCESS;
}

orch_status_t orch_dump_all_list()
{
    printf ("=========================SHOW LIST=========================\n");
    printf ("RUNNING LIST\n");
    orch_dump_list(run_list);
    printf ("PENDING LIST\n");
    orch_dump_list(pend_list);
    printf ("COMPLETED LIST\n");
    orch_dump_list(done_list);
    printf ("=========================SHOW  END=========================\n");
}

orch_status_t orch_create_handler_connection(orch_handler_addr_t *handler, orch_handler_connection_t *con)
{
    int sockfd;
    struct sockaddr_in servaddr;
    struct sockaddr_un unixaddr;
    if (!handler || !con)
        return ORCH_STATUS_ERROR;
    if (handler->handler_type == ORCH_HANDLER_TYPE_TCP) {
        if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        {
            perror("Cannot create socket");
        }
        servaddr.sin_family = AF_INET; 
        servaddr.sin_addr.s_addr = inet_addr(handler->ipv4_addr); 
        servaddr.sin_port = htons(handler->port);
        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) { 
            printf("connection with the server failed...\n"); 
            return ORCH_STATUS_FAIL;
        } else {
            printf("connected to the server..\n");
        }
    } else if (handler->handler_type == ORCH_HANDLER_TYPE_UNIX) {
        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        unixaddr.sun_family = AF_UNIX;
        strcpy(unixaddr.sun_path, handler->unix_path);

        if (connect(sockfd, (struct sockaddr *)&unixaddr, sizeof(unixaddr)) != 0) {
            printf("connection with the server failed...\n");
            return ORCH_STATUS_FAIL;
        } else {
             printf("connected to the server..\n");
        }
    } else {
        printf("Invalid Connection param....Return\n");
        return ORCH_STATUS_ERROR;
    }

    memcpy(&con->info, handler, sizeof(orch_handler_addr_t));
    con->server_socket = con->service_socket = sockfd;

    return ORCH_STATUS_SUCCESS;
}

orch_status_t orch_send_cmd(orch_handler_connection_t *con, orch_cmd_t *cmd)
{
    char buff[ORCH_BUF_LEN] = {0};
    int blen;
    orch_action_data_t request;
    orch_reply_data_t reply;
    if (!con || !cmd)
        return ORCH_STATUS_ERROR;

    blen = sizeof(orch_action_data_t);
    request.cmd_id = cmd->cmd_id;
    strcpy(request.cmd_desc, cmd->cmd_desc);
    strcpy(request.cmd, cmd->cmd_exe);

    memcpy(buff, &request, sizeof(orch_action_data_t));
    write(con->service_socket, buff, blen);

    blen = read(con->service_socket, buff, sizeof(buff));
    memcpy(&reply, buff, sizeof(reply));
    if (reply.status != ORCH_STATUS_SUCCESS) {
        printf ("Reply status = %d\n",reply.status);
        printf ("Reply status des = %s\n", reply.status_desc);
        return ORCH_STATUS_FAIL;
    }
    return ORCH_STATUS_SUCCESS;
}

orch_status_t orch_close_handler_connection(orch_handler_connection_t *con)
{
    if (!con)
         return ORCH_STATUS_ERROR;
    close(con->service_socket);
    con->server_socket = con->service_socket = 0;
}

orch_status_t orch_create_cli_handler_server(orch_handler_addr_t *handler)
{
    while (1)
    {
        //sleep(10);
        DEBUG_PRINT("Running [cli_handler_server] \n");
        orch_create_server(handler);
    }
    return ORCH_STATUS_SUCCESS;
}

void *orch_cli_handler_thread(void *handler)
{
    orch_status_t res;
    res = orch_create_cli_handler_server((orch_handler_addr_t *)handler);
}

orch_status_t orch_create_cli_server(orch_handler_addr_t *handler)
{
    int res;
    DEBUG_PRINT("Starting CLI Thread\n");
    res = pthread_create( &g_cli_thread, NULL, orch_cli_handler_thread, (void*) handler);
    return ORCH_STATUS_SUCCESS;
}

orch_status_t orch_process_cmd(orch_cmd_t *cmd)
{
    int res;
    orch_handler_connection_t con = {0};
    DEBUG_PRINT("PROCESS -------------------------------------> | CMD=%s |\n", cmd->cmd_exe);
    res = orch_create_handler_connection(&g_handler, &con);
    res = orch_send_cmd(&con, cmd);
    orch_close_handler_connection(&con);
    DEBUG_PRINT("PROCESS RES -----------> | CMD=%s | --------> | RES = SUCC|\n", cmd->cmd_exe);
    DEBUG_PRINT("Proc CMD=%s Complete - Sleep %d Seconds\n",cmd->cmd_exe, cmd->exec_wait);
    sleep(cmd->exec_wait);
    return res;
}

orch_status_t orch_dispatcher(orch_cmd_t *cmd)
{
    int runable_cmd = 0;
    orch_status_t res;
    if (!cmd)
        return ORCH_STATUS_ERROR;
    if (cmd->dependency_count == 0) {
        runable_cmd = 1;
    } else if (cmd->run_now == 1) {
        runable_cmd = 1;
    } else {
        if (cmd->dependency_count > 0) {
            cmd->dependency_count_cur = cmd->dependency_count;
            pthread_mutex_lock(&pend_list_lock);
            DEBUG_PRINT("ADD PEND - CMD=%s\n",cmd->cmd_exe);
            orch_add_pend_list(cmd);
            pthread_mutex_unlock(&pend_list_lock);
        } else {
            runable_cmd = 1;
        }
    }

    if (runable_cmd) {
        DEBUG_PRINT("RUNABLE  CMD=%s\n",cmd->cmd_exe);

        pthread_mutex_lock(&run_list_lock);
        DEBUG_PRINT("ADD RUN - CMD=%s\n",cmd->cmd_exe);
        orch_add_run_list(cmd);
        pthread_mutex_unlock(&run_list_lock);

        DEBUG_PRINT("PROCESS RUN - CMD=%s\n",cmd->cmd_exe);
        res = orch_process_cmd(cmd);

        pthread_mutex_lock(&run_list_lock);
        DEBUG_PRINT("DEL RUN - CMD=%s\n",cmd->cmd_exe);
        orch_del_run_list(cmd);
        pthread_mutex_unlock(&run_list_lock);

        pthread_mutex_lock(&done_list_lock);
        DEBUG_PRINT("ADD DONE - CMD=%s\n",cmd->cmd_exe);
        orch_add_done_list(cmd);
        pthread_mutex_unlock(&done_list_lock);
    }

    /*
     * As we complete one running command
     * call reschedule to check if any command from
     * pending_list become runable
     * This will signal the worker thread to annalyze
     * pending list and dispatch runable cmds to Handler
     */
    DEBUG_PRINT("Call RESCHEDULE\n");
    orch_reschedule();
}

orch_status_t orch_reschedule()
{
    int res;
    /*
     * Check if any cmd runable from pend list
     */
    DEBUG_PRINT("Rescheduling Pending List --- Start\n");
    orch_resolve_dependency(pend_list, run_list);
    DEBUG_PRINT("P with R :DEP Resolve - Done!\n");
    orch_resolve_dependency(pend_list, done_list);
    DEBUG_PRINT("P with D :DEP Resolve - Done!\n");
    orch_resolve_self_dependency(pend_list);
    DEBUG_PRINT("P with P :DEP Resolve - Done!\n");
    DEBUG_PRINT("Rescheduling Pending List ---- Complete\n");
    signal_worker_thread();
}

orch_status_t orch_resolve_dependency(orch_cmd_list_t *dst_list, orch_cmd_list_t *src_list)
{
    int i;
    orch_cmd_list_t *pend_l = NULL;
    orch_cmd_list_t *src_l = NULL;
    orch_cmd_t *pend_cmd = NULL;
    orch_cmd_t *src_cmd = NULL;
    for( pend_l = dst_list ; pend_l != NULL ; pend_l = pend_l->next )
    {
        pend_cmd = pend_l->cmd;
        for( src_l = src_list ; src_l != NULL ; src_l = src_l->next )
        {
            src_cmd = src_l->cmd;
            for( i = 0 ; i <pend_cmd->dependency_count ; i++ )
            {
                if (pend_cmd->dependency[i] == src_cmd->cmd_id)
                {
                    if (pend_cmd->dependency_count_cur > 0)
                        pend_cmd->dependency_count_cur--;
               }
            }
        }
    }
}

orch_status_t orch_resolve_self_dependency(orch_cmd_list_t *dst_list){
    int i;
    orch_cmd_list_t *pend_l = NULL;
    orch_cmd_list_t *src_l = NULL;
    orch_cmd_t *pend_cmd = NULL;
    orch_cmd_t *src_cmd = NULL;
    for( pend_l = dst_list ; pend_l != NULL ; pend_l = pend_l->next )
    {
        pend_cmd = pend_l->cmd;
        for( src_l = dst_list ; src_l != NULL ; src_l = src_l->next )
        {
            if (pend_l == src_l)
                continue;
            src_cmd = src_l->cmd;
            for( i = 0 ; i <pend_cmd->dependency_count ; i++ )
            {
                if (pend_cmd->dependency[i] == src_cmd->cmd_id) //TODO Count is going beyond as cking against cmd_id, with dep is repetative
                {
                    if(pend_cmd->dependency_count_cur < pend_cmd->dependency_count)
                        pend_cmd->dependency_count_cur++;
                }
            }
        }
    }
}

void *dispatch_thread(void *cmd)
{
    int res;
    orch_cmd_t *cmd_data = (orch_cmd_t *)cmd;
    DEBUG_PRINT("Call dispatcher for CMD %s\n",cmd_data->cmd_exe);
    res = orch_dispatcher(cmd_data);
    DEBUG_PRINT("Thread for for CMD %s | ===========>> DONE\n",cmd_data->cmd_exe);
}

orch_status_t orch_dispatch(orch_cmd_t *cmd)
{
    int res;
    pthread_t thread;
    void *cmd_data = (void *)cmd;
    DEBUG_PRINT("Starting dispatch thread for CMD %s\n", cmd->cmd_exe);
    res = pthread_create( &thread, NULL, dispatch_thread, cmd_data);
    return ORCH_STATUS_SUCCESS;
}

void *worker_thread(void *cmd)
{
    orch_cmd_list_t *pend_l = NULL;
    orch_cmd_t *pend_cmd = NULL;
    DEBUG_PRINT("Start of Worker Thread\n");
    while (1)
    {
        pthread_mutex_lock(&worker_lock);
        pthread_cond_wait(&worker_cond, &worker_lock);
        //pthread_mutex_unlock(&worker_lock);

        DEBUG_PRINT("========= Process Worker Thread ==========\n");

        pthread_mutex_lock(&pend_list_lock);
        pthread_mutex_lock(&run_list_lock);
        DEBUG_PRINT("===== Scanning Pend List\n");
        for( pend_l = pend_list ; pend_l != NULL ; pend_l = pend_l->next )
        {
            pend_cmd = pend_l->cmd;
            DEBUG_PRINT("Scan : (CMD=%s)| DC=%d| DCC=%d\n", pend_cmd->cmd_exe, pend_cmd->dependency_count, pend_cmd->dependency_count_cur);
            if(pend_cmd->dependency_count_cur == 0) {
                DEBUG_PRINT("CURR Dep count = %d (CMD=%s)\n", pend_cmd->dependency_count_cur, pend_cmd->cmd_exe);
                pend_cmd->run_now = 1;
                orch_dispatch(pend_cmd);
            }
        }
        pthread_mutex_unlock(&run_list_lock);

        orch_del_pend_list_done_items();
        pthread_mutex_unlock(&pend_list_lock);

        DEBUG_PRINT("========= Process Worker Thread ========== DONE!\n");
        pthread_mutex_unlock(&worker_lock);
    }
}

void signal_worker_thread()
{
    DEBUG_PRINT("Signaling Worker Thread\n");
    pthread_mutex_lock(&worker_lock);
    pthread_cond_signal(&worker_cond);
    pthread_mutex_unlock(&worker_lock);
    DEBUG_PRINT("Signalled Worker Thread\n");
}

orch_status_t orch_create_run_worker()
{
    int res;
    void *cmd_data = NULL;
    DEBUG_PRINT("Starting Worker Thread\n");
    res = pthread_create( &g_worker_thread, NULL, worker_thread, cmd_data);
    return ORCH_STATUS_SUCCESS;

}

int main()
{
    int i;
    orch_cmd_t *cmd;
    orch_status_t res;

    /*
     * Build cli server
     */
    DEBUG_PRINT("Starting App\n");
    orch_init();
    res = orch_check_handler_server_connectivity();
    if (res != ORCH_STATUS_SUCCESS) {
        printf("Not Able to connect [ handler_server ]\n");
        return ORCH_STATUS_FAIL;
    }
    printf("PORTTTTT = %d\n",g_cli_server.port);
    res = orch_create_cli_server(&g_cli_server);
    if (res != ORCH_STATUS_SUCCESS)
        return ORCH_STATUS_FAIL;
    //sleep(50000);
    res = orch_create_run_worker();
    if (res != ORCH_STATUS_SUCCESS)
        return ORCH_STATUS_FAIL;

    sleep(10);
    DEBUG_PRINT("Sending Cmds\n");
    for( i = 0 ; i < ORCH_MAX_TEST_CMD ; i++ )
    {
        cmd = (orch_cmd_t *)malloc(sizeof(orch_cmd_t));
        memset(cmd, 0, sizeof(orch_cmd_t));
        orch_gen_cmd(i, cmd);
        DEBUG_PRINT("Generated cmd[%d] = %s\n", cmd->cmd_id, cmd->cmd_exe);
        DEBUG_PRINT("Dispatched from Main\n");
        orch_dispatch(cmd);
        sleep(2);
    }
    pthread_join(g_cli_thread, NULL);
    pthread_join(g_worker_thread, NULL);
    return ORCH_STATUS_SUCCESS;
}
