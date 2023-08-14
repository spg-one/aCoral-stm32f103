#ifndef __HC_SR04_H__
#define __HC_SR04_H__

#include "main.h"
#include "user.h"


#if defined(USE_BARE_BOARD)
#define __HcSr04_LOCK(__HANDLE__)                                               \
                            do{                                                 \
                                    if((__HANDLE__)->Lock == HcSr04_Locked)     \
                                    {                                           \
                                        return HAL_BUSY;                        \
                                    }                                           \
                                    else                                        \
                                    {                                           \
                                        (__HANDLE__)->Lock = HcSr04_Locked;   \
                                    }                                           \
                                }while (0U)

#define __HcSr04_UNLOCK(__HANDLE__)                                                 \
                                    do{                                             \
                                          (__HANDLE__)->Lock = HcSr04_UnLocked;     \
                                    }while (0U)

                                
typedef enum
{
    HcSr04_UnLocked = 0x00U,
    HcSr04_Locked   = 0x01U

} HcSr04_LockTypeDef;
#endif 

typedef enum
{
    HcSr04_STATE_RESET                  = 0x00U,    
    HcSr04_STATE_READY                  = 0x01U,    
    HcSr04_STATE_BUSY                   = 0x03U,    
    HcSr04_STATE_ERROR                  = 0x06U    
} HcSr04_StateTypeDef;

typedef enum 
{
    HcSr04_M_STATE_NONE             = 0x00U,
    HcSr04_M_STATE_IN_RANGE         = 0x01U,
    HcSr04_M_STATE_OVER_RANGE       = 0x02U,
    HcSr04_M_STATE_MEASURE_ERROR    = 0x03U
} HcSr04_MeasureStateTypeDef;

typedef struct
{
    float Distance;
    float Temperature;
    uint32_t TimerCount;
    uint8_t OverRange;
} HcSr04_DistanceTypeDef;

typedef struct
{
    uint8_t StartCapture;
    uint8_t Rising;
    uint8_t Falling;
    uint8_t CompleteCapture;
    uint8_t OverRange; 
    
} HcSr04_CCpatureFlagTypeDef;

typedef struct
{
    uint8_t MeasureTimeGap;         /* !< The time gap of measure distance: ms ; > 200ms*/
    uint8_t TrigPinHighLevelTime;   /* !< The time of that the trig pin high level should keep: us ; >15us ; <25us */
    float MaximumCapacity;          /* !< The max distance that hc_sr04 can measure: cm ; <=600cm */
    
} HcSr04_InitTypeDef;

typedef struct
{
    GPIO_TypeDef                    *TrigGPIOGroup;
    uint16_t                        TrigGPIOPin;
    TIM_HandleTypeDef               *EchoPeripheralHandle;    /* !< The Advanced Timer peripheral handle which used by hc_sr04 measuring distance;\ */
#if defined(USE_BARE_BOARD)
    TIM_HandleTypeDef               *TrigPeripheral2Handle;   /* !< The General Purpose peripheral handle which used to time when not use OS */
    uint8_t                         TimingIsOK;
    HcSr04_LockTypeDef              Lock; 
#else
    uint8_t                         Lock;           /* When use OS, this need modified */
#endif
    HcSr04_InitTypeDef              Init;
    HcSr04_StateTypeDef             State;
    uint32_t                        PreMeasureTick;     /* !< The previous measure distance time: ms*/
    HcSr04_MeasureStateTypeDef      PreMeasureState;    
    HcSr04_CCpatureFlagTypeDef      *CCaptureFlag;
    HcSr04_DistanceTypeDef          *CMeasureData;
} HcSr04_HandleTypeDef;



// The function for initializing hc_sr04 device
HAL_StatusTypeDef HcSr04_Init(void);

// The function for reading distance by hc_sr04
HAL_StatusTypeDef HcSr04_GetDistance(HcSr04_DistanceTypeDef *data);



void get_distance_thread();


#endif 
