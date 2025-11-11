/* AIO Sensor Module . */

/** IO26 -- SDA / IO25 -- SCL May not be others uses . */ 

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"
#include "za_command.h"

#include "zhengan.h"

#include "za_storage.h"

#include "za_aiot.h"
#include "za_aiosm.h"

static const char *TAG = "ZA_AIOSM";

#define ZA_AIOSM_OID_TASK_STACKSIZE ZA_BUFFER_4K
#define ZA_AIOSM_OID_TASK_PRIORITY 5

int dpr_close[3] = {0xE2F, 0x02,0x01};						//oid3 & oid2 enable,the default is oid3;ack =  0x700061
int dpr_open[3] = {0xE2F,0x07,0x02};
int s2_all[3] = {0x0e36, 0x000F, 0x0000};					//s2 all output
int get_version[3] = {0x03AA ,0x0000 ,0x0000};				//A415X ack = 0;
int set_dpr[3] = {0x0e34,0x0002,0x0000};
/*
MCU → A415X：設定DPR3Lt專用碼 (Aicademy)
1’s word: 0x0e34
2’s word: 0x0002
3’s word: null
*/

int cali_cmd_415[3] = {0x0e30,0xc80,0xc80};					//自动校正参数
int cali_cmd_restart_415[3] = {0x0e33,0x0000,0x0000};

int DPR_version = 2;
int DPR_A415 = 2;
int DRP_A413 = 1;

aio_t aio_data;

static void aio_to_index(aio_t *data);

/** private  **/

static void delay_ms(uint16_t uidelay)					
{
	//vTaskDelay(uidelay / portTICK_RATE_MS);
	vTaskDelay(ZA_pdMS_TO_TICKS(uidelay));
}

void aio_set_io_input(int gpio)
{
	gpio_set_direction(gpio,GPIO_MODE_INPUT);
}

//==========================================================//aio set io output mode
void aio_set_io_output(int gpio)
{
	gpio_set_direction(gpio,GPIO_MODE_OUTPUT);
}

//==========================================================//aios set gpio stat
void aio_set_io_level(int gpio,bool level)
{
	uint32_t stat;

	if(level)
	{
		stat = 1;
	}
	else 
	{
		stat = 0;
	}

	gpio_set_level(gpio,stat);
}

//==========================================================//aio get io stat
int aio_read_io_level(int gpio)
{
	int level;

	level = gpio_get_level(gpio);
	
	return level;
}

//==========================================================//aio gpio init

static void aio_gpio_init(void)
{
	//==========================================================//set scl output

	/* // don't use by zach snowden
	gpio_config_t g_conf;

	g_conf.intr_type = GPIO_INTR_DISABLE;
	g_conf.mode = GPIO_MODE_OUTPUT;
	g_conf.pin_bit_mask = AIO_SCL_SEL;
	g_conf.pull_down_en = 1;
	g_conf.pull_up_en = 0;
	gpio_config(&g_conf);

	gpio_set_level(AIO_SCL,GPIO_LOW);							//set scl low

	g_conf.pull_down_en = 0;
	g_conf.pull_up_en = 1;
	g_conf.pin_bit_mask = AIO_SDA_SEL;
	g_conf.mode = GPIO_MODE_INPUT;
	gpio_config(&g_conf);										//set sda input
    */

    gpio_reset_pin(AIO_SCL);
    gpio_reset_pin(AIO_SDA);

	gpio_set_direction(AIO_SCL, GPIO_MODE_OUTPUT);
    gpio_set_direction(AIO_SDA, GPIO_MODE_INPUT_OUTPUT);

	gpio_set_level(AIO_SCL,GPIO_LOW);							//set scl low

   return;
}

//====================================================================//aio read sda
int aio_read_sda(void)
{
	return aio_read_io_level(AIO_SDA);
}

