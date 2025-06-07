#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <time.h>

// Get the value for a command line parameter
// For /P (prompt), collects all words until next parameter
// For other parameters, uses standard /X:value format
char* getParam(int argc, char* argv[], const char* paramName)
{
    static int bufferIndex = 0;
    static char buffers[4][256]; // Multiple buffers for different parameters
    
    // Rotate through buffers to avoid overwriting previous values
    char* buffer = buffers[bufferIndex];
    bufferIndex = (bufferIndex + 1) % 4;
    buffer[0] = '\0';
    
    int i;
    
    // Special handling for /P parameter - collect all words until next parameter
    if (stricmp(paramName, "P") == 0) {
        for (i = 1; i < argc; i++) {
            // Check if this argument is our parameter (case-insensitive)
            if (argv[i][0] == '/' && 
                (stricmp(argv[i] + 1, "P") == 0 || 
                 stricmp(argv[i] + 1, "PROMPT") == 0)) {
                                
                // Parameter found, now collect all subsequent arguments until next parameter
                buffer[0] = '\0'; // Start with empty string
                int currentPos = 0;
                
                // Loop through arguments after the /P parameter
                int j;
                for (j = i + 1; j < argc; j++) {
                    // If we hit another parameter, stop collecting
                    if (argv[j][0] == '/') {
                        break;
                    }
                                        
                    // If not the first word, add a space
                    if (currentPos > 0) {
                        buffer[currentPos++] = ' ';
                    }
                    
                    // Copy this word to our buffer
                    strcpy(buffer + currentPos, argv[j]);
                    currentPos += strlen(argv[j]);
                }
                
                // If we collected anything, return it
                if (currentPos > 0) {
                    return buffer;
                }
                                return NULL; // Parameter found but no value
            }
        }
        
        return NULL;
    }
    else {
        // Standard handling for other parameters in /X:value format
        char paramWithColon[32];
        sprintf(paramWithColon, "/%s:", paramName);
        
        for (i = 1; i < argc; i++) {
            // Check if this argument starts with our parameter prefix
            if (strnicmp(argv[i], paramWithColon, strlen(paramWithColon)) == 0) {
                // Extract value after the colon
                strcpy(buffer, argv[i] + strlen(paramWithColon));
                return buffer;
            }
        }
    }
    
    return NULL; // Parameter not found
}

// Get the value for a command line parameter with colon format (/X:value)
char* getColonParam(int argc, char* argv[], const char* paramName)
{
    static int bufferIndex = 0;
    static char buffers[5][256]; // Multiple buffers for different parameters
    
    // Use a specific buffer for each parameter type to avoid conflicts
    char* buffer;
    if (stricmp(paramName, "V") == 0) {
        buffer = buffers[0];
    } else if (stricmp(paramName, "D") == 0) {
        buffer = buffers[1];
    } else if (stricmp(paramName, "T") == 0) {
        buffer = buffers[2];
    } else if (stricmp(paramName, "O") == 0) {
        buffer = buffers[3];
    } else {
        // For any other parameter, use the rotating buffer
        buffer = buffers[4];
        bufferIndex = (bufferIndex + 1) % 4;
    }
    
    buffer[0] = '\0';
    
    char paramWithColon[32];
    sprintf(paramWithColon, "/%s:", paramName);
    
    for (int i = 1; i < argc; i++) {
        if (strnicmp(argv[i], paramWithColon, strlen(paramWithColon)) == 0) {
            strcpy(buffer, argv[i] + strlen(paramWithColon));
            return buffer;
        }
    }
    
    return NULL;
}

// Alternative approach: separate functions for different parameter types
char* getPromptParam(int argc, char* argv[])
{
    static char buffer[256];
    buffer[0] = '\0';
    
    for (int i = 1; i < argc; i++) {
        // Check for /P or /PROMPT (case insensitive)
        if (argv[i][0] == '/' && 
            (stricmp(argv[i] + 1, "P") == 0 || stricmp(argv[i] + 1, "PROMPT") == 0)) {
                        
            // Collect words until next parameter
            buffer[0] = '\0';
            int pos = 0;
            
            for (int j = i + 1; j < argc; j++) {
                if (argv[j][0] == '/') {
                    break;  // Stop at next parameter
                }
                
                if (pos > 0) {
                    buffer[pos++] = ' ';  // Add space between words
                }
                
                strcpy(buffer + pos, argv[j]);
                pos += strlen(argv[j]);
            }
            
            return buffer[0] ? buffer : NULL;
        }
    }
    
    return NULL;  // Not found
}

