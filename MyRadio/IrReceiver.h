/*
 * IrReceiver.h
 *
 * Created: 2018-04-19 오후 8:53:39
 *  Author: kiki
 */ 


#ifndef IRRECEIVER_H_
#define IRRECEIVER_H_

#include <avr/io.h>

/*
* Normal Data Lead Length : 13.5ms (210 clock)
* Repeat Data Lead Length : 11.25ms (175 clock)
* Bit 0 Length : 1.125ms (17 clock)
* Bit 1 Length : 2.25ms (35 clock)
*/

#define IR_NORMAL_LEAD_CLK_MIN		205
#define IR_NORMAL_LEAD_CLK_MAX		215
#define IR_REPEAT_LEAD_CLK_MIN		170
#define IR_REPEAT_LEAD_CLK_MAX		180
#define IR_BIT_CLK_DIFF				26
#define IR_BIT_CLK_MAX				44

typedef void (*cbFuncPtr)(uint32_t);
void IR_Init(cbFuncPtr funcPtr);

#endif /* IRRECEIVER_H_ */