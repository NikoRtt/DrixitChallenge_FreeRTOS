/*****************************************************************************
* Principal program with the creation of task and the start of the scheduler
*
* Autor: Nicolas Rasitt
* Created: 03/10/22
*
*****************************************************************************/

/*==================[inclusions]=============================================*/

#include "main.h"
#include "cmsis_os.h"
#include "../MyLibs/MyFunctions/MyFunctions.h"
#include "../MyLibs/LIS3MDL/LIS3MDL.h"
#include "../MyLibs/W25Q80DV/W25Q80DV.h"

/*==================[definitions]============================================*/

#define INIT_DATA_ADDRESS (W25Q80DV_FIRST_PAGE_ADDRESS + W25Q80DV_INITIALIZE_SIZE)
#define INIT_DATA_SIZE  2

/*==================[global variables]======================================*/

I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

osThreadId sensorTaskHandle;
osThreadId memoryTaskHandle;
osThreadId receiveTaskHandle;
osThreadId sendTaskHandle;

osSemaphoreId binarySemaphoreUARTHandle;

xQueueHandle queueDataProcessing, queueUsartReception, queueUsartSender;

uint8_t Rx_data[UART_MAX_RECEIVE_DATA];
LIS3MDL_Data_t LIS3MDL_data;
LIS3MDL_StoreData_t LIS3MDL_storedata;
W25Q80DV_Data_t W25Q80DV_data;

/*==================[internal functions definition]==========================*/

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
void sensorFunction(void const * argument);
void recordingFunction(void const * argument);
void receptionFunction(void const * argument);
void sendingFunction(void const * argument);

/*==================[main function]==========================================*/

int main(void){

	// MCU Configuration
	// Reset of all peripherals, Initializes the Flash interface and the Systick.
	HAL_Init();

	// Configure the system clock
	SystemClock_Config();

	// Initialize all configured peripherals
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_I2C1_Init();
	MX_SPI1_Init();
	MX_USART1_UART_Init();

	// First we initialize the memory to recover all the data save(last uid);
	if(w25q80dv_Init(&W25Q80DV_data, &hspi1, SPI1_NCS_Pin, SPI1_NCS_GPIO_Port)){
		// If there's no data save in the memory, we must do an erase to start using it.
		if(w25q80dv_isMemInit(&W25Q80DV_data) == FALSE){
			
			w25q80dv_EraseChip(&W25Q80DV_data);
			
			uint32_t initData = W25Q80DV_INITIALIZE_MEM;
			
			w25q80dv_WriteBytesInAddress(&W25Q80DV_data, W25Q80DV_FIRST_PAGE_ADDRESS, (uint8_t*)&initData, W25Q80DV_INITIALIZE_SIZE);
			
			LIS3MDL_data.uid = 0;
		}
		// Else we have to read the last ID save in memory to know where to start writting the memory.
		else {
			
			uint16_t lastID;
			
			w25q80dv_ReadBytesInAddress(&W25Q80DV_data, INIT_DATA_ADDRESS, (uint8_t*)&lastID, INIT_DATA_SIZE);
			
			W25Q80DV_data.lastAddress = lastID*(sizeof(LIS3MDL_StoreData_t) - 1) + (INIT_DATA_ADDRESS + INIT_DATA_SIZE);
			
			LIS3MDL_data.uid = lastID;
		}
	}

	/* Create the semaphores(s) */
	/* definition and creation of binarySemaphoreUART */
	osSemaphoreDef(binarySemaphoreUART);
	binarySemaphoreUARTHandle = osSemaphoreCreate(osSemaphore(binarySemaphoreUART), 1);
	
	/* Create the queue(s) */
	queueDataProcessing = xQueueCreate(16, sizeof(LIS3MDL_StoreData_t));
	queueUsartReception = xQueueCreate(16, sizeof(uint16_t));
	queueUsartSender = xQueueCreate(16, sizeof(LIS3MDL_StoreData_t));
	
	/* Create the thread(s) */
	osThreadDef(sensorTask, sensorFunction, osPriorityNormal, 0, 128);
	sensorTaskHandle = osThreadCreate(osThread(sensorTask), NULL);
	osThreadDef(receiveTask, receptionFunction, osPriorityNormal, 0, 128);
	receiveTaskHandle = osThreadCreate(osThread(receiveTask), NULL);
	osThreadDef(sendTask, sendingFunction, osPriorityNormal, 0, 128);
	sendTaskHandle = osThreadCreate(osThread(sendTask), NULL);
	osThreadDef(memoryTask, recordingFunction, osPriorityNormal, 0, 128);
	memoryTaskHandle = osThreadCreate(osThread(memoryTask), NULL);

	PrintString(huart1, "Starting FreeRTOS System\r\n", sizeof("Starting FreeRTOS System\r\n"));

	/* Start scheduler */
	osKernelStart();
	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	while (1){}
}

