# command-orchestrator
Simple Linux Command Orchestrator Design to Learn the concept

# A Simple Command Orchestrator
It is distributed architecture orchestrating multiple commands with dependency on each other to be executed by its dependency rule.
The orchestrator will accept multiple dependent/independent command from an external application and convert all of them as an ordered work flow depending on the command dependency. Then the entire work flow will be executed in external work flow execution server. The orchestrator will chose a work flow execution server (micro service) and request the work flow execution in the execution server.

![Orchestrator](/docs/orch.png)


Here each Application will orchestrate set of commands from application and create a ordered work flow depending on command dependencies. Then send those commands to the work flow execution server one by one. Each command execution status will be responded back by the work flow engiene

# Design

# Cli Design
With this design we will first create a Cli application which will send multiple commands to the orchestrator. These commands will be sent in asynchronous way. i.e. Cli can send commands as and when  it wants. Once all commands are sent from Cli Application can query for status and display it with this small design. However it can be further enhanced to aggregate the result and provide the same to Order Service Application, i.e there can be an order service application entire cli application to execute a order.
## Low Level Design
```
1.	Create a connection socket with Orchestrator
2.	Wait for user/app input
3.	Convert command input in to Orchestrator understandable format
4.	Send the command to Orchestrator wait for response on successful dispatch of command
5.	A show command will be send which will display Orchestrator response on work flow status
```
# Orchestrator Design
Orchestrator will be central unit. It will accept the commands from the Cli session. It will look if the command is a simple Orchestrator known command OR a request to parse XML files containing command.  In case of simple command it will build “command_data” using inbuilt API and in case of XML it will parse the corresponding XML file to build “command_data” . 
The “command_data” will be then checked against dependency with other in progress “command_data”.  When the “command_data” become independent (free from its dependency on other command), it will be delivered to “Execution Server” for execution. Orchestrator will wait for each execution status and record them
Each “command_data” will be dispatched with a separate thread context and the thread will be responsible for dispatching the “command_data”, waiting for response and recording the response
## Low Level Design
1.	*Initialize Orchestrator*
```
a.	Create 3 queues “RUN-QUEUE”,  “PENDING- QUEUE” and “COMPLETE- QUEUE”
b.	Each of these QUEUE are a linked-list to store “command_data”
c.	“RUN-QUEUE” will store the command which are dispatched and waiting for response
d.	“PENDING- QUEUE” will store commands which are not dispatch ablecurrently
e.	“COMPLETE- QUEUE” will store  commands which are completed by  “execution server”
```
2.	*Initialize a Worker Thread for serving “PENDING- QUEUE”*
```
a.	Whenever “command_data”  in the “PENDING- QUEUE”  become ready the Worker Thread will wake up and server the “command_data”  from “PENDING- QUEUE”  
b.	Once wake up Worker Thread will go through all the items in the “PENDING- QUEUE”  schedule those for dispatch
c.	It will create dispatch thread for all runnable “command_data”  and remove those from “PENDING- QUEUE”  which will be put into “RUN-QUEUE”,  by dispatch thread
```
3.	*Create connection to the “Execution Server”*
```
a.	It has ability to simultaneously connect to multiple “Execution Server”, however its under development
```
4.	*Created a Cli Server  Thread*
```
a.	This thread is responsible for serving multiple Cli sessions
b.	This is a server instance which will wait for Cli client connection and serve them
c.	This is the thread where Cli connects to and do all transaction
d.	It accepts the command and parses the command (simple/XML) and build “command_data”  
e.	Once “command_data” is made it dispatches it Dispatch Thread
5.	Dispatch Thread (Dynamic Threads)
a.	Dispatch Thread are the Dynamic Threads which carry out major responsibility and multiple such threads run concurrently
```
```
i.	Each thread handle one single “command_data”
ii.	It checks the execution eligibility of the “command_data” 
iii.	Eligibility computing is done by comparing current “command_data” dependency with all “command_data” , which is there in “PENDING- QUEUE”  
iv.	If a “command_data” RUNNABLE, it is put in to “RUN-QUEUE”,  and immediately sent to “Execution Server” for execution of command
v.	If  “command_data” is not  RUNNABLE, it is put in to “PENDING- QUEUE”  
vi.	It sends the command to “Execution Server” and wait for status
vii.	Once it receives the response from “Execution Server” the “command_data” is removed from “RUN-QUEUE” and put into “COMPLETE- QUEUE”
viii.	Upon completion of “command_data” execution the thread signals the Worker Thread to do Eligibility computing of items in “PENDING- QUEUE”
```
6.	*Simple Command Parser*
```
a.	It is a API which parses the Orchestrator known command cli commands and generate “command_data” for orchestration
```
7.	*XML parser*
```
a.	It’s a “libxml2” based API parses the XML file and generate “command_data” for orchestration
```
8.	*QUEUE Management*
```
a.	Each queue will have lock
b.	Eligibility computing for “command_data” is done by taking those locks
c.	Multiple locks can be taken
d.	Each “command_data” will be distributed across “RUN-QUEUE”,  “PENDING- QUEUE” and “COMPLETE- QUEUE” based in their eligibility
e.	“RUN-QUEUE” should only hold “command_data” which are in progress and already sent to “Execution Server”
f.	“PENDING- QUEUE”  should only hold “command_data” which are not RUNNABLE, i.e it has dependency on currently running  “command_data” which are on “RUN-QUEUE”,  
g.	“COMPLETE- QUEUE” should only hold “command_data” which are executed successfully by “Execution Server”
h.	“COMPLETE- QUEUE” is solely for display and work flow status calculation
i.	All queue should be made per session, However it is under development now
```

