/*
 * cpc_commands.h
 * This is the command/notify list that is included in both primary and secondary
 *  Created on: Sept 20, 2023
 *      Author: anbiro, orkevlar
 */

#ifndef CPC_COMMANDS_H_
#define CPC_COMMANDS_H_

//primary->secondary commands
#define CPC_COMMAND_LED_ON              '1'
#define CPC_COMMAND_LED_OFF             '0'
//secondary->primary notifications
#define CPC_NOTIFY_BUTTON_PRESSED       'b'
#define CPC_NOTIFY_LED_ON               '1'
#define CPC_NOTIFY_LED_OFF              '0'

/*
 * Handshake only used to detect a primary connected
 * Primary sends it to the secondary when it connects
 */
#define CPC_COMMAND_HANDSHAKE           'h'

#define CPC_COMMAND_LEN 1
#define CPC_NOTIFY_LEN 1

#endif /* CPC_COMMANDS_H_ */
