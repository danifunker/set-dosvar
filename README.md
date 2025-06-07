# SET-VAR

An MS-DOS program to prompt the user for input and save that as an environment variable.

## How to compile

Using Borland C++ 5.02:

```
BCC -ms set-var.cpp
```

## How to use

This is a two-phase solution due to the way DOS handles environment variables and processes:

1. First, run SET-VAR to create a batch file with the SET command:
   ```
   SET-VAR /P Enter server address: /V:SERVER_IP /D:192.168.0.1 /O:SETSERVER.BAT
   ```

2. Then run the batch file using CALL to set the environment variable:
   ```
   CALL SETSERVER.BAT
   ```

### Parameters

- `/P text` - Text to display as prompt (all words after /P until next parameter)
- `/V:variable` - Environment variable to store the input
- `/D:default` - Default value if user enters nothing
- `/T:timeout` - Timeout in seconds (1-255) before using default
- `/O:filename` - Output batch file name (default: SETVAR.BAT)

### Examples

```
SET-VAR /P Enter your name: /V:USERNAME /D:User /T:10
SET-VAR /P What is the server address? /V:SERVER_IP
```

**Note:** You must CALL the batch file to set variables in the parent shell.