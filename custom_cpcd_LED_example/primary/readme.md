# On the Secondary (RCP)
1. Build and flash bootloader (in this case: bootloader-uart-xmodem)
2. Create a new rcp sample app and make the following changes in the software configurator:
    1. turn off cpc security (by installing CPC Security None)
    2. add simple LED > hit 'done' on the prompt
    3. add simple button > hit 'done' on the prompt
    4. Secondary Device (Co-Processor) > Configure > Max Number of User Endpoints should be at least 1
    5. add cpc_button_led.c, cpc_button_led.h and cpc_commands.h      files to the project
    6. replace app.c with the app.c in this example
3. build & flash to the RCP
4. Connect the RCP to the Host