# Execution Server Design
This is the service create multiple Execution Server as a micro service. Each server just accepts the “command_data” from Orchestrator, execute them and send the execution status to Orchestrator.
## Low Level Design
```
1.	Create multiple server running on different ports
2.	Each server waits for a connection from Orchestrator
3.	Upon Orchestrator connection it creates a session and wait in the client socket for accepting command and executing it
```

# Build Procedure
Clone the repository

Go to each module and do make
# How To Test
Run all the application in separate terminal
```
**START EXECUTION SERVER**
$ ./ action-handler
**START ORCHISTRATOR**
$ ./ orchestrator
**START CLI**
$ ./cli
```

```
$ ./cli
CLI> help
Command          Description
---------------- -----------------------------------
help             Command Help
quit             Quit Cli Session
showr            Show Running List
showp            Show Pending List
showc            Show Completed List
show             Show Progress
cleanup          Cleanup Completed List
exec0            Execute Command 0 ()
exec1            Execute Command 1 ()
exec2            Execute Command 2 ()
exec3            Execute Command 3 ()
exec4            Execute Command 4 ()
exec5            Execute Command 5 ()
CLI> exec4
Dispatched Cmd: exec1
exec1
CLI> exec5
Dispatched Cmd: exec4
exec4
CLI> exec3
Dispatched Cmd: exec5
exec3
CLI> exec3
Dispatched Cmd: exec3
exec3
CLI> show
=================STATUS===========
RUNNING:
CMD [ 2   | pwd        | Present Working Directory      ]
CMD [ 1   | ls         | List                           ]
CMD [ 3   | date       | Print Date                     ]
PENDING:
CMD [ 4   | ps         | Process Status                 ]
COMPLETED:
CMD [ 0   | ls -altr   | List Time Ascending            ]
=================STATUS END===========
show
CLI> show
=================STATUS===========
RUNNING:
CMD [ 2   | pwd        | Present Working Directory      ]
CMD [ 1   | ls         | List                           ]
CMD [ 3   | date       | Print Date                     ]
PENDING:
CMD [ 4   | ps         | Process Status                 ]
COMPLETED:
CMD [ 0   | ls -altr   | List Time Ascending            ]
=================STATUS END===========
show
CLI> show
=================STATUS===========
RUNNING:
CMD [ 4   | ps         | Process Status                 ]
CMD [ 2   | pwd        | Present Working Directory      ]
CMD [ 1   | ls         | List                           ]
PENDING:
COMPLETED:
CMD [ 3   | date       | Print Date                     ]
CMD [ 0   | ls -altr   | List Time Ascending            ]
=================STATUS END===========
Show
CLI> show
=================STATUS===========
RUNNING:
CMD [ 3   | date       | Print Date                     ]
CMD [ 0   | ls -altr   | List Time Ascending            ]
CMD [ 4   | ps         | Process Status                 ]
PENDING:
COMPLETED:
CMD [ 1   | ls         | List                           ]
CMD [ 4   | ps         | Process Status                 ]
CMD [ 2   | pwd        | Present Working Directory      ]
CMD [ 1   | ls         | List                           ]
CMD [ 3   | date       | Print Date                     ]
CMD [ 0   | ls -altr   | List Time Ascending            ]
=================STATUS END===========
Show
CLI> show
=================STATUS===========
RUNNING:
CMD [ 3   | date       | Print Date                     ]
PENDING:
COMPLETED:
CMD [ 0   | ls -altr   | List Time Ascending            ]
CMD [ 4   | ps         | Process Status                 ]
CMD [ 1   | ls         | List                           ]
CMD [ 4   | ps         | Process Status                 ]
CMD [ 2   | pwd        | Present Working Directory      ]
CMD [ 1   | ls         | List                           ]
CMD [ 3   | date       | Print Date                     ]
CMD [ 0   | ls -altr   | List Time Ascending            ]
=================STATUS END===========
show
CLI> show
=================STATUS===========
RUNNING:
PENDING:
COMPLETED:
CMD [ 3   | date       | Print Date                     ]
CMD [ 0   | ls -altr   | List Time Ascending            ]
CMD [ 4   | ps         | Process Status                 ]
CMD [ 1   | ls         | List                           ]
CMD [ 4   | ps         | Process Status                 ]
CMD [ 2   | pwd        | Present Working Directory      ]
CMD [ 1   | ls         | List                           ]
CMD [ 3   | date       | Print Date                     ]
CMD [ 0   | ls -altr   | List Time Ascending            ]
=================STATUS END===========
Show
```

