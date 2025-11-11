/*  All system info & auth or others being about system,chip,key etc . */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"

#include "za_info.h"

#include "esp_chip_info.h"
#include "esp_mac.h"

static const char *TAG = "ZA_INFO";

/** Public **/

ZA_BOOL za_info_monitor_init()
{
    return ZA_TRUE;
}

ZA_VOID za_info_monitor_sys(ZA_VOID *pvParameter)
{
    (void) pvParameter;

    esp_chip_info_t tEspChipInfo;
    ZA_UINT8 aMacAddr[8] = {0};

    esp_chip_info(&tEspChipInfo);

    ZA_ESP_LOGI(TAG, "[SYS] Free memory total: %lu bytes", esp_get_free_heap_size());
    ZA_ESP_LOGI(TAG, "[SYS] Free memory internal: %lu bytes", esp_get_free_internal_heap_size());
    ZA_ESP_LOGI(TAG, "[CPU] :model of chip : %d -- CHIP_ESP32[1] / CHIP_ESP32S2[2] / CHIP_ESP32C3[5] / CHIP_ESP32H2[6] / CHIP_ESP32S3[9]", tEspChipInfo.model);
    ZA_ESP_LOGI(TAG, "[CPU] :Revision of chip : %d ", tEspChipInfo.revision);
    ZA_ESP_LOGI(TAG, "[CPU] :number of cpu cores : %d ", tEspChipInfo.cores);

    esp_read_mac(aMacAddr, ESP_MAC_WIFI_STA);
    ZA_ESP_LOGI(TAG, "[MAC] : Wifi STA Mac Address : %02X:%02X:%02X:%02X:%02X:%02X", aMacAddr[0], aMacAddr[1], aMacAddr[2], aMacAddr[3], aMacAddr[4], aMacAddr[5]);

    esp_read_mac(aMacAddr, ESP_MAC_BT);
    ZA_ESP_LOGI(TAG, "[MAC] : BT Mac Address : %02X:%02X:%02X:%02X:%02X:%02X", aMacAddr[0], aMacAddr[1], aMacAddr[2], aMacAddr[3], aMacAddr[4], aMacAddr[5]);

    ZA_ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    return;
}

ZA_UINT32 za_info_get_free_memory(ZA_VOID *pvParameter)
{
    (void) pvParameter;

    return esp_get_free_heap_size();
}

ZA_VOID za_info_get_mac_wifi_sta(ZA_CHAR *pMacAddr, ZA_UINT32 nLength20more)
{
    ZA_UINT8 aMacAddr[8] = {0};

    if (pMacAddr == ZA_NULL || nLength20more < 19)
    {
        ZA_ESP_LOGE(TAG, "nParams error in za_info_get_mac_wifi_sta.");

        return;
    }

    esp_read_mac(aMacAddr, ESP_MAC_WIFI_STA);

    za_sprintf(pMacAddr, "%02X:%02X:%02X:%02X:%02X:%02X", aMacAddr[0], aMacAddr[1], aMacAddr[2], aMacAddr[3], aMacAddr[4], aMacAddr[5]);
    
    return;
}

ZA_VOID za_info_get_mac_bt(ZA_CHAR *pMacAddr, ZA_UINT32 nLength20more)
{
    ZA_UINT8 aMacAddr[8] = {0};

    if (pMacAddr == ZA_NULL || nLength20more < 19)
    {
        ZA_ESP_LOGE(TAG, "nParams error in za_info_get_mac_wifi_sta.");

        return;
    }

    esp_read_mac(aMacAddr, ESP_MAC_BT);

    za_sprintf(pMacAddr, "%02X:%02X:%02X:%02X:%02X:%02X", aMacAddr[0], aMacAddr[1], aMacAddr[2], aMacAddr[3], aMacAddr[4], aMacAddr[5]);
    
    return;
}