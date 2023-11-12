
#include "Adxl345.h"
#include "user.h"

Adxl345_HandleTypeDef adxl345 = {0};

uint8_t WriteLabel = 0;
uint8_t ReadLabel = 0;
uint8_t intr_flag = 0;
uint8_t let_thread_replace_irq = 0;


HAL_StatusTypeDef Adxl345_MemWrite(uint16_t MemAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef Adxl345_MemRead(uint16_t MemAddress, uint8_t *pData, uint16_t Size);


HAL_StatusTypeDef Adxl345_MemWrite(uint16_t MemAddress, uint8_t *pData, uint16_t Size){
    WriteLabel = 0;
    HAL_I2C_Mem_Write_IT(adxl345.PeripheralHandle, ADDR_ADXL345_Write, MemAddress, I2C_MEMADD_SIZE_8BIT, pData, Size);
    while(!WriteLabel);
    WriteLabel = 0;
    return HAL_OK;
}

HAL_StatusTypeDef Adxl345_MemRead(uint16_t MemAddress, uint8_t *pData, uint16_t Size){
    ReadLabel = 0;
    HAL_I2C_Mem_Read_IT(adxl345.PeripheralHandle, ADDR_ADXL345_Read, MemAddress, I2C_MEMADD_SIZE_8BIT, pData, Size);
    while(!ReadLabel);
    ReadLabel = 0;
    return HAL_OK;
}

HAL_StatusTypeDef Adxl345_Init(void) {
    __Adxl345_LOCK(&adxl345);
    adxl345.State = Adxl345_STATE_BUSY;
    adxl345.PeripheralHandle = &hi2c1;
    adxl345.CCaptureFlag = NULL;
    adxl345.CMeasureData = NULL;
    // MX_I2C1_Init();
    HAL_Delay(1000);
    uint8_t val;
    Adxl345_MemRead(ADDR_DEVID_RO, &val, 1);
    // If the device is ADXL345
    if(val == 0xE5) {
        val = 0x0B; // 11
        Adxl345_MemWrite(ADDR_DATA_FORMAT_RW, &val, 1);
        val = 0x0A; // 10
        Adxl345_MemWrite(ADDR_BW_RATE_RW, &val, 1);

        val = 0x00; // 0
        Adxl345_MemWrite(ADDR_OFSX_RW, &val, 1);
        Adxl345_MemWrite(ADDR_OFSY_RW, &val, 1);
        Adxl345_MemWrite(ADDR_OFSZ_RW, &val, 1);
        // set 600mg
        val = 0x09; // 9
        Adxl345_MemWrite(ADDR_THRESH_FF_RW, &val, 1);
        // set 100 ms 
        val = 0x14; // 20 
        Adxl345_MemWrite(ADDR_TIME_FF_RW, &val, 1);
        val = 0x00; // 0
        Adxl345_MemWrite(ADDR_INT_MAP_RW, &val, 1);
        val = 0x80; // 128
        Adxl345_MemWrite(ADDR_FIFO_CTL_RW, &val, 1);
        val = 0x08; // 8
        Adxl345_MemWrite(ADDR_POWER_CTL_RW, &val, 1);
        val = 0x04; // 4
        Adxl345_MemWrite(ADDR_INT_ENABLE_RW, &val, 1);

        // read registers 
        val = 0x00;
        Adxl345_MemRead(ADDR_DATA_FORMAT_RW, &val, 1);

        Adxl345_MemRead(ADDR_BW_RATE_RW, &val, 1);


        Adxl345_MemRead(ADDR_OFSX_RW, &val, 1);
        Adxl345_MemRead(ADDR_OFSY_RW, &val, 1);
        Adxl345_MemRead(ADDR_OFSZ_RW, &val, 1);
        // set 600mg

        Adxl345_MemRead(ADDR_THRESH_FF_RW, &val, 1);
        // set 100 ms 

        Adxl345_MemRead(ADDR_TIME_FF_RW, &val, 1);
        Adxl345_MemRead(ADDR_INT_MAP_RW, &val, 1);
        Adxl345_MemRead(ADDR_FIFO_CTL_RW, &val, 1);
        Adxl345_MemRead(ADDR_POWER_CTL_RW, &val, 1);

        Adxl345_MemRead(ADDR_INT_ENABLE_RW, &val, 1);
        
    }
    HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
    adxl345.State = Adxl345_STATE_READY;
    
    __Adxl345_UNLOCK(&adxl345);
    return HAL_OK;
}



HAL_StatusTypeDef Adxl345_GetAccelerations(Adxl345_AccelerationTypeDef *data, uint8_t times) {
    if(adxl345.State != Adxl345_STATE_READY) {
        return HAL_BUSY;
    }
    adxl345.State = Adxl345_STATE_BUSY;
    uint8_t buf[6] = {0};
    // uint8_t times = 10;
    float x, y, z = 0; 
    Adxl345_CCpatureFlagTypeDef CCaptureFlag;
    adxl345.CCaptureFlag = &CCaptureFlag;
    adxl345.CMeasureData = data;
    do {
        adxl345.CCaptureFlag -> StartCapture = 1;
        adxl345.CCaptureFlag -> CompleteCapture = 0;
        Adxl345_MemRead(ADDR_DATAX0_RO, &buf[0], 6);
        if((adxl345.CCaptureFlag -> StartCapture == 1) && (adxl345.CCaptureFlag -> CompleteCapture == 1 )){
            x += ((float)( (short)( ((uint16_t)buf[1] << 8) + (uint16_t)buf[0] ) )) / (float)times;
            y += ((float)( (short)( ((uint16_t)buf[3] << 8) + (uint16_t)buf[2] ) )) / (float)times;
            z += ((float)( (short)( ((uint16_t)buf[5] << 8) + (uint16_t)buf[4] ) )) / (float)times;
            times--;
        }

    } while (times > 0);
    adxl345.CMeasureData -> XAxisAcceleration = x;  //  => mg
    adxl345.CMeasureData -> YAxisAcceleration = y;
    adxl345.CMeasureData -> ZAxisAcceleration = z;
    adxl345.CCaptureFlag = NULL;
    adxl345.CMeasureData = NULL;
    adxl345.State = Adxl345_STATE_READY;
    return HAL_OK;
}


void getXYZAxisAccelerationsThread() {
    if (intr_flag == 1)
    {
        return;
    }
    Adxl345_AccelerationTypeDef acceleration_data;
    acceleration_data.XAxisAcceleration = 0;
    acceleration_data.YAxisAcceleration = 0;
    acceleration_data.ZAxisAcceleration = 0;
    Adxl345_GetAccelerations(&acceleration_data, 25);
    Buffer.acceleration_x = acceleration_data.XAxisAcceleration;
    Buffer.acceleration_y = acceleration_data.YAxisAcceleration;
    Buffer.acceleration_z = acceleration_data.ZAxisAcceleration;
    Buffer.acceleration_period = (((period_private_data_t *)acoral_cur_thread->private_data)->time)/1000;
    acoral_enter_critical();
    Buffer.acceleration_collect_time = timestap;
    acoral_exit_critical();
    if(let_thread_replace_irq ) {
        intr_flag = 1;
        let_thread_replace_irq = 0;
    }
    data_ready|=(1<<2);
}


void Adxl345_MemTxCpltCallback(void) {
    if(!WriteLabel){
        WriteLabel = 1;
    }
}

void Adxl345_MemRxCpltCallback(void) {
    if(!ReadLabel){
        ReadLabel = 1;
    }
    if((adxl345.CCaptureFlag != NULL) && (adxl345.CCaptureFlag -> StartCapture == 1)){
        if(adxl345.CCaptureFlag -> CompleteCapture == 0){
            adxl345.CCaptureFlag -> CompleteCapture = 1;
        }
    }
}

void Adxl345_EXIT_Callback(void) {
    
    uint8_t int_src;
    Adxl345_AccelerationTypeDef acceleration_data;
    if (adxl345.State == Adxl345_STATE_READY) {
        adxl345.State = Adxl345_STATE_BUSY;
        Adxl345_MemRead(ADDR_INT_SOURCE_RO, &int_src, 1);
        adxl345.State = Adxl345_STATE_READY;
        if(int_src & 0x04){
            acceleration_data.XAxisAcceleration = 0;
            acceleration_data.YAxisAcceleration = 0;
            acceleration_data.ZAxisAcceleration = 0;
            Adxl345_GetAccelerations(&acceleration_data, 20);
            // 采集完成
            Buffer.acceleration_x = acceleration_data.XAxisAcceleration;
            Buffer.acceleration_y = acceleration_data.YAxisAcceleration;
            Buffer.acceleration_z = acceleration_data.ZAxisAcceleration;
            Buffer.acceleration_period = cur_period.acceleration;
            Buffer.acceleration_collect_time = timestap;
            intr_flag = 1;
            data_ready|=(1<<2);
            
        }
    } else {
        if (!let_thread_replace_irq) {
            let_thread_replace_irq = 1;
        }    
    }
}


void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if(hi2c == adxl345.PeripheralHandle) {
        Adxl345_MemTxCpltCallback();
    }
}


void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if(hi2c == adxl345.PeripheralHandle) {
        Adxl345_MemRxCpltCallback();
    }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    switch(GPIO_Pin){
        case GPIO_PIN_2:
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_SET) {
                Adxl345_EXIT_Callback();
            }
            break;
        default:
            break;
    }
}




