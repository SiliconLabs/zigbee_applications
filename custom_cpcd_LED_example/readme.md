# Custom CPCd commands - Simple Blinky App
The purpose of this example application is to demonstrate how to create custom cpcd commands. The application allows the user to type either '1' or '0' from the host to turn on or off the LED on the RCP. Similarily, button 0 on the RCP will toggle the LED on the RCP. Pressing 'q' will terminate the application.

This sample application works by first initializing and connecting to cpcd via the API call: cpc_init. It then opens a user endpoint using the API call: cpc_open_endpoint. 

Commands that wish to write to the secondary (RCP) from the primary (Host) will use: cpc_write_endpoint

When writing, the maximum buffer size is determined by the secondary and writing buffers larger than this limit will fail. This value can be queried with cpc_get_endpoint_max_write_size.

Commands that involve reading data from the RCP on the Host will use: cpc_read_endpoint

When reading, the buffer must be at least SL_CPC_READ_MINIMUM_SIZE bytes. 

Both APIs will return a negative value in case of error, or the number of bytes read/written.


## Usage

### On the Secondary (RCP)
1. Build and flash bootloader (in this case: bootloader-uart-xmodem)
2. Create a new rcp sample app and make the following changes in the software configurator:
    i. turn off cpc security (by installing CPC Security None)
    ii. add simple LED > hit 'done' on the prompt
    iii. add simple button > hit 'done' on the prompt
    iv. Secondary Device (Co-Processor) > Configure > Max Number of User Endpoints should be at least 1
    v. add cpc_button_led.c, cpc_button_led.h and cpc_commands.h files to the project
    vi. replace app.c with the app.c in this example
3. build & flash to the RCP
4. Connect the RCP to the Host

### On the Primary (Host)
1. Copy 'custom_cpc_host' directory to the host (using scp)
2. ssh to the host
3. cd to the custom_cpc_host directory
4. run: sudo make -f Makefile 
5. start cpcd (/usr/local/bin/cpcd)
6. start the host app ( ./cpcledbutton)

### From the application
1. Press any key to bring up the menu
2. press '1' or '0' on the host to turn on the led on the secondary
3. press 'btn0' on the secondary to toggle the led and notify the host
4. press 'q' to exit the application and close the endpoint