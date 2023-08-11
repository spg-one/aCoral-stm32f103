
#include "HcSr04.h"



HcSr04_HandleTypeDef hcsr04 = {0};
// The params of TIM7 Clock
extern void TIM7_PeriodElapsedCallback(void);


void HC_SR04_RECV_IC_CaptureCallback(void);

void HC_SR04_RECV_PeriodElapsedCallback(void);

float compute_distance(uint32_t count, float temperature); 

#if defined(USE_BARE_BOARD)
void HC_SR04_SEND_PeriodElapsedCallback(void);
#define GET_TEMPERATURE()   20;
void delay_20us(void);
#else 
// When use OS, this need modified.
#endif


HAL_StatusTypeDef HcSr04_Init(void)
{
#if defined(USE_BARE_BOARD)
    __HcSr04_LOCK(&hcsr04);
#else
    hcsr04.Lock = 1;    /* When use OS, this need modified */
#endif
    if(hcsr04.State == HcSr04_STATE_BUSY)
    {
        return HAL_BUSY;
    }

    hcsr04.State = HcSr04_STATE_BUSY;
    hcsr04.PreMeasureState = HcSr04_M_STATE_NONE;
    hcsr04.Init.MeasureTimeGap = 210;
    hcsr04.Init.TrigPinHighLevelTime = 20; 
    hcsr04.Init.MaximumCapacity = 600;
    hcsr04.CCaptureFlag = NULL;
    hcsr04.CMeasureData = NULL;
    
    hcsr04.TrigGPIOGroup = GPIOC;
    hcsr04.TrigGPIOPin = GPIO_PIN_0;
    // PC0_GPIO_Init();
#if defined(USE_BARE_BOARD)
    hcsr04.TrigPeripheral2Handle = &htim6;
    // MX_TIM6_Init();
    hcsr04.TimingIsOK = 0;
#endif
    hcsr04.EchoPeripheralHandle = &htim3;
    // MX_TIM3_Init();
    
    hcsr04.State = HcSr04_STATE_READY;
#if defined(USE_BARE_BOARD)
   __HcSr04_UNLOCK(&hcsr04);
#else
    hcsr04.Lock = 0;    /* When use OS, this need modified */
#endif
    return HAL_OK;
}

