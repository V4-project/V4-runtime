\ blink.fth - LED Blink Example
\
\ Blinks the LED on GPIO 7 (M5Stack NanoC6)
\
\ Compile: v4c blink.fth -o blink.bin
\ Upload:  v4flash -p /dev/ttyUSB0 blink.bin

7 CONSTANT LED-PIN
1000 CONSTANT BLINK-DELAY

: LED-ON   LED-PIN 1 GPIO-WRITE ;
: LED-OFF  LED-PIN 0 GPIO-WRITE ;

: BLINK
    BEGIN
        ." LED ON" CR
        LED-ON
        BLINK-DELAY DELAY

        ." LED OFF" CR
        LED-OFF
        BLINK-DELAY DELAY
    AGAIN
;

\ Start blinking
BLINK
