\ multitask.fth - Multitasking Example
\
\ Demonstrates preemptive multitasking with two concurrent tasks
\
\ Compile: v4c multitask.fth -o multitask.bin
\ Upload:  v4flash -p /dev/ttyUSB0 multitask.bin

: TASK1
    BEGIN
        ." [Task 1] Running..." CR
        1000 DELAY
    AGAIN
;

: TASK2
    BEGIN
        ." [Task 2] Running..." CR
        2000 DELAY
    AGAIN
;

\ Create and start both tasks
.' Creating tasks..." CR
' TASK1 0 TASK-CREATE DROP
' TASK2 0 TASK-CREATE DROP
." Both tasks started!" CR

\ Main task can exit - other tasks continue running