HAL_StatusTypeDef HcSr04_GetDistance(HcSr04_DistanceTypeDef *data) {
    uint32_t measure_time_gap = 0;
    HAL_StatusTypeDef status = HAL_OK;
#if defined(USE_BARE_BOARD)
    __HcSr04_LOCK(&hcsr04);
#else
    hcsr04.Lock = 1;    /* When use OS, this need modified */
#endif
    hcsr04.State = HcSr04_STATE_BUSY;
    HcSr04_CCpatureFlagTypeDef current_flag = {0};
    hcsr04.CCaptureFlag = &current_flag;
    hcsr04.CMeasureData = data;
    hcsr04.CMeasureData -> Distance = 0;
    hcsr04.CMeasureData -> Temperature = 0;
    hcsr04.CMeasureData -> TimerCount = 0;
    hcsr04.CMeasureData -> OverRange = 0;
#if defined(USE_BARE_BOARD)
    measure_time_gap = HAL_GetTick() - hcsr04.PreMeasureTick;
    if(measure_time_gap < hcsr04.Init.MeasureTimeGap){
        HAL_Delay(hcsr04.Init.MeasureTimeGap - measure_time_gap);
    }
#else
    // When use OS , the code in here need modified.
#endif
    // Send Measure Instruction to HC_SR04.
    HAL_GPIO_WritePin(hcsr04.TrigGPIOGroup, hcsr04.TrigGPIOPin, GPIO_PIN_SET);

#if defined(USE_BARE_BOARD)
    delay_20us();
#else
    // When use OS , the code in here need modified.
#endif
    HAL_GPIO_WritePin(hcsr04.TrigGPIOGroup, hcsr04.TrigGPIOPin, GPIO_PIN_RESET);
     // Start capture the signal 
    hcsr04.CCaptureFlag -> StartCapture = 1;
    hcsr04.EchoPeripheralHandle -> Instance -> CNT = 0;
    HAL_TIM_IC_Start_IT(hcsr04.EchoPeripheralHandle, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(hcsr04.EchoPeripheralHandle, TIM_CHANNEL_2);
    // The distance is over range.
    while(!(hcsr04.CCaptureFlag -> CompleteCapture));
#if defined(USE_BARE_BOARD)
    hcsr04.PreMeasureTick = HAL_GetTick();
#else
    // When use OS , the code in here need modified.
#endif
    if(hcsr04.CCaptureFlag -> OverRange){ 
        hcsr04.CMeasureData -> Distance = 0;
        hcsr04.CMeasureData -> TimerCount = 0;
        hcsr04.CMeasureData -> OverRange = 1;
        hcsr04.PreMeasureState = HcSr04_M_STATE_OVER_RANGE;
        hcsr04.CCaptureFlag = NULL;
        status = HAL_ERROR;
    }else { 
    // The distance is in range.
        hcsr04.CMeasureData -> Temperature = GET_TEMPERATURE();
        hcsr04.CMeasureData -> Distance = compute_distance(hcsr04.CMeasureData -> TimerCount, hcsr04.CMeasureData -> Temperature);
        hcsr04.PreMeasureState = HcSr04_M_STATE_IN_RANGE;
        hcsr04.CCaptureFlag = NULL;
    }
    hcsr04.State = HcSr04_STATE_READY;
#if defined(USE_BARE_BOARD)
   __HcSr04_UNLOCK(&hcsr04);
#else
    hcsr04.Lock = 0;    /* When use OS, this need modified */
#endif
    return status;
}

#if defined(USE_BARE_BOARD)
void delay_20us(void) {
    hcsr04.TimingIsOK = 0;
    hcsr04.TrigPeripheral2Handle -> Instance -> CNT = 0x00;
    HAL_TIM_Base_Start_IT(hcsr04.TrigPeripheral2Handle);
    while(!hcsr04.TimingIsOK);
    HAL_TIM_Base_Stop_IT(hcsr04.TrigPeripheral2Handle);
    hcsr04.TimingIsOK = 0;
}

void HC_SR04_SEND_PeriodElapsedCallback(void){
    if (!hcsr04.TimingIsOK){
        hcsr04.TimingIsOK = 1;
    }
}
#endif

float compute_distance(uint32_t count, float temperature) {
    float C = 0;
    // float time = 0;
    // float freq = 1800000;       // 72/40 MHz
    // time = count/1800000;
    // Compute the speed of wind.
    if(temperature < 0){
        C = 340;
    }else if (temperature > 40){
        C = 355;
    }else {
        C = 330.45 + 0.61 * temperature;
    }
    return ((float)count/18000)*(C/2);
}


// The callback which will execute in timer IC capture interrupt function
void HC_SR04_RECV_IC_CaptureCallback(void){
    if(hcsr04.EchoPeripheralHandle -> Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        if((hcsr04.CCaptureFlag -> StartCapture)&&(!hcsr04.CCaptureFlag -> Rising)){
            hcsr04.CCaptureFlag -> Rising = 1;
        }
    } else if (hcsr04.EchoPeripheralHandle -> Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
        if((hcsr04.CCaptureFlag -> StartCapture)&&(hcsr04.CCaptureFlag -> Rising)) {
            hcsr04.CCaptureFlag -> Falling = 1;
            hcsr04.CMeasureData -> TimerCount = HAL_TIM_ReadCapturedValue(hcsr04.EchoPeripheralHandle, TIM_CHANNEL_2) + 2;
            HAL_TIM_IC_Stop_IT(hcsr04.EchoPeripheralHandle, TIM_CHANNEL_1);
            HAL_TIM_IC_Stop_IT(hcsr04.EchoPeripheralHandle, TIM_CHANNEL_2);
            hcsr04.CCaptureFlag -> CompleteCapture = 1;
        }
    }
}

// The callback which will execute in timer period elapsed interrupt function
void HC_SR04_RECV_PeriodElapsedCallback(void) {
    if(hcsr04.CCaptureFlag -> StartCapture && hcsr04.CCaptureFlag -> Rising) {
        if (!hcsr04.CCaptureFlag -> CompleteCapture){
            hcsr04.CCaptureFlag -> OverRange = 1;
            HAL_TIM_IC_Stop_IT(hcsr04.EchoPeripheralHandle, TIM_CHANNEL_1);
            HAL_TIM_IC_Stop_IT(hcsr04.EchoPeripheralHandle, TIM_CHANNEL_2);
            hcsr04.CCaptureFlag -> CompleteCapture = 1;
        } 
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if(htim == hcsr04.EchoPeripheralHandle) {
        HC_SR04_RECV_IC_CaptureCallback();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if(htim == hcsr04.EchoPeripheralHandle){
        HC_SR04_RECV_PeriodElapsedCallback();
#if defined(USE_BARE_BOARD)
    }else if(htim == hcsr04.TrigPeripheral2Handle){
        HC_SR04_SEND_PeriodElapsedCallback();
    }
#else
    }
#endif
    else if (htim == &htim7) {
		TIM7_PeriodElapsedCallback();
	}
}

void get_distance_thread()
{
    HcSr04_DistanceTypeDef data;
    if(HcSr04_GetDistance(&data) == HAL_OK) {
        // acoral_print("Distance: %d cm\r\n", (int)data.Distance);
        Buffer[2] = (int)data.Distance;
        data_ready|=1;
    } else {
        acoral_print("Disctance Measure Failure -----");
    }
    
}

