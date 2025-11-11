/* za_aiosm.h , AIO Sensor Module . */

/* Version : V0.1 */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_AIOSM_H
#define ZA_AIOSM_H

#ifdef __cplusplus
extern "C" {
#endif

#define AIO_SCL 		GPIO_NUM_7				//scl io num
//#define AIO_SCL 		GPIO_NUM_6				//scl io num
//#define AIO_SCL_SEL		GPIO_SEL_6
#define AIO_SDA			GPIO_NUM_6				//sda io num
//#define AIO_SDA			GPIO_NUM_5				//sda io num
//#define AIO_SDA_SEL		GPIO_SEL_5

#define GPIO_HIGH		1
#define GPIO_LOW		0

typedef struct aio
{
	char header;											//aio data 7bit header
	char reserver;											//aio data 6bit reserver
	uint32_t data;											//aio data 30bit  index
	uint32_t index;											//aio s data 30bit index
	bool stat;
}aio_t;

ZA_BOOL za_aiosm_init();

ZA_BOOL za_aiosm_calculate_start();


#ifdef __cplusplus
}
#endif

#endif