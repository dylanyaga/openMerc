
Server Setup
========================================
Using Ubuntu 16.04.1 LTS 32-Bit

Pre Compile Setup:

1. Open a Terminal
2. `cd` to the `Source\Server` directory
3. Run `chmod 777 _setup.sh`
4. Run `./_setup.sh`

To Compile:

1. Open a terminal
2. `cd` the Terminal to your `Source\Server` directory
3. run the command `make`

To Start the Server:

1. Open a Terminal
2. cd the terminal to your server directory
3. There are two ways of running the server
  3a. To log the server on the terminal:
      Run the command `./server console`
  3b. To log the server to the log file: 
      Run the command `./server`
      
To Shutdown:

1. The method to shutdown the server depends on how it is started
2. Server logging to the terminal:
  2a. Press the buttons `CTRL + \` or `CTRL + C` to gracefully shutdown
3. Server logging to the log file:
  3a. The normal process of shutting down/restarting the system will send the shutdown command