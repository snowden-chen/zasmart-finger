/*   Local udp client module in AI IoT . */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"

#include "zhengan.h"
#include "za_appext.h"

#include "za_info.h"
#include "za_json.h"

#include "za_aiot.h"
#include "za_aiot_udp.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

static const char *TAG = "ZA_AIOT_UDP";

typedef struct
{
    za_aiot_udp_type_t  tAiotUdpOpType;
    ZA_INT32 hSockHandle;
    struct sockaddr_in tDestAddrPort;
    ZA_INT32 ip_protocol;
    ZA_BOOL bIfHaveInited;
    ZA_BOOL bIfConnected;
    ZA_BOOL bLastIfConnected;
    ZA_BOOL b_aiot_udp_task_runflag;
    ZA_CHAR sServerIP[ZA_LOCALSERVER_IP];
    ZA_UINT16 nServerPort;
    ZA_QueueHandle_t h_aiot_udp_msg;
} za_aiot_udp_data_t;

static za_aiot_udp_data_t tCurrentDataContext;

static const char *sZA_UDP_URL[] = {
    "null",                                 /* ZA_AIOT_COM_NULL */
    "/zdmedia/command/",                     /* ZA_AIOT_COM_MAIN_SERVICE */
    "null",                                 /* ZA_AIOT_COM_UPDATE_TOUCH_KEY */
    "/zdmedia/event/",                      /* ZA_AIOT_COM_UPDATE_EVENT */
    "/zdmedia/information/",                /* ZA_AIOT_COM_UPDATE_SYSINFO */
    "/zdmedia/bindresult/",                 /* ZA_AIOT_COM_BIND_SERVICE */
    "/zdmedia/bindcode/",                   /* ZA_AIOT_COM_UPDATA_BINDCODE */
    "null",                                 /* ZA_AIOT_COM_TBD_DATA */
    "null",                                 /* ZA_AIOT_COM_TEST */
    "null"                                  /* ZA_AIOT_COM_END */
    };  // must be same as the za_aiot_com_type_t 

#define ZA_AIOTUDP_MSG_QUEUE_DEEPTH (8)

#define ZA_AIOT_UDP_TASK_STACKSIZE ZA_BUFFER_2K
#define ZA_AIOT_UDP_TASK_PRIORITY 5

void _za_aiot_udp_task(void *arg);

/* Just Only IPv4 Now.*/
ZA_BOOL za_aiot_udp_init(const ZA_CHAR * sUdpServAddr, const ZA_CHAR * sUserName, const ZA_CHAR * sPassWord)
{
    za_app_msg_data_t za_appmsg_data;

    za_memset(&tCurrentDataContext, 0, za_sizeof(za_aiot_udp_data_t));

    tCurrentDataContext.tDestAddrPort.sin_family = AF_INET;
    tCurrentDataContext.ip_protocol = IPPROTO_IP;
    tCurrentDataContext.tDestAddrPort.sin_addr.s_addr = inet_addr(za_aiot_local_get_server_ipv4(ZANULL));
    tCurrentDataContext.tDestAddrPort.sin_port = htons(za_aiot_local_get_server_port(ZANULL));
    za_strcpy(tCurrentDataContext.sServerIP, sUdpServAddr);
    tCurrentDataContext.nServerPort = za_aiot_local_get_server_port(ZANULL);

    tCurrentDataContext.hSockHandle 
    = socket(tCurrentDataContext.tDestAddrPort.sin_family, SOCK_DGRAM, tCurrentDataContext.ip_protocol);

    if (tCurrentDataContext.hSockHandle < 0)
    {
        ZA_ESP_LOGE(TAG, "Unable to create socket in za_aiot_udp_init: errno %d", errno);
        ZA_ESP_LOGE(TAG, "Shutting down socket and restarting...");
        
        shutdown(tCurrentDataContext.hSockHandle, 0);
        close(tCurrentDataContext.hSockHandle);

        return ZA_FALSE;
    }

    tCurrentDataContext.h_aiot_udp_msg = za_msg_queue_create(ZA_AIOTUDP_MSG_QUEUE_DEEPTH, za_sizeof(za_aiot_udp_msg_data_t));

    ZA_ESP_LOGI(TAG, "Socket created, sending to %s:%d", sUdpServAddr, tCurrentDataContext.nServerPort);

    tCurrentDataContext.b_aiot_udp_task_runflag = ZA_TRUE;

    za_task_create(_za_aiot_udp_task, "za_aiot_task", ZA_AIOT_UDP_TASK_STACKSIZE, NULL, ZA_AIOT_UDP_TASK_PRIORITY, NULL);

    tCurrentDataContext.bIfHaveInited = ZA_TRUE;
    tCurrentDataContext.bIfConnected = ZA_TRUE;

    za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_AIOTLOCAL_CONNECTED;

    if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(NULL), &za_appmsg_data, portMAX_DELAY))
    {
        ESP_LOGE(TAG, "za_msg_queue_send failure: %d", za_appmsg_data.za_appmsg_type);
    }

    tCurrentDataContext.bLastIfConnected = ZA_TRUE;

    return ZA_TRUE;
}

