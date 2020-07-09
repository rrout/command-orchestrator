#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __ORCHESTRATOR_H__
#define __ORCHESTRATOR_H__

#define ORCH_CMD_LEN            64
#define ORCH_DEP_MAX            64

#define ORCH_IP_LEN             16
#define ORCH_PATH_LEN           64
#define ORCH_BUF_LEN            512
#define ORCH_CMD_LEN            64
#define ORCH_CMD_DESC_LEN       128

#define ORCH_MAX_TEST_CMD       5

extern int orch_debug;
#define DEBUG_PRINT(msg, ...) \
    if( orch_debug == 1) \
    { \
        printf("ORCH:%s(%d): " msg, __func__, __LINE__, ##__VA_ARGS__);  \
    }

typedef struct orch_cmd_s {
    char cmd[ORCH_CMD_LEN];
    int  cmd_id;
    char cmd_exe[ORCH_CMD_LEN];
    char cmd_desc[ORCH_CMD_LEN];
    int  exec_wait;
    int  dependency[ORCH_DEP_MAX];
    int  dependency_count;
    int  dependency_count_cur;
    int  run_now;
}orch_cmd_t;

typedef struct orch_cmd_list_s {
    orch_cmd_t *cmd;
    struct orch_cmd_list_s *next;
}orch_cmd_list_t;

typedef enum {
    ORCH_STATUS_FAIL,
    ORCH_STATUS_SUCCESS,
    ORCH_STATUS_ERROR,
    ORCH_STATUS_MAX,
}orch_status_t;

typedef enum {
    ORCH_HANDLER_TYPE_TCP,
    ORCH_HANDLER_TYPE_RAW,
    ORCH_HANDLER_TYPE_UNIX,
    ORCH_HANDLER_TYPE_MAX,
}orch_handler_type_t;

typedef struct orch_handler_addr_s {
    orch_handler_type_t handler_type;
    char ipv4_addr[ORCH_IP_LEN];
    char unix_path[ORCH_PATH_LEN];
    int port;
}orch_handler_addr_t;

typedef struct orch_handler_connection_s {
    orch_handler_addr_t info;
    int server_socket;
    int service_socket;
}orch_handler_connection_t;

typedef struct orch_action_data_s {
    int cmd_id;
    char cmd_desc[ORCH_CMD_DESC_LEN];
    char cmd[ORCH_CMD_LEN];
}orch_action_data_t;

typedef struct orch_reply_data_s {
    int status;
    char status_desc[ORCH_PATH_LEN];
}orch_reply_data_t;

extern orch_cmd_list_t *run_list;
extern orch_cmd_list_t *done_list;
extern orch_cmd_list_t *pend_list;


orch_status_t orch_dispatcher(orch_cmd_t *cmd);
orch_status_t orch_reschedule();
orch_status_t orch_resolve_dependency(orch_cmd_list_t *dst_list, orch_cmd_list_t *src_list);
orch_status_t orch_resolve_self_dependency(orch_cmd_list_t *dst_list);
orch_status_t orch_create_handler_connection(orch_handler_addr_t *handler, orch_handler_connection_t *con);
orch_status_t orch_close_handler_connection(orch_handler_connection_t *con);
orch_status_t orch_dump_all_list();
orch_status_t orch_parse_xml_cmd(char *file, orch_cmd_t *cmd);


#endif //__ORCHESTRATOR_H__