//======================================================================//aio read data;
int aio_read(aio_t *buf)
{
	uint8_t i,j;
	uint8_t aio_len = 0;
	uint64_t num = 0;
	int len = 0;
	
	if(aio_read_sda()==1)
	{
		return 0;
	}

	buf->stat = true;
	buf->header = 0;
	buf->reserver = 0;
	buf->data = 0;
	buf->index = 0;
   	aio_set_io_output(AIO_SCL);
	aio_set_io_output(AIO_SDA);
	aio_set_io_level(AIO_SCL,GPIO_HIGH);
	aio_set_io_level(AIO_SDA,GPIO_LOW);

	usleep(3);

	aio_set_io_level(AIO_SCL,GPIO_LOW);

	usleep(3);

	aio_set_io_level(AIO_SCL,GPIO_HIGH);
	aio_set_io_input(AIO_SDA);

	for(i=0;i<7;i++)
	{
		aio_set_io_level(AIO_SCL,GPIO_HIGH);

		buf->header = buf->header << 1;
		num = num << 1;

		usleep(5);

		aio_set_io_level(AIO_SCL,GPIO_LOW);

		if(aio_read_io_level(AIO_SDA))
		{
			buf->header++;
			num++;
		}

		usleep(5);
	}

	if(buf->header >= 0x61 && buf->header <= 0x63)						//43bit
	{
		for(i=0;i<6;i++)
		{
			aio_set_io_level(AIO_SCL,GPIO_HIGH);

			buf->reserver = buf->reserver << 1;
			num = num << 1;

			usleep(5);

			aio_set_io_level(AIO_SCL,GPIO_LOW);

			if(aio_read_io_level(AIO_SDA))
			{
				buf->reserver++;
				num++;
			}

			usleep(5);
		}

		len = 43;
		aio_len = 30;
	}
	else if(buf->header == 0x69)									//pen list signal
	{
		aio_len = 9;
		len =  16;
	}
	else if(buf->header == 0x70)									//cmd ack
	{
		aio_len = 16;
		len = 23;
	}
	else
	{
		aio_len = 16;
		len = 23;
	}

	i=j=0;

	while(1)
	{
		aio_set_io_level(AIO_SCL,GPIO_HIGH);

		buf->data = buf->data << 1;
		num=num<<1;

		usleep(5);

		aio_set_io_level(AIO_SCL,GPIO_LOW);

		if(aio_read_io_level(AIO_SDA))
		{
			buf->data++;
			num++;
		}

		i++;

		if(i>=8)
		{
			i=0;
			j++;
		}

		usleep(5);

		aio_len--;

		if(aio_len == 0)
		{
			usleep(80);
			break;
		}
	}

	aio_to_index(buf);

	return len;
}

//=============================================================================================//write cmd
void aio_write_cmd(int cmd)
{
	//int i = 0;
	int j = 0;
	int  iTemp;
	ESP_LOGI(TAG,"aio_cmd 1");

	if(aio_read_sda()==0)
	{
		aio_read(&aio_data);
	}

	aio_set_io_output(AIO_SDA);
	aio_set_io_level(AIO_SCL,1);
	aio_set_io_level(AIO_SDA,1);

	usleep(5);

	aio_set_io_level(AIO_SCL,0);

	usleep(5);
	
	iTemp = cmd;

	for(j=0;j<8;j++)
	{
		aio_set_io_level(AIO_SCL,1);

		if(iTemp & 0x80)
		{
			aio_set_io_level(AIO_SDA, 1);
		}
		else
		{
			aio_set_io_level(AIO_SDA, 0);
		}

		iTemp = iTemp << 1;

		usleep(5);

		aio_set_io_level(AIO_SCL, 0);

		usleep(5);
	}

	aio_set_io_input(AIO_SDA);

	usleep(78);

	ESP_LOGI(TAG,"write the cmd :0x%x suc",cmd);
}

//=============================================================================================//write buffer 

void aio_write_data(int *buffer,int len)
{
	int i = 0;
	int j = 0;
	int  iTemp;

	ESP_LOGI(TAG,"aio init end,write cmd buf is 0x%04x   0x%04x   0x%04x",(ZA_UINT16)(buffer[0]),(ZA_UINT16)(buffer[1]),(ZA_UINT16)(buffer[2]));

	if(aio_read_sda()==0)
	{
		aio_read(&aio_data);
	}

	aio_set_io_output(AIO_SDA);
	aio_set_io_level(AIO_SCL,1);
	aio_set_io_level(AIO_SDA,1);

	usleep(5);

	aio_set_io_level(AIO_SCL,0);

	usleep(5);
	
	for( i=0; i<len; i++)
	{
		iTemp = buffer[i];

		for(j=0;j<16;j++)
		{
			aio_set_io_level(AIO_SCL,1);

			if(iTemp & 0x8000)
			{
				aio_set_io_level(AIO_SDA, 1);
			}
			else
			{
				aio_set_io_level(AIO_SDA, 0);
			}

			iTemp = iTemp << 1;

			usleep(5);

			aio_set_io_level(AIO_SCL, 0);

			usleep(5);
		}
	}

	aio_set_io_input(AIO_SDA);

	usleep(78);

	i = 200;

	while(i)
	{
		delay_ms(10);

		if(aio_read_sda()==0)
		{
			aio_read(&aio_data);
			break;
		}
		
		i--;
	}
}
//==========================================================================================//calibration