# Caveats
```
1. Currentlt the project support only Simple command parsing
2. XML parsing is not supported yet as I an facing some challenges in libxml2 (no time to work on it)
3. Per session work flow handling is under development
4. There are some minor bugs which will be fixed soon
5. Multiple "Execution Server" connection is not yet supported
```

# Known Bugs
```
1. CLI re-connection is not working, if you kill *cli*, you need to restart *orchestrator* for cli to work again
2. There are some junk strings comming in telnet session output
3. Multiple debugs are enabled as this project is under development
```

# Note
```
1. orchestrator runs on localhost:9000 (127.0.0.1:9000)
2. Multiple "Execution Server" runs on localhost:8888-8895 (127.0.0.1:8888-8895)
3. Both "orchestrator: and "Execution Server" supports telnet connection (telnet localhost:9000)
4. "action-handler" is the execution server/micro service where "orchestrator" connects, So you MUST
5. start "action-handler"  prior to "orchestrator" ("orchestrator" will fail is it cant connect to execution server)
```
**Telnet based interface**
The project suppoet Telnet based interface as well

Open Another Terminal and telnet to "orchestrator" which is running at port 9000
```
$ telnet localhost 9000
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
ORCH>help
=================CMD HELP===========
help          :Help on Commands
show          :Show Current Status
show_p        :Show Pending Status
show_r        :Show Running Status
show_c        :Show Completed Status
help
�ORCH>show
=================STATUS===========
RUNNING:
CMD [ 1 | ls | List ]
CMD [ 3 | date | Print Date ]
CMD [ 0 | ls -altr | List Time Ascending ]
PENDING:
CMD [ 4 | ps | Process Status ]
CMD [ 2 | pwd | Present Working Directory ]
COMPLETED:
=================STATUS END===========
```
# Contact
**Author** *Rashmi Ranjan Rout*

**Email**  *rashmiranjanrout3@gmail.com*

**Mobile** *+91-9019915715*

