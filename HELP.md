
# Server Setup
========================================
Using Ubuntu 16.04.1 LTS 32-Bit

## Initial Setup:
1. Open a Terminal
2. `cd` to the `Source\Server` directory
3. Run `chmod +x setup.sh`
4. Run `./setup.sh`

The setup script will install all necessary libraries, configure Apache2, compile the server, setup web-based editors, and copy files to correct locations.

## To Compile, after initial Setup:
1. Open a terminal
2. `cd` the Terminal to your `Source\Server` directory
3. run the command `make`

The makefile will compile, and move all necessary files after.

## To Start the Server:
1. Open a Terminal
2. cd the terminal to your server directory
3. There are two ways of running the server
  * To log the server on the terminal:
    * Run the command `./server console`
  * To log the server to the log file:
    * Run the command `./server`

## To Shutdown:
1. The method to shutdown the server depends on how it is started
2. Server logging to the terminal:
  * Press the buttons `CTRL + \` or `CTRL + C` to gracefully shutdown
3. Server logging to the log file:
  * The normal process of shutting down/restarting the system will send the shutdown command
