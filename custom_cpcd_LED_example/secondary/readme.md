# On the Primary (Host)
1. Copy 'custom_cpc_host' directory to the host (using scp)
2. ssh to the host
3. cd to the custom_cpc_host directory
4. run: sudo make -f Makefile 
5. start cpcd (/usr/local/bin/cpcd)
6. start the host app ( ./cpcledbutton)

# From the application
1. Press any key to bring up the menu
2. press '1' or '0' on the host to turn on the led on the secondary
3. press 'btn0' on the secondary to toggle the led and notify the host
4. press 'q' to exit the application and close the endpoint