void showHelp()
{
    printf("SET-VAR - Prompts for input and saves to environment variable\n");
    printf("Syntax: SET-VAR /P prompt text /V:variable [/D:default] [/T:timeout] [/O:filename]\n\n");
    printf("  /P text         Text to display as prompt (all words after /P until next parameter)\n");
    printf("  /V:variable     Environment variable to store the input\n");
    printf("  /D:default      Default value if user enters nothing\n");
    printf("  /T:timeout      Timeout in seconds (1-255) before using default\n");
    printf("  /O:filename     Output batch file name (default: SETVAR.BAT)\n");
    printf("\nExamples:\n");
    printf("  SET-VAR /P Enter your name: /V:USERNAME /D:User /T:10\n");
    printf("  SET-VAR /P What is the server address? /V:SERVER_IP\n");
    printf("\nThen run the batch file to set the variable:\n");
    printf("CALL SETVAR.BAT\n");
    printf("\nNote: You must CALL the batch file to set variables in the parent shell\n");
}

// Main function with the new approach
int main(int argc, char* argv[])
{
    if (argc < 2 || (argc == 2 && strcmp(argv[1], "/?") == 0)) {
        showHelp();
        return 1;
    }

    // Remove debug argument printing
    // Get parameters using the new approach
    char* prompt = getPromptParam(argc, argv);
    char* variable = getColonParam(argc, argv, "V");
    char* defaultVal = getColonParam(argc, argv, "D");
    char* timeoutStr = getColonParam(argc, argv, "T");
    char* outputFile = getColonParam(argc, argv, "O");
    
    // Default output file if not specified
    if (!outputFile) {
        outputFile = "SETVAR.BAT";
    }
    
    if (!prompt || !variable) {
        printf("ERROR: Required parameters missing. Use /? for help.\n");
        return 1;
    }
    
    int timeout = timeoutStr ? atoi(timeoutStr) : 0;
    if (timeout < 0 || timeout > 255) {
        timeout = 0;
    }

    // Remove current value check
    char input[256] = {0};
    
    printf("%s ", prompt);
    if (defaultVal) {
        printf("[%s] ", defaultVal);
    }
    
    if (timeout > 0 && defaultVal) {
        // Handle timeout with default value
        clock_t start = clock();
        int timeLeft = timeout;
        int pos = 0;
        int ch;
        
        while (timeLeft > 0) {
            // Update countdown every second
            int newTimeLeft = timeout - (int)((clock() - start) / CLK_TCK);
            if (newTimeLeft != timeLeft) {
                timeLeft = newTimeLeft;
                printf("\r%s [%s] (%d)  ", prompt, defaultVal, timeLeft);
            }
            
            if (kbhit()) {
                ch = getch();
                if (ch == 13) { // Enter key
                    input[pos] = '\0';
                    printf("\n");
                    break;
                }
                else if (ch == 8 && pos > 0) { // Backspace
                    pos--;
                    printf("\b \b");
                }
                else if (ch >= 32 && ch <= 126 && pos < 255) { // Printable chars
                    input[pos++] = ch;
                    printf("%c", ch);
                }
            }
            
            if (timeLeft <= 0) {
                printf("\n");
                break;
            }
        }
        
        if (pos == 0) { // No input provided
            strcpy(input, defaultVal);
            printf("Timeout - using default value: %s\n", defaultVal);
        }
    }
    else {
        // Regular input without timeout
        gets(input);
        
        // Use default if input is empty and default is provided
        if (input[0] == '\0' && defaultVal) {
            strcpy(input, defaultVal);
        }
    }
    
    // Create the batch file with the SET command
    if (input[0] != '\0') {
        FILE* batFile = fopen(outputFile, "w");
        if (batFile) {
            fprintf(batFile, "SET %s=%s\n", variable, input);            
            fclose(batFile);
            return 0;
        }
        else {
            printf("Error: Could not create batch file '%s'.\n", outputFile);
            return 1;
        }
    }
    
    return 1;
}