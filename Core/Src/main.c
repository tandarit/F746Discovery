#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "quadspi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"
#include "SDRAM.h"
#include "BME280_STM32.h"
#include "stdio.h"
#include "string.h"

extern SDRAM_HandleTypeDef hsdram1;
extern FMC_SDRAM_TimingTypeDef  SDRAM_Timing;
FMC_SDRAM_CommandTypeDef command;
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern QSPI_HandleTypeDef hqspi;
extern ADC_HandleTypeDef hadc3;
extern TIM_HandleTypeDef htim2;
extern DMA_HandleTypeDef hdma_tim2_ch1;


struct SDRAMResults {
	float Temperature;
	float Pressure;
	float Humidity;
	uint16_t JoyAnalog[2];
	char *text;
};
struct SDRAMResults* sdResults=(uint32_t *)0xC0000000;

#define CCRValue_BufferSize     37	//with 10 degree resulution

uint32_t SineCCRValue_Buffer[CCRValue_BufferSize] =
{
  4999, 5867, 6708, 7498, 8212, 8828, 9328, 9696, 9922,
  9998, 9922, 9696, 9328, 8828, 8212, 7498, 6708, 5867,
  4999, 4130, 3289, 2499, 1785, 1169, 669, 301, 75, 0, 75,
  301, 669, 1169, 1785, 2499, 3289, 4130, 4999
};

uint8_t RxData[256];

int HTC = 0, FTC = 0;
uint32_t indx=0;

int isCommandRxed = 0;
uint32_t size=0;

#if defined(__CC_ARM)
extern uint32_t Load$$QSPI$$Base;
extern uint32_t Load$$QSPI$$Length;
#elif defined(__ICCARM__)
#pragma section =".qspi"
#pragma section =".qspi_init"
#elif defined(__GNUC__)
extern uint32_t _qspi_init_base;
extern uint32_t _qspi_init_length;
#endif

void SystemClock_Config(void);
static void MPU_Config(void);

int main(void)
{
	  //MPU_Config();
	  SCB_EnableICache();
	  SCB_EnableDCache();
	  HAL_Init();
	  SystemClock_Config();
	  MX_GPIO_Init();
	  MX_DMA_Init();
	  MX_QUADSPI_Init();
	  MX_USART1_UART_Init();
	  MX_FMC_Init();
	  SDRAM_Initialization_Sequence(&hsdram1, &command);
	  MX_ADC3_Init();
	  MX_I2C1_Init();
	  MX_TIM2_Init();

	  HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, SineCCRValue_Buffer, CCRValue_BufferSize);

	  BME280_Config(OSRS_2, OSRS_16, OSRS_1, MODE_NORMAL, T_SB_0p5, IIR_16);

	  SDRAM_ClearRAM(&hsdram1);

	  HAL_ADC_Start_DMA(&hadc3, sdResults->JoyAnalog, 2);
	  HAL_UART_Receive_DMA(&huart1, RxData, 256);

	  while(1)
	  {
		  BME280_Measure(&(sdResults->Temperature), &(sdResults->Humidity), &(sdResults->Pressure));
		  if (((size-indx)>0) && ((size-indx)<128))
		    {
		      if (HTC==1)
		      {
		        strcpy((char *)&(sdResults->text)+indx, (char *)RxData+128);  // memcpy (FinalBuf+indx, RxData+128, (size-indx));
		        indx = size;
		        isCommandRxed = 0;
		        HTC = 0;
		        HAL_UART_DMAStop(&huart1);
		        HAL_UART_Receive_DMA(&huart1, RxData, 256);
		      }

		    else if (FTC==1)
		    {
		       strcpy((char *)&(sdResults->text)+indx, (char *)RxData);  // memcpy (FinalBuf+indx, RxData, (size-indx));
		       indx = size;
		       isCommandRxed = 0;
		       FTC = 0;
		       HAL_UART_DMAStop(&huart1);
		       HAL_UART_Receive_DMA(&huart1, RxData, 256);
		    }
		    else if ((indx == size) && ((HTC==1)||(FTC==1)))
		    	  {
		    	isCommandRxed = 0;
		    	HTC = 0;
		    	FTC = 0;
		    	HAL_UART_DMAStop(&huart1);
		    	HAL_UART_Receive_DMA(&huart1, RxData, 256);
		    	  }
		    }
	  }
}
	  
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	if (isCommandRxed == 0)
	{
		size = ((RxData[0]-48)*1000)+((RxData[1]-48)*100)+((RxData[2]-48)*10)+((RxData[3]-48));  // extract the size
		indx = 0;
		memcpy((char *)&(sdResults->text) + indx, RxData+4, 124);  // copy the data into the main buffer/file
		memset(RxData, '\0', 128);  // clear the RxData buffer
		indx += 124;  // update the indx variable
		isCommandRxed = 1;  // set the variable to 1 so that this loop does not enter again
	}
	else
	{
		memcpy((char *)&(sdResults->text) + indx, RxData, 128);
		memset(RxData, '\0', 128);
		indx += 128;
	}
	HTC=1;  // half transfer complete callback was called
	FTC=0;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	  memcpy((char *)&(sdResults->text) + indx, RxData+128, 128);
	  memset(RxData+128, '\0', 128);
	  indx+=128;
	  HTC=0;
	  FTC=1;
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {

}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x90000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0xA0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8KB;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0xC0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8MB;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
  MPU_InitStruct.BaseAddress = 0xA0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8KB;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL2;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
