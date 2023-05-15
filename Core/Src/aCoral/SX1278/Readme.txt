目录结构：
SX1278
|-- include
|   |-- fifo.h
|   |-- lora.h
|   |-- platform.h
|   |-- radio.h
|   |-- spi.h
|   |-- sx1276.h
|   |-- sx1276Fsk.h
|   |-- sx1276FskMisc.h
|   |-- sx1276Hal.h
|   |-- sx1276LoRa.h
|   `-- sx1276LoRaMisc.h
`-- src
    |-- fifo.c
    |-- lora.c
    |-- radio.c
    |-- spi.c
    |-- sx1276.c
    |-- sx1276Fsk.c
    |-- sx1276FskMisc.c
    |-- sx1276Hal.c
    |-- sx1276LoRa.c
    `-- sx1276LoRaMisc.c
除了lora.h和lora.c为其他文件均为sx1276相关驱动文件（sx1278也适用）。

需要关注的文件：
	platform.h中设置芯片选型，当前项目设置为：#define USE_SX1276_RADIO。
	radio.h中设置芯片工作模式为LORA或者FSK，当前项目设置为#define LORA 1，选择为LORA模式。
	spi.h、spi.c中定义spi通信，需要将spi.c的SpiInOut（）函数中的spi句柄设置为主板的spi句柄，芯片和主板所有数据都通过spi进行传输。
	sx1276LoRa.c中主要关注SX1276LoRaGetRxPacket（）、SX1276LoRaSetTxPacket（）、SX1276LoRaProcess（）函数，分别用于lora模式下的数据包获取，数据包发送和接发调度。
	
	lora.h和lora.c为方便lora通信定义的文件

lora通信使用流程：
	1、需要定义lora消息接收发送用户数据缓冲区（Buffer）、lora操作指针（Radio）。
	2、在sx1278LoRa.c中设置LoRaSettings变量，对lora通信进行参数配置，LoRaSettings变量的类型定义于sx1276LoRa.h中。
	3、调用RadioDriverInit()函数对Radio进行初始化，此函数定义于radio.c中。
	4、调用Radio->Init(),进行lora通信相关初始化。
	5、完成上述初始化后，定义自己的接发服务函数。

	（这些均已在lora.h和lora.c中完成，用户调用lora_init()进行初始化后，可根据需求自行定义服务函数的接发逻辑）





