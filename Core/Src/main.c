/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sensor_filter.h"
#include "led_dimmer.h"
#include "seg7_display.h"
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
// Ð?nh nghia State Machine cho ch? d? ho?t d?ng
typedef enum {
    MODE_AUTO = 0,
    MODE_MANUAL = 1
} SystemMode_t;

volatile SystemMode_t current_mode = MODE_AUTO;
volatile uint8_t is_person_detected = 0;      // C? báo có ngu?i t? PIR
volatile uint32_t last_motion_tick = 0;       // Th?i di?m cu?i cùng PIR còn phát hi?n ngu?i
volatile uint32_t last_button_tick = 0;       // Bi?n ch?ng d?i nút nh?n

uint16_t adc_dma_buffer[2];                   // [0]: LDR (PA0), [1]: Bi?n tr? (PA1)
MovingAverageFilter_t ldr_filter;
MovingAverageFilter_t pot_filter;

#define PIR_HOLD_TIME_MS   3000              // Gi? dèn sáng 10 giây sau khi ngu?i r?i di
#define DAYLIGHT_THRESHOLD 2000               // Ngu?ng tr?i n?ng g?t thì t?t h?n dèn
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// HÀM L?NG NGHE S? KI?N NG?T NGOÀI (EXTI CALLBACK)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    uint32_t current_tick = HAL_GetTick();

    // 1. X? lý s? ki?n nh?n nút (Chân PB0 - EXTI0)
    if (GPIO_Pin == GPIO_PIN_0) {
        // Ch?ng d?i phím b?ng ph?n m?m (Software Debounce 200ms)
        if (current_tick - last_button_tick > 200) {
            current_mode = (current_mode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
            last_button_tick = current_tick;
        }
    }

    // 2. X? lý s? ki?n t? C?m bi?n PIR HC-SR501 (Chân PB1 - EXTI1)
    if (GPIO_Pin == GPIO_PIN_1) {
        // Ð?c tr?ng thái th?c t? c?a chân PB1 t?i th?i di?m ng?t
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
            // Su?n lên (Rising Edge): Có ngu?i bu?c vào
            is_person_detected = 1;
        }
        // C?p nh?t l?i m?c th?i gian m?i khi có bi?n d?ng t? PIR
        last_motion_tick = current_tick;
    }
}
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  // Kh?i t?o các Service / Module
  Filter_Init(&ldr_filter);
  Filter_Init(&pot_filter);
  Dimmer_Init(&htim2, TIM_CHANNEL_3); 
  Seg7_Init(); 

  // B?t d?u quét ADC 2 kênh liên t?c b?ng DMA
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // BU?C 1: L?y d? li?u thô t? DMA và dua qua b? l?c trung bình tru?t
    uint16_t ldr_val = Filter_Update(&ldr_filter, adc_dma_buffer[0]);
    uint16_t pot_val = Filter_Update(&pot_filter, adc_dma_buffer[1]);

    uint8_t target_brightness = 0; // Ð? sáng m?c tiêu (0 - 100%)

// BU?C 2: X? lý Business Logic theo ch? d? (State Machine)
    if (current_mode == MODE_MANUAL) {
        // Ch? d? ch?nh tay: Ph? thu?c 100% vào Bi?n tr? (0 -> 4095)
        target_brightness = (uint8_t)((pot_val * 100) / 4095);
    } 
    else {
        // Ch? d? T? d?ng: Hai c?m bi?n ho?t d?ng Ð?C L?P (Logic OR)
        
        // 1. C?p nh?t tr?ng thái C?m bi?n ngu?i (PIR)
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
            is_person_detected = 1;
            last_motion_tick = HAL_GetTick();
        } 
        else if (HAL_GetTick() - last_motion_tick > PIR_HOLD_TIME_MS) {
            is_person_detected = 0;
        }

        // 2. Tính toán d? sáng d?a trên Quang tr? (LDR) tru?c
        uint8_t ldr_brightness = 0;
        
        // Ð?t ngu?ng tr?i sáng (Ví d?: 1000). Du?i m?c này là tr?i ban ngày.
        if (ldr_val <= 1000) { 
            ldr_brightness = 0; // Tr?i ban ngày
        } else {
            // Tr?i s?p t?i (ADC > 1000) -> % d? sáng tang d?n theo d? t?i
            ldr_brightness = (uint8_t)((ldr_val * 100) / 4095);
        }

        // 3. X? lý Logic HO?C (Quy?t d?nh d? sáng cu?i cùng)
        if (ldr_brightness > 0) {
            // BAN ÐÊM: Quang tr? dã báo t?i -> Uu tiên b?t dèn theo d? t?i c?a tr?i
            // (Lúc này không c?n quan tâm bi?n is_person_detected n?a)
            target_brightness = ldr_brightness;
        } 
        else {
            // BAN NGÀY: Quang tr? báo sáng (ldr_brightness = 0)
            if (is_person_detected) {
                // Có ngu?i di qua vào ban ngày -> Ép b?t dèn ? m?c c? d?nh (VD: 50%)
                target_brightness = 100; 
            } else {
                // Không có ngu?i vào ban ngày -> T?t dèn hoàn toàn
                target_brightness = 0;
            }
        }
    }

    // BU?C 3: Ð?y ra t?ng Output (Qua hi?u ch?nh Gamma -> PWM MOSFET)
    Dimmer_SetBrightness(&htim2, TIM_CHANNEL_3, target_brightness);

    // BU?C 4: Hi?n th? LED 7 do?n (T?i uu I/O: Ch? d?y data khi % thay d?i)
    static uint8_t last_displayed_percent = 255;
    if (target_brightness != last_displayed_percent) {
        Seg7_DisplayPercent(target_brightness); 
        last_displayed_percent = target_brightness;
    }

    HAL_Delay(50); // Chu k? l?y m?u và c?p nh?t h? th?ng: 50ms (20 l?n/giây)
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
#ifdef USE_FULL_ASSERT
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
/* EOF */