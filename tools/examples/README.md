# Forth Application Examples

Forth source code examples that run on V4 RTOS.

## Usage

These examples are compiled to bytecode and sent to the runtime on your device.

### Workflow

1. **Flash runtime** to device (once):
   ```bash
   cd bsp/esp32c6/runtime
   idf.py build flash
   ```

2. **Compile** Forth source to bytecode:
   ```bash
   v4c hello.fth -o hello.bin
   ```

3. **Send** bytecode to device:
   ```bash
   v4flash -p /dev/ttyUSB0 hello.bin
   ```

4. **Watch** execution on serial monitor:
   ```bash
   idf.py monitor
   ```

## Examples

### hello.fth - Basic Output

```forth
: HELLO  ." Hello from V4 RTOS!" CR ;
HELLO
```

Demonstrates:
- Forth word definition
- String output
- Program execution

### blink.fth - LED Control

```forth
: BLINK
    BEGIN
        7 GPIO-PIN 1 GPIO-WRITE   \ LED on
        1000 DELAY
        7 GPIO-PIN 0 GPIO-WRITE   \ LED off
        1000 DELAY
    AGAIN ;

BLINK
```

Demonstrates:
- GPIO control via syscalls
- Infinite loops
- Timing/delays

### multitask.fth - Concurrent Tasks

```forth
: TASK1
    BEGIN
        ." Task 1" CR
        1000 DELAY
    AGAIN ;

: TASK2
    BEGIN
        ." Task 2" CR
        2000 DELAY
    AGAIN ;

' TASK1 0 TASK-CREATE DROP
' TASK2 0 TASK-CREATE DROP
```

Demonstrates:
- Creating multiple tasks
- Preemptive multitasking
- Task execution

### message.fth - Inter-task Communication

```forth
VARIABLE TASK2-ID

: SENDER
    BEGIN
        S" Hello" TASK2-ID @ SEND DROP
        1000 DELAY
    AGAIN ;

: RECEIVER
    CREATE BUF 64 ALLOT
    BEGIN
        BUF 64 RECV DROP
        BUF COUNT TYPE CR
    AGAIN ;

' RECEIVER 0 TASK-CREATE TASK2-ID !
' SENDER 0 TASK-CREATE DROP
```

Demonstrates:
- Message passing between tasks
- String handling
- Task synchronization

## Forth Syntax

V4 RTOS uses a subset of ANS Forth:

### Stack Operations

```forth
DUP     ( n -- n n )       Duplicate top
DROP    ( n -- )           Remove top
SWAP    ( a b -- b a )     Swap top two
OVER    ( a b -- a b a )   Copy second
```

### Arithmetic

```forth
+  -  *  /  MOD
```

### Logic

```forth
AND  OR  XOR  NOT
<  >  =  <=  >=
```

### Control Flow

```forth
IF ... THEN
IF ... ELSE ... THEN
BEGIN ... UNTIL
BEGIN ... AGAIN
```

### Memory

```forth
@  !           ( Fetch/store cell )
C@ C!          ( Fetch/store byte )
VARIABLE name  ( Create variable )
CONSTANT name  ( Create constant )
```

### System Calls

See [System Calls Reference](../../docs/api-reference/syscalls.md) for complete list.

```forth
TASK-CREATE    ( xt priority -- task-id )
TASK-DELAY     ( ms -- )
SEND           ( addr len task-id -- result )
RECV           ( addr maxlen -- len )
GPIO-WRITE     ( pin level -- )
```

## Development Tools

### v4c - Compiler

```bash
v4c source.fth -o output.bin
v4c source.fth --disassemble    # View bytecode
```

### v4flash - Uploader

```bash
v4flash -p /dev/ttyUSB0 program.bin
v4flash -p /dev/ttyUSB0 program.bin --verbose
```

### v4repl - Interactive REPL

```bash
v4repl -p /dev/ttyUSB0
```

(Note: Tools are planned, not yet implemented)

## Tips

- Keep programs small (4KB bytecode buffer on device)
- Use tasks for concurrent operations
- Leverage syscalls for hardware access
- Test incrementally with short programs

## See Also

- [V4 RTOS Architecture](../../docs/architecture.md)
- [System Calls Reference](../../docs/api-reference/syscalls.md)
- [Runtime Documentation](../../bsp/esp32c6/runtime/README.md)
