#include "MT25QLXXX.h"
#include "main.h"


void MT25QL_QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand;
  QSPI_AutoPollingTypeDef sConfig;


  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = WRITE_ENABLE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }


  sConfig.Match           = 0x02;	//Write enable latch bit position in the status register
  sConfig.Mask            = 0x02;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  sCommand.Instruction    = READ_STATUS_REG_CMD;
  sCommand.DataMode       = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
}

void MT25QL_QSPI_Erase(QSPI_HandleTypeDef *hqspi) {
	QSPI_CommandTypeDef     sCommand;

	sCommand.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize       	= QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode           	= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction 		= SECTOR_ERASE_CMD;
	sCommand.AddressMode 		= QSPI_ADDRESS_1_LINE;
	sCommand.Address     		= 0;
	sCommand.DataMode    		= QSPI_DATA_NONE;
	sCommand.DummyCycles 		= 0;

	if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
	    Error_Handler();
	}
	MT25QL_QSPI_AutoPollingMemReady(hqspi);
}

void MT25QL_QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand;
  QSPI_AutoPollingTypeDef sConfig;

  /* Configure automatic polling mode to wait for memory ready ------ */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = READ_STATUS_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;

  sConfig.Match           = 0x00;
  sConfig.Mask            = 0x01;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
}

void MT25QL_QSPI_Write(QSPI_HandleTypeDef *hqspi, uint8_t *txBuffer, uint32_t address, uint32_t size) {
	QSPI_CommandTypeDef sCommand;

	MT25QL_QSPI_WriteEnable(hqspi);

	/* Writing Sequence ------------------------------------------------ */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction = QUAD_IN_FAST_PROG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
	sCommand.DataMode    = QSPI_DATA_4_LINES;
	sCommand.Address	=	address;
	sCommand.NbData      = size;

	if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
	    Error_Handler();
	}

	if (HAL_QSPI_Transmit(hqspi, txBuffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
	    Error_Handler();
	}
	MT25QL_QSPI_AutoPollingMemReady(hqspi);
}

void MT25QL_QSPI_Read(QSPI_HandleTypeDef *hqspi, uint8_t *rxBuffer, uint32_t address, uint32_t size) {
	QSPI_CommandTypeDef sCommand;

	MT25QL_QSPI_DummyCyclesCfg(hqspi);

	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction       = QUAD_OUT_FAST_READ_CMD;
	sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
	sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
	sCommand.Address           = address;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DummyCycles       = DUMMY_CLOCK_CYCLES_READ_QUAD;
	sCommand.DataMode          = QSPI_DATA_4_LINES;
	sCommand.NbData            = size;
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
	    Error_Handler();
	}
	if (HAL_QSPI_Receive(hqspi, rxBuffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
	    Error_Handler();
	}
}

void MT25QL_QSPI_ReadID(QSPI_HandleTypeDef *hqspi, uint8_t *rxBuffer) {
	  QSPI_CommandTypeDef     sCommand;

	  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	  sCommand.Instruction       = MULTIPLE_IO_READ_ID_CMD;//READ_ID_CMD;
	  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
	  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	  sCommand.DummyCycles       = 0;
	  sCommand.DataMode          = QSPI_DATA_1_LINE;
	  sCommand.NbData            = 20;
	  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
	  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	  if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  if (HAL_QSPI_Receive(hqspi, rxBuffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	  {
	      Error_Handler();
	  }
}

void MT25QL_QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef sCommand;
  uint8_t reg;

  /* Read Volatile Configuration register --------------------------- */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = READ_VOL_CFG_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  sCommand.NbData            = 1;

  if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_QSPI_Receive(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  /* Enable write operations ---------------------------------------- */
  MT25QL_QSPI_WriteEnable(hqspi);

  /* Write Volatile Configuration register (with new dummy cycles) -- */

  sCommand.Instruction = WRITE_VOL_CFG_REG_CMD;
  MODIFY_REG(reg, 0xF0, (DUMMY_CLOCK_CYCLES_READ_QUAD << POSITION_VAL(0xF0)));

  if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_QSPI_Transmit(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
}
