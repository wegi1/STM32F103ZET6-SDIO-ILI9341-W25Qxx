/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "w25qxx.h"
#include "ili9341.h"
#include "pic02.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t idx[12];
const uint8_t filename[] = {'p','i','c','0','1','.','b','i','n'};
uint8_t act_name[15];
FRESULT SD_RESULT;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//************************************
void copy_name(uint8_t data)
{
	uint32_t i;
	for(i = 0; i < sizeof(filename); i++)
	{
		act_name[i] = filename[i];
	}
	act_name[i] = 0;
	act_name[4] = data + 0x31; // set number of picture
}
//*****************************************
void lcd_setup_picture(uint8_t pic_nr)
{
    uint8_t dana ;
    uint16_t x, y;
    lcdOrientationTypeDef dana2;

    lcdOrientationTypeDef setup_pic[] = {
            LCD_ORIENTATION_LANDSCAPE,
            LCD_ORIENTATION_PORTRAIT_MIRROR,
            LCD_ORIENTATION_PORTRAIT,
            LCD_ORIENTATION_LANDSCAPE_MIRROR,
            LCD_ORIENTATION_LANDSCAPE_MIRROR,
            LCD_ORIENTATION_LANDSCAPE,
            LCD_ORIENTATION_LANDSCAPE,
            LCD_ORIENTATION_LANDSCAPE,
            LCD_ORIENTATION_LANDSCAPE,
            LCD_ORIENTATION_LANDSCAPE,
            LCD_ORIENTATION_LANDSCAPE
            };

    dana = pic_nr;
    if (dana > 10) {dana =0;}
    dana2 =  setup_pic[dana];
    lcdSetOrientation(dana2);

    x=319;
    y=239;

    if((dana2 == LCD_ORIENTATION_PORTRAIT  ) || (dana2 == LCD_ORIENTATION_PORTRAIT_MIRROR))
    {
        y=319;
        x=239;
    }
    lcdSetWindow(0, 0, x, y);
}
//************************************
void readPicFromFlash(void)
{
	uint32_t i;
	uint32_t page_nr, i2;
	uint16_t rx_buff[128];
	uint16_t* RX_ADDR  = (uint16_t*) &rx_buff[0];

	lcd_setup_picture(1);

	page_nr = 0;

	for(i = 0; i < 600; i++)
	{
		W25qxx_ReadPage((uint8_t*)RX_ADDR, page_nr, 0, 256);
		page_nr ++;
		for(i2 = 0; i2 < 128 ; i2++){
			LCD->LCD_RAM = rx_buff[i2];
		}
	}
}
//************************************
int open_file_From_SD(uint8_t file_nr)
{

	SD_RESULT = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
	if( SD_RESULT != FR_OK)
	{
		lcdFillRGB(COLOR_CYAN);
		return 1;
	}
	copy_name(file_nr);
	lcd_setup_picture(file_nr);

	if(f_open(&SDFile, (char*) &act_name, FA_READ) != FR_OK)
	{
		lcdFillRGB(COLOR_YELLOW);
		f_mount(NULL, (TCHAR const*)NULL, 0);
		return 2;
	}

	return 0;
}
uint16_t rx_buff[256];
int readPicFrom_SD(uint8_t pic_nr)
{

	uint32_t i, i2, bytesread;
	uint16_t* RX_ADD  = (uint16_t*) &rx_buff[0];


	if((open_file_From_SD(pic_nr)) != 0){
		return 1;
	}


	for(i2 = 0; i2 < 300; i2++)
	{
		SD_RESULT = f_read(&SDFile, (uint8_t*) &rx_buff[0], 512, (void *)&bytesread);
		if((bytesread == 0) || (SD_RESULT != FR_OK))
		{
			lcdFillRGB(COLOR_RED);
			f_close(&SDFile);
			f_mount(NULL, (TCHAR const*)NULL, 0);
			return 2;
		}
		else
		{
			for(i=0;i<256;i++) { LCD_DataWrite(RX_ADD[i]); }
		}
	}

	f_close(&SDFile);
	f_mount(NULL, (TCHAR const*)NULL, 0);

	HAL_Delay(3000);

	return 0;
}
//*******************************************************************************************
// if you want store picture into external flash you need unrem line below "#define photos"
//#define save_photos

#ifdef save_photos
void savePicToFlash(void){

	uint32_t index=0;

	W25qxx_EraseBlock(0);
	W25qxx_EraseBlock(1);
	W25qxx_EraseBlock(2);

	W25qxx_WriteBlock((uint8_t*)&laki[index], 0, 0,65536);index+=65536/2;
	W25qxx_WriteBlock((uint8_t*)&laki[index], 1, 0,65536);index+=65536/2;
	W25qxx_WriteBlock((uint8_t*)&laki[index], 2, 0,22528);
}
#endif

