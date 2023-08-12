
#ifndef __ADXL345_H__
#define __ADXL345_H__

#include "main.h"


#define ADDR_ADXL345_Write              (uint16_t)(0x53 << 1)
#define ADDR_ADXL345_Read               ADDR_ADXL345_Write + (uint16_t)0x01


/** The macro of ADXL345's regiters **/
/** ADXL345 device ID**/
#define ADDR_DEVID_RO                   0x00

/** Registers of TAP function  **/
#define ADDR_THRESH_TAP_RW              0x1D
#define ADDR_DUR_RW                     0x21
#define ADDR_LATENT_RW                  0x22
#define ADDR_WINDOW_RW                  0x23
#define ADDR_TAP_AXES_RW                0x2A
#define ADDR_ACT_TAP_STATUS_RO          0x2B

/** Registers of detecting ACTIVE  **/
#define ADDR_THRESH_ACT_RW              0x24
#define ADDR_THRESH_INACT_RW            0x25
#define ADDR_TIME_INACT_RW              0x26
#define ADDR_ACT_INACT_CTL_RW           0x27

/** Registers of detecting Free Fall **/
#define ADDR_THRESH_FF_RW               0x28
#define ADDR_TIME_FF_RW                 0x29

/** Register of controling Device **/
#define ADDR_BW_RATE_RW                 0x2C
#define ADDR_POWER_CTL_RW               0x2D

/** Registers of controling interrupts **/
#define ADDR_INT_ENABLE_RW              0x2E
#define ADDR_INT_MAP_RW                 0x2F
#define ADDR_INT_SOURCE_RO              0x30

/** Registers of X Y Z-axis offsets **/
#define ADDR_OFSX_RW                    0x1E
#define ADDR_OFSY_RW                    0x1F
#define ADDR_OFSZ_RW                    0x20

/** Registers of controling the data format of X Y Z-axis acceleration **/
#define ADDR_DATA_FORMAT_RW             0x31

/** Registers of reading x/y/z-axis acceleration **/
#define ADDR_DATAX0_RO                  0x32
#define ADDR_DATAX1_RO                  0x33

#define ADDR_DATAY0_RO                  0x34
#define ADDR_DATAY1_RO                  0x35

#define ADDR_DATAZ0_RO                  0x36
#define ADDR_DATAZ1_RO                  0x37

/** Registers of controling FIFO **/
#define ADDR_FIFO_CTL_RW                0x38
/** Registers of reading FIFO status **/
#define ADDR_FIFO_STATUS_RO             0x39


#if defined(USE_BARE_BOARD)
#define __Adxl345_LOCK(__HANDLE__)                                              \
                            do{                                                 \
                                    if((__HANDLE__)->Lock == Adxl345_Locked)    \
                                    {                                           \
                                        return HAL_BUSY;                        \
                                    }                                           \
                                    else                                        \
                                    {                                           \
                                        (__HANDLE__)->Lock = Adxl345_Locked;    \
                                    }                                           \
                                }while (0U)

#define __Adxl345_UNLOCK(__HANDLE__)                                                 \
                                    do{                                              \
                                          (__HANDLE__)->Lock = Adxl345_UnLocked;     \
                                    }while (0U)
                                    
typedef enum
{
    Adxl345_UnLocked = 0x00U,
    Adxl345_Locked   = 0x01U

} Adxl345_LockTypeDef;
#endif 

typedef enum
{
    Adxl345_STATE_RESET                  = 0x00U,    
    Adxl345_STATE_READY                  = 0x01U,    
    Adxl345_STATE_BUSY                   = 0x03U,    
    Adxl345_STATE_ERROR                  = 0x06U    
} Adxl345_StateTypeDef;


typedef struct
{
    float XAxisAcceleration;
    float YAxisAcceleration;
    float ZAxisAcceleration;
    
} Adxl345_AccelerationTypeDef;

typedef struct
{
    uint8_t StartCapture;
    uint8_t CompleteCapture;
} Adxl345_CCpatureFlagTypeDef;


typedef struct
{
    I2C_HandleTypeDef                   *PeripheralHandle;
#if defined(USE_BARE_BOARD)
    Adxl345_LockTypeDef                 Lock;
#else
    uint8_t                             Lock;           /* When use OS, this need modified */
#endif
    // Adxl345_InitTypeDef              Init;
    Adxl345_StateTypeDef                State;
    Adxl345_CCpatureFlagTypeDef         *CCaptureFlag;
    Adxl345_AccelerationTypeDef         *CMeasureData;

} Adxl345_HandleTypeDef;



HAL_StatusTypeDef Adxl345_Init(void);

HAL_StatusTypeDef Adxl345_GetAccelerations(Adxl345_AccelerationTypeDef *data);

void getXYZAxisAccelerationsThread();

#endif
