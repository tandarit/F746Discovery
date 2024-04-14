#ifndef __COM_PROTOCOL_H
#define __COM_PROTOCOL_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

#define READ_FLASH	0x01
#define WRITE_FLASH	0x02
#define READ_SDRAM	0x04
#define WRITE_SDRAM	0x08


typedef enum {PASSED = 0, FAILED = !PASSED} Transmited_Status_t;



#ifdef __cplusplus
}
#endif
#endif
