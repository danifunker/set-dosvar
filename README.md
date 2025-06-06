An MSDOS program to prompt the user for input and set that as an environment variable. 

How to compile:

Using Borland C++ 5.02:

`BCC -ms set-var.cpp`

How to use:

INPUT /Q:"Enter your full name:" /V:USERNAME /D:User /T:10

Full list of switches:

    /Q:"question"  Text to display as prompt (use quotes for spaces)
    /V:variable     Environment variable to store the input
    /D:default      Default value if user enters nothing
    /T:timeout      Timeout in seconds (1-255) before using default

/D and /T are not required