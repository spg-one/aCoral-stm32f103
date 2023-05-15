#include <stdint.h>
#include "spi.h"
#include "main.h"

extern SPI_HandleTypeDef hspi2;
uint8_t SpiInOut( uint8_t outData )
{

	uint8_t pData = 0;

  if(HAL_SPI_TransmitReceive(&hspi2,&outData,&pData,1,0xffff) != HAL_OK)
	{
		return ERROR;
	}    
  else
	{   

		return pData;
	}	
}