//aio module init
ZA_INT32 aio_init(void)
{
	ZA_ESP_LOGI(TAG,"aio init start");
	
	aio_gpio_init();

	vTaskDelay(ZA_pdMS_TO_TICKS(10));

	uint16_t delay_times = 1000;
	
	aio_set_io_level(AIO_SCL,GPIO_HIGH);
	
	//delay_ms(50);
	vTaskDelay(ZA_pdMS_TO_TICKS(50));
	
	aio_set_io_level(AIO_SCL,GPIO_LOW);
	
	while(delay_times)
	{
		delay_times--;
		
		delay_ms(10);
		
		if(aio_read_sda() == 0)
		{
			ZA_ESP_LOGI(TAG,"aio_init:aio read!");
			
			aio_read(&aio_data);
			
			break;
		}

		if(delay_times == 0)
		{
			ZA_ESP_LOGE(TAG,"aio init failed!!!");
		}
	}
	
	ZA_ESP_LOGI(TAG,"aio_init:aio read! head is 0x%x ,data is 0x%x ",(ZA_UINT32)(aio_data.header),(ZA_UINT32)(aio_data.data));
	
	if(aio_data.header == 0x60)
	{
		if(aio_data.data== 0xFFF8)		
		{
			ZA_ESP_LOGW(TAG,"AIO pen INIT SUC!!!");
			
			aio_write_data(dpr_open,3);			
			
			return 0;
		}
		else
		{
			ZA_ESP_LOGE(TAG,"AIO Pen INIT FAILED!!!");
			
			return 1;
		}
	}
	
	return 2;
}

void _za_aiosm_oid_task(void *arg)
{
   	(void)arg;

   	aio_t aio_data;

	ZA_BOOL appmsg_run_flag = ZA_TRUE;

	//za_app_msg_data_t za_appmsg_data;

	while(appmsg_run_flag)
	{
		aio_read(&aio_data);

		if (aio_data.stat)
		{
			aio_data.stat = false;

			if(aio_data.header == 0x62)
			{ 	
				ZA_ESP_LOGI(TAG,"get the oid3 code:%d;0x%x\n",(ZA_UINT32)(aio_data.index),(ZA_UINT32)(aio_data.index));
				//OID3 index aio_data.index
				/*
				//(event)
				if ((aio_data.index != 0) && (za_aiot_connect_status(ZA_NULL)))
				{
					za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_AIOT_SEND_OID;

					za_sprintf(za_appmsg_data.cBuffer, "%d", (ZA_UINT32)(aio_data.index));

					if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(NULL), &za_appmsg_data, portMAX_DELAY))
					{
						ZA_ESP_LOGE(TAG, "_za_aiosm_oid_task send msg failure. ---- > %d", za_appmsg_data.za_appmsg_type);
					}

					memset(za_appmsg_data.cBuffer, 0, sizeof(ZA_CHAR)*ZHENGAN_APP_MAG_DATABUF_LEN);
				
				}
				*/
			}
			else if(aio_data.header == 0x61)
			{
				ZA_ESP_LOGI(TAG,"get the oid2 code:%d\n",(ZA_UINT32)(aio_data.index));
				//OID2 Index aio_data.index

				// This driver can not support oid2. by Zach Snowden Chen
			}
			else if(aio_data.header == 0x63)
			{
				ZA_ESP_LOGI(TAG,"get the drp code:%d\n",(ZA_UINT32)(aio_data.index));
				//dpr index aio_data.index
			}
			else if(aio_data.header == 0x60)		
			{
				//cmd 
				if(aio_data.data == 0xFFF1)		//reset cmd
				{
					aio_write_data(dpr_close,3);			
				}
			}

		}

		ZA_vTaskDelay(10 / ZA_portTICK_PERIOD_MS); // 15ms for test
	}


	return;
}


//aio data change to aio index

static void aio_to_index(aio_t *data)
{
	//uint32_t data_pos = 0;
	//uint32_t g_aio_index = 0;
	//uint32_t data_tmp = 0;
	//int res;
	if(data->header == 0x61)														//to s2 index,mapping file to mapping index
	{
		/*
		FILE *file;		
		printf("start getting the true index number from mapping.bin file\n");
		file = fopen("/sdcard/System/mapping.bin","r");
		if(file != NULL) 
		{
			data_pos = data->data * 2;
			res = fseek(file,data_pos,0);
			if (res == -1)
			{
				printf("mapping.bin seek the pos %d byte failed\r\n",data_pos);
				fclose(file);
				return;
			}
			res = fread(&g_aio_index,2,1,file);
			if(res == -1)
			{
				printf("mapping.bin read the pos %d byte failed\r\n",data_pos);
				fclose(file);
				return;
			}
			data_tmp = g_aio_index;
			data_tmp = (data_tmp & 0x0000FF00)>>8;										//bit & little Endian
			g_aio_index = (g_aio_index & 0x000000FF)<<8;
			g_aio_index = g_aio_index | data_tmp;
			printf("get the true index number is %d\r\n",g_aio_index);
			data->index = g_aio_index;
		}
		else
		{
			printf("Open the mapping.bin file failed\r\n");
		}
		fclose(file);
		*/

		ZA_ESP_LOGE(TAG, "aio_to_index error: can not support --> %d", (ZA_UINT32)(data->data));
	}
	else if(data->header == 0x62)														//to s3 index
	{
		data->index = data->data;
	}
	else if(data->header == 0x63)
	{
		data->index = data->data;
	}
}