ZA_BOOL za_aiot_udp_term(ZA_VOID *pvParameter)
{
    (void) pvParameter;

    za_app_msg_data_t za_appmsg_data;

    if (tCurrentDataContext.bIfHaveInited && tCurrentDataContext.b_aiot_udp_task_runflag)
    {
        tCurrentDataContext.b_aiot_udp_task_runflag = ZA_FALSE;
        tCurrentDataContext.bIfConnected = ZA_FALSE;

        za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_AIOTLOCAL_DISCONNECTED;

        if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(NULL), &za_appmsg_data, portMAX_DELAY))
        {
            ESP_LOGE(TAG, "za_msg_queue_send failure: %d", za_appmsg_data.za_appmsg_type);
        }

        ZA_vTaskDelay(100 / ZA_portTICK_PERIOD_MS); // 1ms for switch

        tCurrentDataContext.bLastIfConnected = ZA_FALSE;

        shutdown(tCurrentDataContext.hSockHandle, 0);
        close(tCurrentDataContext.hSockHandle);
    }
    else
    {
        ZA_ESP_LOGE(TAG, "Unable to za_aiot_udp_term : init ? : %d , taskrun ? : %d .", 
        tCurrentDataContext.bIfHaveInited, tCurrentDataContext.b_aiot_udp_task_runflag);

        return ZA_FALSE;
    }

    za_memset(&tCurrentDataContext, 0, za_sizeof(za_aiot_udp_data_t));

    return ZA_TRUE;
}

ZA_QueueHandle_t za_get_aiot_udp_msg_handle(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   return tCurrentDataContext.h_aiot_udp_msg;
}

ZA_BOOL za_aiot_udp_push_data(za_aiot_com_data_t tAiotComData)
{
    ZA_CHAR aMacAddr[30] = {0};
    ZA_CHAR aTopic[150] = {0};
    ZA_CHAR *pTopicName = ZA_NULL;

    ZA_VOID *pStrData = ZA_NULL;
    ZA_UINT32 nStrLength = ZA_NULL;

    za_aiot_udp_msg_data_t za_msg_data;

    pTopicName = aTopic;

    if (! tCurrentDataContext.bIfHaveInited)
    {
        ZA_ESP_LOGE(TAG, "za_aiot_udp_push_data : udp client has not inited.");

        return ZA_FALSE;
    }

    if (! tCurrentDataContext.bIfConnected)
    {
        ZA_ESP_LOGE(TAG, "za_aiot_udp_push_data : udp client has not connected.");

        return ZA_FALSE;
    }

    za_info_get_mac_wifi_sta(aMacAddr, 20);

    strcpy(pTopicName, sZA_UDP_URL[tAiotComData.tZaAiotComType]);
    strcat(pTopicName, aMacAddr);

    pStrData = za_app_udp_package(pTopicName, tAiotComData.p_data, &nStrLength);

    ZA_INT32 err = sendto(tCurrentDataContext.hSockHandle, pStrData, nStrLength, 0, 
    (struct sockaddr *)&tCurrentDataContext.tDestAddrPort, sizeof(tCurrentDataContext.tDestAddrPort));
    if (err < 0) 
    {
        ZA_ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);

        return ZA_FALSE;
    }

    za_msg_data.za_aiotudp_type = ZA_AIOT_UDP_EVENT_WAIT;

    if(ZA_ESP_PASS != za_msg_queue_send(za_get_aiot_udp_msg_handle(ZA_NULL), &za_msg_data, portMAX_DELAY))
    {
        ZA_ESP_LOGI(TAG, "ZA_AIOT_UDP_EVENT_WAIT send msg failure. ---- > %d", za_msg_data.za_aiotudp_type);
    }

    ZA_ESP_LOGI(TAG, "za_aiot_udp_push_data : Message sent.");

    return ZA_TRUE;
}