/*==================[internal functions definition]==========================*/

//*************************************************************
//* @brief System Clock Configuration
//* @retval None
//*************************************************************
void SystemClock_Config(void){
  
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
								|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK){
		Error_Handler();
	}
}

//*************************************************************
//* @brief I2C1 Initialization Function
//* @param None
//* @retval None
//*************************************************************
static void MX_I2C1_Init(void){

	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK){
		Error_Handler();
	}
}

//*************************************************************
//* @brief SPI1 Initialization Function
//* @param None
//* @retval None
//*************************************************************
static void MX_SPI1_Init(void){

	/* SPI1 parameter configuration*/
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK){
		Error_Handler();
	}
}

//*************************************************************
//* @brief USART1 Initialization Function
//* @param None
//* @retval None
//*************************************************************
static void MX_USART1_UART_Init(void){

	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK){
		Error_Handler();
	}
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, Rx_data, UART_MAX_RECEIVE_DATA);
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
}

//*************************************************************
//* Enable DMA controller clock
//*************************************************************
static void MX_DMA_Init(void){

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Channel5_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}

//*************************************************************
//* @brief GPIO Initialization Function
//* @param None
//* @retval None
//*************************************************************
static void MX_GPIO_Init(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(SPI1_NCS_GPIO_Port, SPI1_NCS_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin : LED_Pin */
	GPIO_InitStruct.Pin = LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : SPI1_NCS_Pin */
	GPIO_InitStruct.Pin = SPI1_NCS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(SPI1_NCS_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : BUTTON_Pin */
	GPIO_InitStruct.Pin = BUTTON_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

//*************************************************************
//*Callback for the interrupt reception on the RxPin of usart
//*************************************************************
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size){
  
	if (huart->Instance == USART1){

		if(StoreUSARTData(Rx_data, size)){

			osSemaphoreRelease(binarySemaphoreUARTHandle);
		}

		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, Rx_data, UART_MAX_RECEIVE_DATA);

		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
	}
}

//*************************************************************
//*Callback for the button interruption
//*************************************************************
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	
	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

	if(GPIO_Pin == BUTTON_Pin){

		LIS3MDL_StoreData_t message = LIS3MDL_storedata;

		message.statusData = DATA_READ;

		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

		xQueueSendToFrontFromISR(queueUsartSender, &message, &xHigherPriorityTaskWoken);
	}

	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

//*************************************************************
//* @brief  Function implementing the sensorTask thread. It's
// in charge of getting the measure of x, y and z axes of the
// magnetometer and the temperature, and then send it to the
// memoryTask to save the data. In the case that there is a
// problem with the sensor, it will report inmediately to the
// sendTask.
//* @param  argument: Not used
//* @retval None
//*************************************************************
void sensorFunction(void const * argument){

	LIS3MDL_data.scale = LIS3MDL_SCALE_12_GAUSS;

	lis3mdl_Init(&LIS3MDL_data, &hi2c1);

	while(1){

		osDelay(1000);

		if(lis3mdl_DataReady(&LIS3MDL_data) == HAL_OK){

			LIS3MDL_data.uid++;

			lis3mdl_ReadMagnetometer(&LIS3MDL_data);

			lis3mdl_ReadTemperature(&LIS3MDL_data);

			LIS3MDL_storedata = lis3mdl_ConvertToStoreData(&LIS3MDL_data);

			LIS3MDL_storedata.statusData = SAVE_DATA;

			xQueueSend(queueDataProcessing, &LIS3MDL_storedata, portMAX_DELAY);
		}

		else {

			LIS3MDL_storedata.statusData = LIS3MDL_ERROR;

			xQueueSend(queueUsartSender, &LIS3MDL_storedata, portMAX_DELAY);
		}
	}
}

//*************************************************************
//* @brief Function implementing the memoryTask thread. It's in
// charge to save the data of the sensor in the memory and
// reporting the data of the memory if it receives an id number
// from the receiveTask. In case there is a problem with the
// memory,it will report inmediately to the sendTask.
//* @param argument: Not used
//* @retval None
//*************************************************************
void recordingFunction(void const * argument){

	LIS3MDL_StoreData_t message;

	while(1){

		xQueueReceive(queueDataProcessing, &message, portMAX_DELAY);

		if(W25Q80DV_data.statusMemInit == TRUE){

			if(message.statusData == SAVE_DATA){

				uint8_t splitData = 0;

				if(w25q80dv_AddressToWrite(&W25Q80DV_data, (sizeof(LIS3MDL_StoreData_t) - 1), &splitData)){
					// We must save the data in memory
					w25q80dv_WriteBytesInSequence(&W25Q80DV_data, (uint8_t*)&message, (sizeof(LIS3MDL_StoreData_t) - 1));
					// We save in the first bytes the last id save in memory, for the next power up
					w25q80dv_WriteBytesInAddress(&W25Q80DV_data, INIT_DATA_ADDRESS, (uint8_t*)&message, INIT_DATA_SIZE);

					message.statusData = DATA_SAVE;
				}

				else {
					// There's no more space in memory, we must adopt a rule, overwrite data starting
					// with the first page or send an alert.
					message.statusData = MEMORY_FULL;
					// Send the data to the sendTask
					xQueueSend(queueUsartSender, &message, portMAX_DELAY);
				}
			}

			if(message.statusData == READ_DATA){
				// First we must get the address of that data id
				uint32_t addressAskID = message.uid*(sizeof(LIS3MDL_StoreData_t) - 1) + (INIT_DATA_ADDRESS + INIT_DATA_SIZE);

				if(addressAskID > 0
					&& addressAskID <= (W25Q80DV_LAST_ADDRESS - (sizeof(LIS3MDL_StoreData_t) - 1))
					&& message.uid <= LIS3MDL_data.uid){
					// We must read the data in memory of the address
					w25q80dv_ReadBytesInAddress(&W25Q80DV_data, addressAskID, (uint8_t*)&message, (sizeof(LIS3MDL_StoreData_t) - 1));

					message.statusData = DATA_READ;
				}

				else {
					// The ID selected is wrong, send error message
					message.statusData = WRONG_ID;
				}
				// Send the data to the sendTask
				xQueueSend(queueUsartSender, &message, portMAX_DELAY);
			}
		}

		else {

			// The memory is not operative
			message.statusData = MEMORY_ERROR;
			// Send the data to the sendTask
			xQueueSend(queueUsartSender, &message, portMAX_DELAY);
		}
	}
}

//*************************************************************
//* @brief Function implementing the receiveTask thread. Decodes
// the data from the usart1 when the interruption is trigger. In
// case there is a problem with the decoding of the string into
// a integer, it will report inmediately to the sendTask.
//* @param argument: Not used
//* @retval None
//*************************************************************
void receptionFunction(void const * argument){

	LIS3MDL_StoreData_t message;

	while(1){
		
		osSemaphoreWait(binarySemaphoreUARTHandle, osWaitForever);
			
		uint16_t measureAsk = 0;
			
		if(DecodeReceivedData(&measureAsk)){

			message.uid = measureAsk;

			message.statusData = READ_DATA;
			// Send the data to the sendTask
			xQueueSend(queueDataProcessing, &message, portMAX_DELAY);
		}

		else {

			message.statusData = UNKNOWN_ERROR;
			// Send the data to the sendTask
			xQueueSend(queueUsartSender, &message, portMAX_DELAY);
		}
	}
}

//*************************************************************
//* @brief Function implementing the sendTask thread. Receives
// all the data from the other task and report what is neccesary
// in the usart1.
//* @param argument: Not used
//* @retval None
//*************************************************************
void sendingFunction(void const * argument){

	LIS3MDL_StoreData_t message;

	while(1){

		xQueueReceive(queueUsartSender, &message, portMAX_DELAY);

		switch(message.statusData){

			case MEMORY_FULL:

				PrintString(huart1, "Memory full", 11);

			break;

			case WRONG_ID:

				PrintString(huart1, "Wrong ID", 8);

			break;

			case DATA_READ:

				PrintString(huart1, "Data sensor -> x:", 15);

				PrintFloat(huart1, message.mag_x, 4);

				PrintString(huart1, " , y: ", 6);

				PrintFloat(huart1, message.mag_y, 4);

				PrintString(huart1, " , z: ", 6);

				PrintFloat(huart1, message.mag_z, 4);

				PrintString(huart1, " , temp: ", 9);

				PrintFloat(huart1, message.temp, 4);

			break;

			case UNKNOWN_ERROR:

				PrintString(huart1, "Unknown error", 13);

			break;

			case LIS3MDL_ERROR:

				PrintString(huart1, "Lis3mdl error", 13);

			break;

			case MEMORY_ERROR:

				PrintString(huart1, "Memory error", 12);

			break;

			case DATA_SAVE:
			case NO_ERROR:
			case READ_DATA:
			case SAVE_DATA:
			break;
		}

		PrintEnter(huart1);
	}
}

//*************************************************************
//* @brief  Period elapsed callback in non blocking mode
//* @note   This function is called  when TIM1 interrupt took place, inside
//* HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
//* a global variable "uwTick" used as application time base.
//* @param  htim : TIM handle
//* @retval None
//*************************************************************
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM1) {
		HAL_IncTick();
	}
}

//*************************************************************
//* @brief  This function is executed in case of error occurrence.
//* @retval None
//*************************************************************
void Error_Handler(void){
	__disable_irq();
	while (1){}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line){
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
