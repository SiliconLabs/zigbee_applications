# Green Power Gateway example
## 1. Summary
This project was made for the Green Power Gateway KBA. //insert link here to the KBA when posted on the forums
## 2. Gecko SDK version
Gecko SDK Suite 2.7.6
## 3. Hardware Required
Three or more Wireless Starter Kit Main Boards and three or more EFR32MG12 2.4 GHz 19 dBm Radio Boards.
## 4. Connections Required
Connect the radio boards to the WSTK mainboards. Connect your desired gateway device to a computer with Linux operating system (you can also use Linux-like environment in Windows, like [Cygwin](https://www.silabs.com/community/wireless/zigbee-and-thread/knowledge-base.entry.html/2017/04/13/setting_up_cygwinfo-CA2n "Cygwin")).
## 5. Setup
Build and flash the GP_Gateway_NCP application to your board. On your Linux, build the GP_Gateway_Host application. Open a terminal and run the host application.

Build and flash a Green Power On/Off Switch application to one of your boards.

Make some Z3Light devices: build and flash the Z3Light sample application to the rest of your boards.
## 6. How It Works
A detailed explanation can be found here. //link the KBA
## 7. .sls Projects Used
GP_Gateway_NCP.sls
GP_Gateway_Host.sls
## 8. How to Port to Another Part
Open GP_Gateway_NCP's .isc file. On General tab, click „Edit Architecture” and select the desired board.
## 9. Special Notes
N/A