/** Private **/


void _za_aiot_udp_task(void *arg)
{
    (void)arg;

    ZA_CHAR rx_buffer[ZA_SERVER_LEN];

    tCurrentDataContext.bIfConnected = ZA_FALSE;

    za_aiot_udp_msg_data_t za_msg_data;
    za_app_msg_data_t za_appmsg_data;

    ZA_UINT32 nUnConnectCount = 0;

    while (tCurrentDataContext.b_aiot_udp_task_runflag)
    {

        if (ZA_ESP_PASS != za_msg_queue_recv(tCurrentDataContext.h_aiot_udp_msg, &za_msg_data, ZA_portMAX_DELAY))
        {
            ZA_ESP_LOGE(TAG, "za_msg_queue_recv : tCurrentDataContext.h_aiot_udp_msg.");

            continue;
        }

        switch (za_msg_data.za_aiotudp_type)
        {
            case ZA_AIOT_UDP_EVENT_WAIT:
            {
                ;
            }
            break;

            default:
            {
                ;
            }
            break;
        }

        while(1) 
        {
            struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);

            ZA_INT32 len = recvfrom(tCurrentDataContext.hSockHandle, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            
            // Error occurred during receiving
            if (len < 0)
            {
                ZA_ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);

                nUnConnectCount ++;

                if (nUnConnectCount == 20) // how many times 
                {
                    tCurrentDataContext.bIfConnected = ZA_FALSE;

                    if (tCurrentDataContext.bLastIfConnected == ZA_TRUE)
                    {
                        za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_AIOTLOCAL_DISCONNECTED;

                        if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(NULL), &za_appmsg_data, portMAX_DELAY))
                        {
                            ESP_LOGE(TAG, "za_msg_queue_send failure: %d", za_appmsg_data.za_appmsg_type);
                        }

                        tCurrentDataContext.bLastIfConnected = ZA_FALSE;
                    }

                    nUnConnectCount = 0;
                }

                break;
            }
            // Data received
            else
            {
                tCurrentDataContext.bIfConnected = ZA_TRUE;

                if (tCurrentDataContext.bLastIfConnected == ZA_FALSE)
                {
                    za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_AIOTLOCAL_CONNECTED;

                    if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(NULL), &za_appmsg_data, portMAX_DELAY))
                    {
                        ESP_LOGE(TAG, "za_msg_queue_send failure: %d", za_appmsg_data.za_appmsg_type);
                    }

                    tCurrentDataContext.bLastIfConnected = ZA_TRUE;
                }

                nUnConnectCount = 0;

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string

                ZA_ESP_LOGI(TAG, "Received %d bytes from %s:", len, tCurrentDataContext.sServerIP);
                ZA_ESP_LOGI(TAG, "%s", rx_buffer);

                if (strncmp(rx_buffer, "ok", 2) == 0)
                {
                    ZA_ESP_LOGI(TAG, "Received expected message, rerecv now.");
                    break;
                }

                if (strncmp(rx_buffer, "bad", 3) == 0)
                {
                    ZA_ESP_LOGE(TAG, "Received json, but not the expected data, rerecv now.");
                    break;
                }

                if (strncmp(rx_buffer, "not", 3) == 0)
                {
                    ZA_ESP_LOGE(TAG, "Received not json format, check the data right, rerecv now.");
                    break;
                }
            }

            ZA_vTaskDelay(10 / ZA_portTICK_PERIOD_MS); // 1ms for switch
        }


        ZA_vTaskDelay(20 / ZA_portTICK_PERIOD_MS); // 1ms for switch
    }

    ZA_ESP_LOGI(TAG, "ZA AIOT UDP TASK Delete. ");

    vTaskDelete(NULL);

    return;
}