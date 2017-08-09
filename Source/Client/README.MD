# openMerc Client

## Setting IP
The IP address for the server must be changed in two places:
* main.c in  the function: WinMain - strcmp(host_addr,"127.0.0.1")
* common.h - #define MHOST  "127.0.0.1"