//************************************
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FSMC_Init();
  MX_SPI2_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */

  lcdSetTextFont(&Font16);
  LCD_ILI9341_init();
  lcd_setup_picture(1);

#ifdef save_photos
  if (W25qxx_Init()) {

	  lcdSetOrientation(LCD_ORIENTATION_LANDSCAPE);
	  lcdFillRGB(COLOR_BLACK);
	  lcdSetTextFont(&Font24);
	  lcdSetCursor(0, 70);
	  lcdPrintf(" WAIT FOR WRITE\n");
	  lcdPrintf("  DATA TO FLASH\n\n");
	  lcdPrintf("  ID : 0x%s\n", &idx[4]);
	  lcdSetTextFont(&Font16);
	  savePicToFlash();

	  lcdFillRGB(COLOR_BLUE);
	  readPicFromFlash();
  }
#endif



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1)
  {
	  lcdSetOrientation(LCD_ORIENTATION_LANDSCAPE);
	  lcdFillRGB(COLOR_BLACK);

	  lcdSetTextFont(&Font24);
	  lcdSetCursor(20, 100);
	  lcdPrintf("READ FROM SD CARD\n");
	  HAL_Delay(2000);

	  for(uint8_t pic = 0 ; pic < 7 ; pic++)
	  {
		  int stats = readPicFrom_SD(pic);
		  if(stats != 0)
		  {
			  lcdSetOrientation(LCD_ORIENTATION_LANDSCAPE);
			  lcdFillRGB(COLOR_BLACK);

			  lcdSetTextFont(&Font24);
			  lcdSetCursor(20, 100);
			  lcdPrintf("SD READ ERROR...\n");
			  HAL_Delay(2000);
			  break;
		  }
	  }

	  lcdSetOrientation(LCD_ORIENTATION_LANDSCAPE);
	  lcdFillRGB(COLOR_BLACK);

	  lcdSetTextFont(&Font24);
	  lcdSetCursor(20, 100);
	  lcdPrintf("TEST SPI FLASH\n");
	  HAL_Delay(2000);

	  lcdSetOrientation(LCD_ORIENTATION_LANDSCAPE);
	  lcdFillRGB(COLOR_BLACK);
	  lcdSetTextFont(&Font16);
	  lcdSetCursor(0, 10);
	  lcdPrintf("INIT OF SPI FLASH W25Qxx :");
	  if (W25qxx_Init())
	  {
		  lcdPrintf("OK\n");
		  lcdPrintf("FACTORY ID : 0x%s\n", &idx[4]);
		  lcdPrintf("CAPACITY :%d KB\n",w25qxx.CapacityInKiloByte);
		  lcdPrintf("SECTOR COUNT :%d\n",w25qxx.SectorCount);
		  lcdPrintf("SECTOR SIZE :%d\n",w25qxx.SectorSize);
		  lcdPrintf("BLOCK COUNT :%d\n",w25qxx.BlockCount);
		  lcdPrintf("BLOCK SIZE :%d\n",w25qxx.BlockSize);
		  lcdPrintf("PAGE COUNT :%d\n",w25qxx.PageCount);
		  lcdPrintf("PAGE SIZE :%d\n",w25qxx.PageSize);

		  lcdPrintf("DATA EXIST IN EXT. FLASH\n");
		  lcdPrintf("---------------------------\n");
		  lcdPrintf("-  READ EXT.  SPI  FLASH  -\n");
		  lcdPrintf("---------------------------\n");
		  HAL_Delay(5000);
		  lcdFillRGB(COLOR_BLACK);
		  lcdSetOrientation(LCD_ORIENTATION_LANDSCAPE);

		  lcdSetTextFont(&Font24);
		  lcdSetCursor(10, 50);
		  lcdPrintf("READ PIC FROM\n");
		  lcdPrintf("    W25Qxx\n");
		  lcdPrintf("EXT. SPI FLASH\n\n");
		  lcdPrintf("  ID : 0x%s", &idx[4]);
		  HAL_Delay(3500);
		  readPicFromFlash();
		  HAL_Delay(3000);
	  }
	  else
	  {
		  lcdSetTextFont(&Font24);
		  lcdSetCursor(70, 100);
		  lcdPrintf("FLASH ERROR...\n");
		  HAL_Delay(2000);
	  }

#ifdef save_photos
	  lcdFillRGB(COLOR_BLACK);
	  lcdSetOrientation(LCD_ORIENTATION_LANDSCAPE);
	  lcdSetTextFont(&Font24);
	  lcdSetCursor(10, 100);
	  lcdPrintf("READ INT. FLASH\n");
	  HAL_Delay(2000);
	  lcd_setup_picture(1);
	  for(uint32_t i = 0 ; i < 76800 ; i++) {
		  LCD->LCD_RAM = laki[i];
	  }
	  HAL_Delay(5000);
#endif


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
