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

#include "action_handler.h"


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

int main(int argc, char **argv)
{
    int i, j, max = 0, sfds[MAX_SERVER] = {0}, afd;
    size_t len;
    fd_set list;
    char buff[MAXBUFF] = {0};
    struct sockaddr_in sock[MAX_SERVER] = {0};
    struct pollfd pfds[MAX_CONN] = {0};
    struct pollfd tmp_pfds[MAX_CONN] = {0};
    int newcon = 0;
    int num_sfds = 0, num_pfds = 0, num_tmp_pfds = 0;
    int nread = 0;
    int poll_res, res;
    int reuse_addr = 1, linger_addr = 1;
    struct linger lin;

    /* initialize our buffer */
    memset(buff, 0, MAXBUFF);

    /*
     * We will loop through each file descriptor. First,
     * we will create a socket bind to it and then call 
     * listen. If we get and error we simply exit, 
     * which is fine for demo code, but not good in the
     * real world where errors should be handled properly. 
     */
    for( i = 0; i < MAX_SERVER; i++ )
    {
        /* check to see that we can create them */
        if( (sfds[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        {
            perror("Cannot create socket");
            exit(1);
        }

        if (reuse_addr) {
            if (setsockopt(sfds[i], SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int)) < 0)
            {
                perror("Cannot set setsockopt");
            }
        }
        if (linger_addr)
        {
            lin.l_onoff = 0;
            lin.l_linger = 0;
            setsockopt(sfds[i], SOL_SOCKET, SO_LINGER, (const char *)&lin, sizeof(int));
        }

        /* now fill out the socket stuctures */
        memset(&sock[i], 0, sizeof(struct sockaddr_in));
        sock[i].sin_family = AF_INET;
        sock[i].sin_port = htons(PORT + i);
        len = INADDR_ANY;
        memset(&sock[i].sin_addr, len, sizeof(struct in_addr));

        /* Now bind to the socket   */
        if( bind(sfds[i], (struct sockaddr *) &sock[i], sizeof(struct sockaddr_in)) < 0 )
        {
            perror("Cannot bind to the socket");
            exit(1);
        }

        /* set our options */
        if( setsockopt(sfds[i], SOL_SOCKET, SO_REUSEADDR, &j, sizeof(int)) < 0 )
        {
            perror("Cannot set socket options \n");
        }

        /* set the socket to the listen state */
        if( listen(sfds[i], 5) < 0 )
        {
            perror("Failed to listen on the socket \n");
        }

        /* now set our pollfd struct */
        pfds[i].fd = sfds[i];
        pfds[i].events = POLLIN ;
        num_pfds++;

        num_sfds++;
    }/* for */

    /*
     * Our main loop. Note, with the poll function we do 
     * not need to modify our structure before we call 
     * poll again. Also note that the overall function
     * is much easier to implement over select.   
     */
    while( 1 )
    {

        /*
         * Now call poll. When poll returns, one of 
         * the three conditions will be true:
         * I)   The timeout has expired
         * II)  The poll call had an error
         * III) We have a socket ready to accept
         */
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
                //break;

            default:   
                /* 
                 * Now we have to loop through each descriptor to
                 * see which is ready to accept. We will know if 
                 * the POLLIN bit is set on this descriptor that this
                 * descriptor is ready to use. 
                 */
                for( i =0; i < num_pfds; i++ )
                {
                    if( pfds[i].revents & POLLIN )
                    {
                        /*
                         * We now have to accept the connection and then
                         * echo back what is written.
                         */
                        newcon = RESULT_FALSE;
                        for( j = 0 ; j < num_sfds ; j++ )
                        {
                            if(pfds[i].fd == sfds[j]) {
                                newcon = RESULT_TRUE;
                            }
                        }
                        if (newcon == RESULT_TRUE) {
                            printf("We have a connection \n");
                            len = sizeof(struct sockaddr_in);
                            afd = accept(sfds[i], (struct sockaddr *)&sock[i], &len);
                            /*
                             * New connection
                             * Keep the fd inside poll to poll for data
                             */
                            pfds[num_pfds].fd = afd;
                            pfds[num_pfds].events = POLLIN ;
                            num_pfds++;
                        } else {
                            printf ("We have data on FD at %d POLL index\n",i);
                            ioctl(pfds[i].fd, FIONREAD, &nread);
                            if( nread == 0 ) {
                                afd = pfds[i].fd;
                                printf ("Closing FD at %d POLL index\n",i);
                                close(afd);
                                pfds[i].events = 0;
                            } else {
                                len = read(pfds[i].fd, buff, MAXBUFF);
                                if(debug_enabled) {
                                    write(pfds[i].fd, buff, len +1);
                                    printf("Echoing back:\n %s \n");
                                } else {
                                    res = action_handler(buff, len);
                                    len = MAXBUFF;
                                    action_handler_reply(res, buff, &len);
                                    write(pfds[i].fd, buff, len+1);
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


