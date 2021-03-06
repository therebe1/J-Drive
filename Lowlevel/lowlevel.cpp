#include <Lowlevel/lowlevel.hpp>
#include <FastMath/fast_math.hpp>

uint16_t SO1 = 0;
uint16_t SO2 = 0;
uint16_t ADC3Raw[3] = { 0, 0, 0 };
uint8_t adcIdx = 0;

uint8_t SPIDataRx[2];

void (*Control)();

uint16_t bufferCount = 0;

uint8_t phaseOrder = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM1)
	{
		__HAL_TIM_DISABLE_IT(&htim8, TIM_IT_UPDATE);
		__HAL_TIM_DISABLE(&htim8);
		__HAL_TIM_SET_COUNTER(&htim8, 0x0);
		__HAL_TIM_ENABLE_IT(&htim8, TIM_IT_UPDATE);
		__HAL_TIM_ENABLE(&htim8);
	}
	else if (htim->Instance == TIM8)
	{
		HAL_GPIO_TogglePin(TP1_GPIO_Port, TP1_Pin);

		SO1 = HAL_ADC_GetValue(&hadc1);
		SO2 = HAL_ADC_GetValue(&hadc2);

		Control();

		ADC3Raw[adcIdx++] = HAL_ADC_GetValue(&hadc3);
		if (adcIdx == 3)
			adcIdx = 0;

		HAL_ADC_Start(&hadc1);
		HAL_ADC_Start(&hadc2);
		HAL_ADC_Start(&hadc3);

		HAL_GPIO_TogglePin(TP1_GPIO_Port, TP1_Pin);
	}
}

void StartOnBoardLED()
{
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
}

void SetOnBoardLED(uint32_t duty)
{
	htim2.Instance->CCR4 = duty;
}

void SetControlFunc(void (*funcPtr)())
{
	Control = funcPtr;
}

void StartControlTimer()
{
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim8);
}

void StartInverterPWM()
{
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
}

void SetInverterPWMDuty(uint32_t aDuty, uint32_t bDuty, uint32_t cDuty)
{
	if (phaseOrder)
	{
		TIM1->CCR1 = aDuty;
		TIM1->CCR2 = bDuty;
		TIM1->CCR3 = cDuty;
	}
	else
	{
		TIM1->CCR1 = cDuty;
		TIM1->CCR2 = bDuty;
		TIM1->CCR3 = aDuty;
	}
}

void StartADC()
{
	HAL_ADC_Start(&hadc1);
	HAL_ADC_Start(&hadc2);
	HAL_ADC_Start(&hadc3);
}

uint16_t GetSO1()
{
	return SO1;
}

uint16_t GetSO2()
{
	return SO2;
}

uint16_t GetDCVoltageRaw()
{
	return ADC3Raw[1];
}

uint16_t GetMotorTempRaw()
{
	return ADC3Raw[2];
}

uint16_t GetFETTempRaw()
{
	return ADC3Raw[0];
}

void OnGateDriver()
{
	HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_SET);
}

void OffGateDriver()
{
	HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_RESET);
}

uint8_t GateFault()
{
	return !HAL_GPIO_ReadPin(nFAULT_GPIO_Port, nFAULT_Pin);
}

void ADCCalibration(uint8_t status)
{
	if (status)
	{
		HAL_GPIO_WritePin(DC_CAL_GPIO_Port, DC_CAL_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(DC_CAL_GPIO_Port, DC_CAL_Pin, GPIO_PIN_RESET);
	}
}

void SetPhaseOrder(uint8_t _phaseOrder)
{
	phaseOrder = _phaseOrder;
}

void SPITransmitPool(uint8_t *dataTx, uint32_t len)
{
	HAL_GPIO_WritePin(Encoder_CS_GPIO_Port, Encoder_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Encoder_CS_GPIO_Port, Encoder_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi2, dataTx, SPIDataRx, len, 1);
}

void SPITransmit(uint8_t *dataTx, uint32_t len)
{
	HAL_GPIO_WritePin(Encoder_CS_GPIO_Port, Encoder_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Encoder_CS_GPIO_Port, Encoder_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive_IT(&hspi2, dataTx, SPIDataRx, len);
}

uint8_t* SPIReceive()
{
	return SPIDataRx;
}
