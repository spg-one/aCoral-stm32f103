
#include "Adxl345.h"
#include "user.h"

Adxl345_HandleTypeDef adxl345 = {0};

uint8_t WriteLabel = 0;
uint8_t ReadLabel = 0;


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
        val = 0x0B;
        Adxl345_MemWrite(ADDR_DATA_FORMAT_RW, &val, 1);
        val = 0x0A;
        Adxl345_MemWrite(ADDR_BW_RATE_RW, &val, 1);
        val = 0x00;
        Adxl345_MemWrite(ADDR_INT_ENABLE_RW, &val, 1);
        val = 0x00;
        Adxl345_MemWrite(ADDR_OFSX_RW, &val, 1);
        Adxl345_MemWrite(ADDR_OFSY_RW, &val, 1);
        Adxl345_MemWrite(ADDR_OFSZ_RW, &val, 1);
        val = 0x80;
        Adxl345_MemWrite(ADDR_FIFO_CTL_RW, &val, 1);
        val = 0x08;
        Adxl345_MemWrite(ADDR_POWER_CTL_RW, &val, 1);
        
    }
    adxl345.State = Adxl345_STATE_READY;
    __Adxl345_UNLOCK(&adxl345);
    return HAL_OK;
}



HAL_StatusTypeDef Adxl345_GetAccelerations(Adxl345_AccelerationTypeDef *data) {
    uint8_t buf[6] = {0};
    uint8_t times = 10;
    float x, y, z = 0; 
    Adxl345_CCpatureFlagTypeDef CCaptureFlag;
    adxl345.CCaptureFlag = &CCaptureFlag;
    adxl345.CMeasureData = data;
    do {
        adxl345.CCaptureFlag -> StartCapture = 1;
        adxl345.CCaptureFlag -> CompleteCapture = 0;
        Adxl345_MemRead(ADDR_DATAX0_RO, &buf[0], 6);
        if((adxl345.CCaptureFlag -> StartCapture == 1) && (adxl345.CCaptureFlag -> CompleteCapture == 1 )){
            x += (float)( (short)( ((uint16_t)buf[1] << 8) + (uint16_t)buf[0] ) );
            y += (float)( (short)( ((uint16_t)buf[3] << 8) + (uint16_t)buf[2] ) );
            z += (float)( (short)( ((uint16_t)buf[5] << 8) + (uint16_t)buf[4] ) );
            times--;
        }
    } while (times > 0);
    adxl345.CMeasureData -> XAxisAcceleration = x * 0.4;  // (x/10)*4.0  => mg
    adxl345.CMeasureData -> YAxisAcceleration = y * 0.4;
    adxl345.CMeasureData -> ZAxisAcceleration = z * 0.4;
    adxl345.CCaptureFlag = NULL;
    adxl345.CMeasureData = NULL;
    
    return HAL_OK;
}

void getXYZAxisAccelerationsThread() {
	Adxl345_AccelerationTypeDef acceleration_data;
	acceleration_data.XAxisAcceleration = 0;
    acceleration_data.YAxisAcceleration = 0;
    acceleration_data.ZAxisAcceleration = 0;
    Adxl345_GetAccelerations(&acceleration_data);
    data_ready|=(1<<2);
    *((float *)(&Buffer[11])) = acceleration_data.XAxisAcceleration;
    *((float *)(&Buffer[15])) = acceleration_data.YAxisAcceleration;
    *((float *)(&Buffer[19])) = acceleration_data.ZAxisAcceleration;
	// acoral_print("The acceleration of X-Axis：%d", (int)acceleration_data.XAxisAcceleration);
	// acoral_print("The acceleration of Y-Axis：%d", (int)acceleration_data.YAxisAcceleration);
	// acoral_print("The acceleration of Z-Axis：%d", (int)acceleration_data.ZAxisAcceleration);
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




