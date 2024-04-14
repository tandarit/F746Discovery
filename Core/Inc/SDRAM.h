#ifndef __SDRAM_H
#define __SDRAM_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include "fmc.h"

typedef enum {PASSED = 0, FAILED = !PASSED} TestStatus_t;

 #define SDRAM_BANK_ADDR                 ((uint32_t)0xC0000000)

 /* #define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_8  */
 #define SDRAM_MEMORY_WIDTH               FMC_SDRAM_MEM_BUS_WIDTH_16

 #define SDCLOCK_PERIOD                   FMC_SDRAM_CLOCK_PERIOD_2
 /* #define SDCLOCK_PERIOD                FMC_SDRAM_CLOCK_PERIOD_3 */

 #define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)

 #define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
 #define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
 #define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
 #define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
 #define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
 #define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
 #define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
 #define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
 #define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
 #define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
 #define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

 #define SDRAM_Write8(address, value)       (*(__IO uint8_t *) (SDRAM_BANK_ADDR + (address)) = (value))
 #define SDRAM_Read8(address)               (*(__IO uint8_t *) (SDRAM_BANK_ADDR + (address)))
 #define SDRAM_Write16(address, value)      (*(__IO uint16_t *) (SDRAM_BANK_ADDR + (address)) = (value))
 #define SDRAM_Read16(address)              (*(__IO uint16_t *) (SDRAM_BANK_ADDR + (address)))
 #define SDRAM_Write32(address, value)      (*(__IO uint32_t *) (SDRAM_BANK_ADDR + (address)) = (value))
 #define SDRAM_Read32(address)              (*(__IO uint32_t *) (SDRAM_BANK_ADDR + (address)))
 #define SDRAM_WriteFloat(address, value)   (*(__IO float *) (SDRAM_BANK_ADDR + (address)) = (value))
 #define SDRAM_ReadFloat(address)           (*(__IO float *) (SDRAM_BANK_ADDR + (address)))

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command);
void SDRAM_ClearRAM(SDRAM_HandleTypeDef *hsdram);

#ifdef __cplusplus
}
#endif
#endif
