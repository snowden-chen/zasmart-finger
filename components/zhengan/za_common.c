/*   za common base file . */

/*
   NOTE: Writed By ZachSnowdenChen
*/

#include "za_common.h"

static const char *ZADIYURL = "https://ff3nod.zdmedia.com.cn/cza/zadiy/app/";

ZA_SemaphoreHandle_t _za_semaphore_create( ZA_INT32 ux_max_count, ZA_INT32 ux_initial_count )
{
    ZA_SemaphoreHandle_t h_semaphore_handle = NULL;
    do
    {
        if(ux_max_count < ux_initial_count)
        {
            break;
        }

        if(1 < ux_max_count)
        {
            h_semaphore_handle =  (ZA_SemaphoreHandle_t)xSemaphoreCreateCounting(ux_max_count,ux_initial_count);
            break;
        }

        h_semaphore_handle =  (ZA_SemaphoreHandle_t)xSemaphoreCreateBinary();
        if(!h_semaphore_handle)
        {
            break;
        }

        if(ux_initial_count == ux_max_count)
        {
            xSemaphoreGive(h_semaphore_handle);
        }

    }
    while(0);
    return h_semaphore_handle;
}

ZA_BOOL _za_semaphore_request(ZA_SemaphoreHandle_t h_semaphore, ZA_INT32 n_time_out)
{
    if(!h_semaphore)
    {
        return pdFAIL;
    }
    return xSemaphoreTake(h_semaphore,n_time_out);
}

ZA_BOOL _za_semaphore_release(ZA_SemaphoreHandle_t h_semaphore)
{
    if(!h_semaphore)
    {
        return pdFAIL;
    }
    return xSemaphoreGive(h_semaphore);
}


ZA_VOID za_debug()
{
    #define ZA_ESP_DEBUG_LEVEL  ESP_LOG_ERROR
    #define ZA_ESP_DEBUG_INFO_LEVEL  ESP_LOG_INFO
    //#define ZA_ESP_DEBUG_LEVEL  ESP_LOG_DEBUG

    //esp_log_level_set("*", ESP_LOG_ERROR);
    esp_log_level_set("*", ESP_LOG_DEBUG);

    esp_log_level_set("ZA_MAIN", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZHENGAN", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_BLUFI", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_STORAGE", ZA_ESP_DEBUG_INFO_LEVEL);
    esp_log_level_set("ZA_WIFI", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_INFO", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_CONSOLE", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_COMMAND", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_INNERSOR", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_SYSTEM", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_UPGRADE", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_INFO", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_JSON", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_AIOT", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_APPEXT", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_AIOT_MQTT", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_AIOT_MQTTS", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_AIOT_MQTTLOCAL", ZA_ESP_DEBUG_LEVEL);
    esp_log_level_set("ZA_AIOSM", ESP_LOG_INFO);

    return;
}

ZA_UINT32 za_common_first_before_place(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput)
{
    ZA_UINT32 nLen = 0;
    ZA_CHAR *pStr = strchr(pcStrInput,' ');

    if (pStr != NULL)
    {
        nLen = pStr - pcStrInput + 1;

        strncpy(pcStrOutput, pcStrInput, nLen-1);

        pcStrOutput[nLen-1] = '\0';

        return nLen;
    }

    return 0;
}

ZA_UINT32 za_common_middle_value_between(const ZA_CHAR *pcStrInput, const ZA_CHAR *pcStrStart, const ZA_CHAR *pcStrEnd, ZA_CHAR *pcStrOutput)
{
    ZA_UINT32 nLen = 0;
    ZA_CHAR* pStrS = strstr(pcStrInput, pcStrStart);

    if (pStrS == NULL)
    {
        return 0;
    }

    pStrS += strlen(pcStrStart);

    ZA_CHAR* pStrE = strstr(pStrS, pcStrEnd);

    if (pStrE == NULL)
    {
        return 0;
    }

    nLen = pStrE - pStrS;

    strncpy(pcStrOutput, pStrS, nLen);

    pcStrOutput[nLen] = '\0';

    return nLen;
}

ZA_UINT32 za_common_get_bind_id_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput)
{
    ZA_UINT32 nLen = 0;

    nLen = za_common_first_before_place(pcStrInput, pcStrOutput);

    return nLen;
}

ZA_UINT32 za_common_get_localmqtt_url_from_string_only(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput)
{
    ZA_UINT32 nLen = 0;

    nLen = za_common_middle_value_between(pcStrInput, "h ", " end", pcStrOutput);

    return nLen;
}

ZA_UINT32 za_common_get_localmqtt_url_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput)
{
    ZA_UINT32 nLen = 0;

    nLen = za_common_middle_value_between(pcStrInput, "MQTT-Server h ", " u ", pcStrOutput);

    return nLen;
}

ZA_UINT32 za_common_get_localmqtt_uname_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput)
{
    ZA_UINT32 nLen = 0;

    nLen = za_common_middle_value_between(pcStrInput, " u ", " p ", pcStrOutput);

    return nLen;
}

ZA_UINT32 za_common_get_localmqtt_upasswd_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput)
{
    ZA_UINT32 nLen = 0;

    nLen = za_common_middle_value_between(pcStrInput, " p ", " end", pcStrOutput);

    return nLen;
}

ZA_UINT32 za_common_get_local_udpurl_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput)
{
    ZA_UINT32 nLen = 0;

    nLen = za_common_middle_value_between(pcStrInput, "UDP-Server ", " end", pcStrOutput);

    return nLen;
}

ZA_UINT32 za_common_get_local_udpipv4_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput)
{
    ZA_UINT32 nLen = 0;

    nLen = za_common_middle_value_between(pcStrInput, "UDP-Server udp:", "end", pcStrOutput);

    return nLen;
}

ZA_UINT32 za_common_get_local_udpport_from_string(const ZA_CHAR *pcStrInput, ZA_UINT32 *pnPort)
{
    ZA_CHAR aUrl[128] = {0};
    ZA_CHAR aPort[8] = {0};
    ZA_UINT32 nLen = 0;

    nLen = za_common_middle_value_between(pcStrInput, "UDP-Server udp:", "end", aUrl);

    if (nLen == 0)
    {
        *pnPort = 0;

        return 0;
    }

    nLen = za_common_middle_value_between(pcStrInput, ":", " ", aPort);

    if (nLen == 0)
    {
        *pnPort = 0;

        return 0;
    }

    sscanf(aPort, "%x", pnPort);

    return nLen;
}

ZA_UINT32 za_common_get_zadiy_bin_path(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput, ZA_UINT32 nOutLenMax)
{
    ZA_UINT32 nLen = 0;

    if ((pcStrInput == NULL) || (pcStrOutput == NULL) || (nOutLenMax == 0))
    {
        return nLen;
    }

    if (nOutLenMax <= (strlen(ZADIYURL)+(2*strlen(pcStrInput))+6))
    {
        return nLen;
    }

    za_memset(pcStrOutput, 0, za_sizeof(ZA_CHAR)*nOutLenMax);

    strcpy(pcStrOutput, ZADIYURL);
    strcat(pcStrOutput, pcStrInput);
    strcat(pcStrOutput, "/");
    strcat(pcStrOutput, pcStrInput);
    strcat(pcStrOutput, ".bin");

    nLen = strlen(pcStrOutput);

    return nLen;
}