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
#include <signal.h>

#include "orch_cli.h"


/* our defines */
#define PORT             (8888)
#define MAXBUFF          (1024)
#define MAX_SERVER       (16)
#define MAX_CONN         (1024)
#define TIMEOUT          (1024 * 1024)
#define MY_MAX(a,b)      (a = (a > b) ? a : b )
#define POLL_ERR         (-1)
#define POLL_EXPIRE      (0)
#define RESULT_FALSE     (0)
#define RESULT_TRUE      (1)

int debug_enabled = 0;

int orch_create_server(orch_handler_addr_t *server)
{
    int i, j, max = 0, sfds, afd;
    size_t len;
    fd_set list;
    char buff[MAXBUFF] = {0};
    struct sockaddr_in sock = {0};
    struct pollfd pfds[MAX_CONN] = {0};
    struct pollfd tmp_pfds[MAX_CONN] = {0};
    int newcon = 0;
    int num_sfds = 0, num_pfds = 0, num_tmp_pfds = 0;
    int nread = 0;
    int poll_res, res;
    int reuse_addr = 1, linger_addr = 1;
    struct linger lin;
    char *prompt = "ORCH>";

    /* initialize our buffer */
    memset(buff, 0, MAXBUFF);

    DEBUG_PRINT("Creating Server on port %d \n",server->port);

    if( (sfds = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("Cannot create socket");
        exit(1);
    }

    if (reuse_addr) {
        if (setsockopt(sfds, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int)) < 0)
        {
            perror("Cannot set setsockopt");
        }
    }
    if (linger_addr)
    {
        lin.l_onoff = 0;
        lin.l_linger = 0;
        setsockopt(sfds, SOL_SOCKET, SO_LINGER, (const char *)&lin, sizeof(int));
    }

    /* now fill out the socket stuctures */
    memset(&sock, 0, sizeof(struct sockaddr_in));
    sock.sin_family = AF_INET;
    sock.sin_port = htons(server->port);
    len = INADDR_ANY;
    memset(&sock.sin_addr, len, sizeof(struct in_addr));

    /* Now bind to the socket   */
    if( bind(sfds, (struct sockaddr *) &sock, sizeof(struct sockaddr_in)) < 0 )
    {
        perror("Cannot bind to the socket");
        exit(1);
    }

    /* set our options */
    if( setsockopt(sfds, SOL_SOCKET, SO_REUSEADDR, &j, sizeof(int)) < 0 )
    {
        perror("Cannot set socket options \n");
    }

    /* set the socket to the listen state */
    if( listen(sfds, 5) < 0 )
    {
        perror("Failed to listen on the socket \n");
    }

    /* now set our pollfd struct */
    pfds[i].fd = sfds;
    pfds[i].events = POLLIN ;
    num_pfds++;

    while( 1 )
    {

        if (debug_enabled) {
            printf("POLLING: numSfd=%d, numPfds=%d\n", num_sfds, num_pfds);
        }
        poll_res = poll(pfds, (unsigned int)num_pfds, TIMEOUT);
        switch(poll_res)
        {
            case POLL_EXPIRE:
                printf("Timeout has expired !\n");
                break;                                                    

            case POLL_ERR:
                perror("Error on poll");

            default:   
                for( i =0; i < num_pfds; i++ )
                {
                    if( pfds[i].revents & POLLHUP )
                    {
                        afd = pfds[i].fd;
                        pfds[i].events = 0;
                        printf ("Closing FD at %d POLL index(POLLHUP)\n",i);
                        close(afd);
                    } else if( pfds[i].revents & POLLIN ) {
                        /*
                         * We now have to accept the connection and then
                         * echo back what is written.
                         */
                        newcon = RESULT_FALSE;
                        if(pfds[i].fd == sfds) {
                            newcon = RESULT_TRUE;
                        }
                        if (newcon == RESULT_TRUE) {
                            printf("We have a connection \n");
                            len = sizeof(struct sockaddr_in);
                            afd = accept(sfds, (struct sockaddr *)&sock, &len);
                            /*
                             * New connection
                             * Keep the fd inside poll to poll for data
                             */
                            pfds[num_pfds].fd = afd;
                            pfds[num_pfds].events = POLLIN ;
                            num_pfds++;
                            /* Write Prompt */
                            strcpy(buff, prompt);
                            len = sizeof(prompt);
                            //write(afd, buff, len+1);
                        } else {
                            printf ("We have data on FD at %d POLL index\n",i);
                            ioctl(pfds[i].fd, FIONREAD, &nread);
                            if( nread == 0 ) {
                                afd = pfds[i].fd;
                                printf ("Closing FD at %d POLL index\n",i);
                                len = read(afd, buff, MAXBUFF);
                                close(afd);
                                pfds[i].events = 0;
                            } else {
                                len = read(pfds[i].fd, buff, MAXBUFF);
                                if(debug_enabled) {
                                    write(pfds[i].fd, buff, len +1);
                                    printf("Echoing back:\n %s \n");
                                } else {
                                    res = orch_cli_handler(pfds[i].fd, buff, len);
                                    //len = MAXBUFF;
                                    //orch_cli_handler_reply(res, buff, &len);
                                    write(pfds[i].fd, buff, len+1);
                                    strcpy(buff, prompt);
                                    len = sizeof(prompt);
                                    write(afd, buff, len);
                                }
                            }
                        }
                    }

                } /* for */

        } /* switch */
        /*
         * It's time for managing the pfds to remove the closed fds
         */
        if(debug_enabled)
            printf("PFD Mgmt(S): numPfd=%d\n", num_pfds);
        num_tmp_pfds = 0;
        for( i = 0 ; i < num_pfds ; i++ )
        {
            if(pfds[i].events != 0) {
                tmp_pfds[num_tmp_pfds++] = pfds[i];
            }
        }
        for( i = 0 ; i < num_tmp_pfds ; i++ )
        {
            pfds[i] = tmp_pfds[i];
        }
        num_pfds = i;
        if(debug_enabled)
            printf("PFD Mgmt(E): numPfd=%d\n", num_pfds);
    }/* while(1) */

    /* FIN */
    return(0);

} /* main */