/** public **/

ZA_BOOL za_aiosm_init()
{
	/*
	za_storage_sys_mode_t tSystemMode = za_storage_get_system_mode(NULL);

	if (tSystemMode.tOutmoduleType != ZA_STORAGE_OUT_MARK_DIY)
	{
		return ZA_TRUE;
	}
	*/

   	ZA_ESP_LOGI(TAG, "ZA AIOSM ENTER");

	aio_init();
	
	if(! za_aiosm_calculate_start())
	{
		ZA_ESP_LOGE(TAG, "za_aiosm_calculate_start error");

		return ZA_FALSE;
	}

	//za_task_create(_za_aiosm_oid_task, "za_aiosm_oid_task", ZA_AIOSM_OID_TASK_STACKSIZE, NULL, ZA_AIOSM_OID_TASK_PRIORITY, NULL);

	return ZA_TRUE;
}

ZA_BOOL za_aiosm_calculate_start()
{
	ZA_ESP_LOGI(TAG, "za_aiosm_calculate_start.");

	ZA_UINT32 Param1 = 0;
	ZA_UINT32 Param2 = 0;

	Param1 = za_storage_get_calibra_param1(NULL);
	Param2 = za_storage_get_calibra_param2(NULL);

	if (Param1 != 0)
	{ 
		cali_cmd_415[1] = Param1;
	}

	if (Param2 != 0)
	{
		cali_cmd_415[2] = Param2;
	}
	
	aio_gpio_init();

	uint16_t delay_times = 1000;
	
	aio_set_io_level(AIO_SCL,GPIO_HIGH);
	
	delay_ms(50);
	
	aio_set_io_level(AIO_SCL,GPIO_LOW);
	
	
	while(delay_times)
	{
		delay_times--;
		
		delay_ms(10);
		
		if(aio_read_sda() == 0)
		{
			ZA_ESP_LOGI(TAG,"aio_init:aio read!");
			
			aio_read(&aio_data);
			
			break;
		}

		if(delay_times == 0)
		{
			ZA_ESP_LOGE(TAG,"aio init failed!!!---");
			delay_ms(200);//by zach.snowden.chen
		}
	}
	

	//delay_ms(200); //temp test . by zach.snowden.chen

	ZA_ESP_LOGI(TAG,"za_aiosm_calculate_startt: -----> head is 0x%x ,data is 0x%x ",(ZA_UINT32)(aio_data.header),(ZA_UINT32)(aio_data.data));

	if(aio_data.header == 0x60)
	{
		if(aio_data.data== 0xFFF8)		
		{
			
			ZA_ESP_LOGI(TAG,"AIO pen INIT SUC!!!");
			
			aio_write_data(cali_cmd_415,3);
			ZA_ESP_LOGI(TAG,"输出ack的值0x%x",aio_data.header);

			delay_ms(2);

			aio_write_data(cali_cmd_restart_415,3);
			ZA_ESP_LOGI(TAG,"restart 输出ack的值0x%x",aio_data.header);

			delay_ms(2);
			

			//aio_write_data(dpr_open,3);	
			
			return ZA_TRUE;
		}
		else
		{
			ZA_ESP_LOGE(TAG,"AIO Pen INIT FAILED!!!");
			
			return ZA_FALSE;
		}
	}

	if (aio_data.header == 0x7F) //by zach snowden chen
	{
		if(aio_data.data== 0xFFFF)
		{
			
			ZA_ESP_LOGI(TAG,"AIO pen INIT SUC!!!");
			
			aio_write_data(cali_cmd_415,3);
			ZA_ESP_LOGI(TAG,"输出ack的值0x%x",aio_data.header);

			delay_ms(2);

			aio_write_data(cali_cmd_restart_415,3);
			ZA_ESP_LOGI(TAG,"restart 输出ack的值0x%x",aio_data.header);

			delay_ms(2);
			

			//aio_write_data(dpr_open,3);	
			
			return ZA_TRUE;
		}
		else
		{
			ZA_ESP_LOGE(TAG,"AIO Pen INIT FAILED!!!");
			
			return ZA_FALSE;
		}
	}

	return ZA_TRUE;
}
