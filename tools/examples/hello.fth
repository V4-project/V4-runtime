\ hello.fth - Basic V4 RTOS Hello World
\
\ This is the simplest V4 RTOS program.
\ It demonstrates basic Forth syntax and string output.
\
\ Compile: v4c hello.fth -o hello.bin
\ Upload:  v4flash -p /dev/ttyUSB0 hello.bin

: HELLO
    ." Hello from V4 RTOS!" CR
    ." This is Forth running on ESP32-C6" CR
;

\ Execute immediately
HELLO
