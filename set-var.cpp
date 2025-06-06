#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <time.h>

void showHelp()
{
    cout << "INPUT - Prompts for input and saves to environment variable\n";
    cout << "Syntax: INPUT /Q:\"question\" /V:variable [/D:default] [/T:timeout]\n\n";
    cout << "  /Q:\"question\"  Text to display as prompt (use quotes for spaces)\n";
    cout << "  /V:variable     Environment variable to store the input\n";
    cout << "  /D:default      Default value if user enters nothing\n";
    cout << "  /T:timeout      Timeout in seconds (1-255) before using default\n";
    cout << "\nExample: INPUT /Q:\"Enter your name:\" /V:USERNAME /D:User /T:10\n";
}

// Modified getParam function to handle quoted parameters
char* getParam(int argc, char* argv[], const char* param)
{
    int paramLen = strlen(param);
    
    for (int i = 1; i < argc; i++)
    {
        if (strnicmp(argv[i], param, paramLen) == 0)
        {
            char* value = &argv[i][paramLen];
            
            // Handle quotes in parameters
            if (*value == '"')
            {
                // If this parameter starts with a quote, look for the ending quote
                value++; // Skip the opening quote
                
                // If this parameter has the closing quote
                char* endQuote = strchr(value, '"');
                if (endQuote)
                {
                    *endQuote = '\0'; // Null-terminate at the closing quote
                    return value;
                }
                
                // If the closing quote is in another argument (multi-part quoted string)
                static char mergedParam[512];
                strcpy(mergedParam, value);
                int mergedLen = strlen(mergedParam);
                
                // Search next arguments for the closing quote
                for (int j = i + 1; j < argc; j++)
                {
                    mergedParam[mergedLen++] = ' '; // Add space between args
                    
                    char* nextArg = argv[j];
                    endQuote = strchr(nextArg, '"');
                    
                    if (endQuote)
                    {
                        // Found end quote, copy up to that point
                        int copyLen = endQuote - nextArg;
                        strncpy(&mergedParam[mergedLen], nextArg, copyLen);
                        mergedParam[mergedLen + copyLen] = '\0';
                        
                        // Mark these arguments as used so they're not processed again
                        for (int k = i+1; k <= j; k++)
                        {
                            argv[k][0] = '\0';
                        }
                        
                        return mergedParam;
                    }
                    else
                    {
                        // No end quote yet, copy the whole argument
                        strcpy(&mergedParam[mergedLen], nextArg);
                        mergedLen += strlen(nextArg);
                    }
                }
                
                // If we get here, there was no closing quote
                return value;
            }
            
            return value;
        }
    }
    
    return NULL;
}

int main(int argc, char* argv[])
{
    if (argc < 2 || strcmp(argv[1], "/?") == 0)
    {
        showHelp();
        return 1;
    }

    char* question = getParam(argc, argv, "/Q:");
    char* variable = getParam(argc, argv, "/V:");
    char* defaultVal = getParam(argc, argv, "/D:");
    char* timeoutStr = getParam(argc, argv, "/T:");
    
    int timeout = 0;
    if (timeoutStr != NULL)
    {
        timeout = atoi(timeoutStr);
        if (timeout < 0 || timeout > 255)
        {
            timeout = 0;
        }
    }

    if (question == NULL || variable == NULL)
    {
        cout << "ERROR: Required parameters missing. Use /? for help.\n";
        return 1;
    }

    // Check if the environment variable is already set
    char* currentValue = getenv(variable);
    if (currentValue != NULL)
    {
        cout << "Current value of " << variable << " is: " << currentValue << "\n";
    }

    char input[256] = {0};
    
    cout << question << " ";
    if (defaultVal != NULL)
    {
        cout << "[" << defaultVal << "] ";
    }
    
    if ((timeout > 0) && (defaultVal != NULL))
    {
        // Handle timeout with default value
        clock_t start = clock();
        int timeElapsed = 0;
        int pos = 0;
        int ch;
        
        while (timeElapsed < timeout)
        {
            if (kbhit())
            {
                ch = getch();
                if (ch == 13) // Enter key
                {
                    input[pos] = '\0';
                    break;
                }
                else if ((ch == 8) && (pos > 0)) // Backspace
                {
                    pos--;
                    cout << "\b \b";
                }
                else if ((ch >= 32) && (ch <= 126) && (pos < 255)) // Printable chars
                {
                    input[pos++] = ch;
                    cout << (char)ch;
                }
            }
            
            timeElapsed = (clock() - start) / CLK_TCK;
            
            // Show countdown every second
            int currentSeconds = (clock() - start) / CLK_TCK;
            if (timeElapsed != currentSeconds)
            {
                timeElapsed = currentSeconds;
                gotoxy(wherex(), wherey());
                cout << " (" << (timeout - timeElapsed) << ")";
            }
        }
        cout << "\n";
        
        if ((strlen(input) == 0) && (timeElapsed >= timeout))
        {
            strcpy(input, defaultVal);
            cout << "Timeout - using default value: " << defaultVal << "\n";
        }
    }
    else
    {
        // Regular input without timeout
        gets(input);
        
        // Use default if input is empty and default is provided
        if ((strlen(input) == 0) && (defaultVal != NULL))
        {
            strcpy(input, defaultVal);
        }
    }
    
    // Set the environment variable
    if (strlen(input) > 0)
    {
        char envString[512];
        sprintf(envString, "%s=%s", variable, input);
        putenv(envString);
        cout << "Environment variable " << variable << " set to: " << input << "\n";
        return 0;
    }
    else
    {
        cout << "No input provided. Environment variable not set.\n";
        return 1;
    }
}