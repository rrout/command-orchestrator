# command-orchestrator
Simple Linux Command Orchestrator Design to Learn the concept

# How to Build
Go to each directory and do make

[rrout@RASHMI-PC /vol/command-orchestrator/action-handler (master=)]

$make

[rrout@RASHMI-PC /vol/command-orchestrator/orchestrator (master=)]

$make

# How to test
Open 2 Terminal and execute the process

**START EXECUTION SERVER**

[rrout@RASHMI-PC /vol/command-orchestrator/action-handler (master=)]

$./action-handler

**START ORCHISTRATOR**

[rrout@RASHMI-PC /vol/command-orchestrator/orchestrator (master=)]

$./orchestrator

**Note**, "action-handler" is the execution server/micro service where "orchestrator" connects, So you MUST
start "action-handler"  prior to "orchestrator" ("orchestrator" will fail is it cant connect to execution server)

**Display Status**
Open Another Terminal and telnet to "orchestrator" which is running at port 9000

[rrout@RASHMI-PC /vol/command-orchestrator/orchestrator (master=)]

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


