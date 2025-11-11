/* za_common.h , common base head file. */

/* Version : V0.1 */

/*
   NOTE: Writed By ZachSnowdenChen
*/

#ifndef ZA_COMMON_H
#define ZA_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#include <sys/stat.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "esp_task_wdt.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_random.h"
#include "esp_chip_info.h"

#include "esp_private/system_internal.h"

#include "driver/gpio.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi.h"

//#include "esp_spiffs.h"

#include "string.h"

#define PRODUCT_VERSION "%d.%d.%d.%d(%d)"

#define ZA_BUFFER_1K    (0x400)
#define ZA_BUFFER_2K    (0x800)
#define ZA_BUFFER_3K    (0xc00)
#define ZA_BUFFER_4K    (0x1000)
#define ZA_BUFFER_5K    (0x1400)
#define ZA_BUFFER_6K    (0x1800)
#define ZA_BUFFER_7K    (0x1c00)
#define ZA_BUFFER_8K    (0x2000)
#define ZA_BUFFER_12K   (0x3000)
#define ZA_BUFFER_16K   (0x4000)
#define ZA_BUFFER_32K   (0x8000)
#define ZA_BUFFER_64K   (0x10000)
#define ZA_BUFFER_128K  (0x20000)
#define ZA_BUFFER_1M    (0x100000)
#define ZA_BUFFER_8M    (0x800000)

#define ZA_UDP_PKG_LEN            (512 - 1)
#define ZA_LONG_HELP_PKG_LEN      (ZA_BUFFER_2K - 1)
#define ZA_HELP_PKG_LEN           (ZA_BUFFER_1K - 1)
#define ZA_SOFT_VERSION_LEN       (16)
#define ZA_SN_LEN                 (16)
#define ZA_PN_LEN                 (17)
#define ZA_MBL_LEN                (16)
#define ZA_MAC_LEN                (18)
#define ZA_KEY_LEN                (16)
#define ZA_CDN_ID_LEN             (8)
#define ZA_SQL_ID_LEN             (8)
#define ZA_VERDOR_LEN             (8)
#define ZA_PLATFORM_LEN           (40)
#define ZA_PASSWORD_LEN           (48)
#define ZA_USER_LEN               (18)
#define ZA_TOKEN_LEN              (512)
#define ZA_APP_IDENT_LEN          (40)
#define ZA_OPEN_ID_LEN            (16)
#define ZA_OPEN_CODE_LEN          (8)
#define ZA_SERVER_LEN             (128)
#define ZA_RESPONSE_LEN           (32)
#define ZA_URL_LEN                (256)
#define ZA_LOCAL_NAME_LEN         (128)
#define ZA_DATE_TIME_LEN          (32)
#define ZA_MD5_LEN                (32)
#define ZA_VERSION_LEN            (32)
#define ZA_ETAG_LEN               (64)
#define ZA_VCODE_LEN              (32)
#define ZA_PAIR_NAME_LEN          (32)
#define ZA_SHORT_TOKEN_LEN        (48)
#define ZA_MODIFIED_LEN           (64)
#define ZA_HEARTBEATKEY_LEN       (24)
#define ZA_SSID_ID_LEN            (32)
#define ZA_SSID_PWD_LEN           (64)
#define ZA_FALSH_KEY_LEN          (16)
#define ZA_NET_TRANS_TIME_OUT     (5000) //ms
#define ZA_URL_NAME_LEN           (ZA_URL_LEN)
#define ZA_PAIR_VALUE_LEN         (ZA_URL_LEN)
#define ZA_TASK_INFO_LENGTH       (16)
#define ZA_SP_KEY_LEN             (64)
#define ZA_SN_CERT_LEN            (64)
#define ZA_VER_CODE_LEN           (16) //Verification code len
#define ZA_OID_LEN                (16)
#define ZA_SHORT_URL_LEN          (64)
#define ZA_BAND_CODE_LEN          (8)
#define ZA_SYS_DATA_BUF_LEN       (128)
#define ZA_LONGMORE_CMD_HEAD_LEN  (4)
#define ZA_LOCALSERVER_IP         (24)

#define ZA_APPEXT_MAX_DATA_LEN 500
#define ZA_APPEXT_NORMAL_DATA_LEN 200

//typedef void *  semaphore_handle_t;

#define ZA_UNUSED(x)  ((void)x)

#define ZA_STORAGE_LOCALSTORAGE_PATH    "LOCALSTORAGE"
#define ZA_STORAGE_SYSTEM_MODE_KEY  	"SYSMODE"
#define ZA_STORAGE_CMDSTORE_MODE_KEY  	"CMDSTOREMODE"
#define ZA_STORAGE_LOCAL_SERVER_KEY     "LOCALSERVERKEY"

/*
* Define something used in freertos env.
*/

#define ZA_SemaphoreHandle_t SemaphoreHandle_t
ZA_SemaphoreHandle_t _za_semaphore_create( int ux_max_count, int ux_initial_count );
bool _za_semaphore_request(ZA_SemaphoreHandle_t h_semaphore, int n_time_out);
bool _za_semaphore_release(ZA_SemaphoreHandle_t h_semaphore);

#define za_semaphore_create  _za_semaphore_create
#define za_semaphore_request _za_semaphore_request
#define za_semaphore_release _za_semaphore_release
#define za_semaphore_count   uxSemaphoreGetCount
#define za_semaphore_delete(h_semaphore){\
        if(NULL != h_semaphore){vSemaphoreDelete(h_semaphore);h_semaphore = NULL;}}
#define za_semaphore_create_mutex xSemaphoreCreateMutex

#define za_msg_queue_create             xQueueCreate
#define za_msg_queue_delete             vQueueDelete

//#define za_msg_queue_send               xQueueSendToBack
#define za_msg_queue_send               xQueueSendToFront

#define za_msg_queue_recv               xQueueReceive
#define ZA_QueueHandle_t                QueueHandle_t

#define za_timer_create                 xTimerCreate
#define za_timer_start                  xTimerStart
#define za_timer_stop                   xTimerStop
#define za_timer_reset                  xTimerReset
#define za_timer_delete                 xTimerDelete
#define za_timer_change_period          xTimerChangePeriod
#define ZA_TimerHandle_t                TimerHandle_t

#define za_task_create                  xTaskCreate
#define za_task_createPinnedToCore      xTaskCreatePinnedToCore

#define ZA_vTaskDelay              vTaskDelay
#define ZA_pdMS_TO_TICKS           pdMS_TO_TICKS  /* Maybe use : 1000 / portTICK_RATE_MS*/
#define ZA_portTICK_PERIOD_MS      portTICK_PERIOD_MS
#define ZA_portMAX_DELAY           portMAX_DELAY

#ifdef CONFIG_ZHENGAN_DEBUG
#define ZA_ESP_LOGE ESP_LOGE
#else
#define ZA_ESP_LOGE(tag, fmt, args...) ((void)0)
#endif

#ifdef CONFIG_ZHENGAN_DEBUG
#define ZA_ESP_LOGW ESP_LOGW
#else
#define ZA_ESP_LOGW(tag, fmt, args...) ((void)0)
#endif

#ifdef CONFIG_ZHENGAN_DEBUG
#define ZA_ESP_LOGI ESP_LOGI
#else
#define ZA_ESP_LOGI(tag, fmt, args...) ((void)0)
#endif

#ifdef CONFIG_ZHENGAN_DEBUG
#define ZA_ESP_LOGD ESP_LOGD
#else
#define ZA_ESP_LOGD(tag, fmt, args...) ((void)0)
#endif

#ifdef CONFIG_ZHENGAN_DEBUG
#define ZA_ESP_LOGV ESP_LOGV
#else
#define ZA_ESP_LOGV(tag, fmt, args...) ((void)0)
#endif

#ifdef CONFIG_ZHENGAN_PRINTF_OPENCLOSE
#define ZA_PRINTF(fmt, args...) \
    do \
    { \
        printf(fmt, ##args); \
    }while (0)
#else
#define ZA_PRINTF(fmt, args...) ((void)0)
#endif

#define ZHENGAN_MAX(x, y) ( ((x) > (y)) ? (x) : (y) )
#define ZHENGAN_MIN(x, y) ( ((x) < (y)) ? (x) : (y) )

#define ZHENGAN_ABS(x)    (((x) < 0) ? -(x) : (x))

#define ZHENGAN_SIGN(x)   (((x) < 0) ? -1 : 1)
#define ZHENGAN_EVEN(x)   (((x) % 2) == 0)
#define ZHENGAN_ODD(x)    (((x) % 2) == 1)

#define ZHENGAN_ARRAY_SIZE(arr) ( sizeof( (arr) ) / sizeof( (arr[0]) ) )

/** @name macro functions
*   Instead of standard C functions.
*/
///@{


/** \def za_memcmp
    \brief Same usage with memcmp.
*/

/** \def za_memcpy
    \brief Same usage with memcpy.
*/

/** \def za_memmove
    \brief Same usage with memmove.
*/

/** \def za_memset
    \brief Same usage with memset.
*/

/** \def za_strcat
    \brief Same usage with strcat.
*/

/** \def za_strcpy
    \brief Same usage with strcpy.
*/

/** \def za_strncpy
    \brief Same usage with strncpy.
*/

/** \def za_strcmp
    \brief Same usage with strcmp.
*/

/** \def za_strncmp
    \brief Same usage with strncmp.
*/

/** \def za_strcasecmp
    \brief Same usage with strcasecmp.
*/

/** \def za_strncasecmp
    \brief Same usage with strncasecmp.
*/

/** \def za_strchr
    \brief Same usage with strchr.
*/

/** \def za_strrchr
    \brief Same usage with strrchr.
*/

/** \def za_strlen
    \brief Same usage with strlen.
*/

/** \def za_strstr
    \brief Same usage with strstr.
*/

/** \def za_strdup
    \brief Same usage with strdup.
*/

/** \def za_snprintf
    \brief Same usage with snprintf.
*/

/* macro functions instead of standard C functions ------------------------- */

#define za_memcmp(s1, s2, n) memcmp(s1, s2, n)
#define za_memcpy(dest, src, n) memcpy(dest, src, n)
#define za_memmove(dest, src, n) memmove(dest, src, n)
#define za_memset(s, c, n) memset(s, c, n)

#define za_strcat(dest, src)        strcat(dest, src)
#define za_strcpy(dest, src)        strcpy(dest, src)
#define za_strncpy(dest, src, n)    strncpy(dest, src, n)

#define za_strcmp(s1, s2) strcmp(s1, s2)
#define za_strncmp(s1, s2, n) strncmp(s1, s2, n)

#define za_strcasecmp(s1, s2) strcasecmp(s1, s2)
#define za_strncasecmp(s1, s2, n) strncasecmp(s1, s2, n)

#define za_strchr(s, c) strchr(s, c)
#define za_strrchr(s, c) strrchr(s, c)
#define za_strlen(s) strlen(s)
#define za_strstr(s1, s2) strstr(s1, s2)

#define za_strdup(s) strdup(s)

#define za_snprintf snprintf
#define za_sprintf sprintf

#define za_fgets fgets
#define za_fprintf fprintf

#define za_sizeof sizeof

#define za_malloc(n) malloc(n)
#define za_free(s) free(s)

typedef char                	ZA_INT8;    /**< Defines the data type of ZHENGAN_INT8. */
typedef unsigned char       	ZA_UINT8;   /**< Defines the data type of ZHENGAN_UINT8. */

typedef short               	ZA_INT16;   /**< Defines the data type of ZHENGAN_INT16. */
typedef unsigned short      	ZA_UINT16;  /**< Defines the data type of ZHENGAN_UINT16. */

typedef int                 	ZA_INT32;   /**< Defines the data type of ZHENGAN_INT32. */
typedef unsigned int        	ZA_UINT32;  /**< Defines the data type of ZHENGAN_UINT32. */

#ifdef ZHENGAN_HAVE_INT64
typedef long long           	ZA_INT64;   /**< Defines the data type of ZHENGAN_INT64 under Linux. */
typedef unsigned long long  	ZA_UINT64;  /**< Defines the data type of ZHENGAN_UINT64 under Linux. */
#endif

typedef char                	ZA_CHAR;    /**< Defines the data type of ZHENGAN_CHAR. */
typedef unsigned char       	ZA_UCHAR;   /**< Defines the data type of ZHENGAN_UCHAR. */
typedef unsigned short      	ZA_WCHAR;   /**< Defines the data type of ZHENGAN_WCHAR. */

typedef void                	ZA_VOID;    /**< Defines the data type of ZHENGAN_VOID. */
/** \def ZHENGAN_VOID_PARA
    \brief Declares that a function has no parameter.
*/
#define ZA_VOID_PARA            void

typedef float               	ZA_FLOAT;   /**< Defines the data type of ZHENGAN_FLOAT. */
typedef double              	ZA_DOUBLE;  /**< Defines the data type of ZHENGAN_DOUBLE. */

typedef bool                    ZA_BOOL;    /**< Defines the data type of ZHENGAN_BOOL. */
typedef ZA_UINT32               ZA_HANDLE;  /**< Defines the data type of ZHENGAN_HANDLE. */

typedef ZA_VOID * 		ZA_ESPOS_HANDLE; /**< Defines the handel data type of esp rtos op queue, semaphore, task etc. */

/** \def ZHENGAN_SIZE_t
    \brief Defines the type for expressing the file size.
*/
#ifdef ZHENGAN_HAVE_INT64
typedef ZA_UINT64 			ZA_SIZE_t;
#else
typedef ZA_UINT32 			ZA_SIZE_t;
#endif

/** Defines the unified error return value type of function. */
typedef enum
{
    ZA_SUCCESS = 0,                     /**< Successful.                            */
    ZA_FAILURE,                         /**< Failed.                                */
    ZA_ERROR_BAD_PARAMETER,             /**< Bad parameter passed.                  */
    ZA_ERROR_NO_MEMORY,                 /**< Memory allocation failed.              */
    ZA_ERROR_UNKNOWN_DEVICE,            /**< Unknown device name.                   */
    ZA_ERROR_ALREADY_INITIALIZED,       /**< Device already initialized.            */
    ZA_ERROR_NO_FREE_HANDLES,           /**< Cannot open device again.              */
    ZA_ERROR_OPEN_HANDLE,               /**< At least one open handle.              */
    ZA_ERROR_INVALID_HANDLE,            /**< Handle is not valid.                   */
    ZA_ERROR_FEATURE_NOT_SUPPORTED,     /**< Feature unavailable.                   */
    ZA_ERROR_INTERRUPT_INSTALL,         /**< Interrupt install failed.              */
    ZA_ERROR_INTERRUPT_UNINSTALL,       /**< Interrupt uninstall failed.            */
    ZA_ERROR_TIMEOUT,                   /**< Timeout occured.                       */
    ZA_ERROR_BUSY,                      /**< Device or resource is currently busy.  */
    ZA_ERROR_NOT_INITIALIZED,           /**< Device or resource not initialized.    */
    ZA_ERROR_DESTROYED,                 /**< Resource has been destroyed whilst calling thread is suspended on it. */
    ZA_ERROR_PERM,                      /**< Permission denied */

    ZA_ERROR_USER_CANCEL,	             /**< User cancels operation.                */
    ZA_ERROR_HOST_NAME_UNRESOLVED,      /**< Host name unresovled.                  */
    ZA_ERROR_SERVER_NOT_FOUND,          /**< Server not found.                      */
    ZA_ERROR_SERVER_NOT_RESPONDING,     /**< Server not responding.                 */
    ZA_ERROR_AUTH_FAILURE,              /**< Authentication failure.                */
    ZA_ERROR_MD5_UNMATCHED,             /**< MD5 unmatched.                         */
    ZA_ERROR_WRONG_CHECKSUM,            /**< Wrong checknum.                        */

    ZA_ERROR_QUEUE_EMPTY,               /**< Message queue is empty.                */
    ZA_ERROR_QUEUE_FULL,                /**< Message queue is full.                 */

    ZA_ERROR_NO_SIGNAL,                 /**< There is no signal. */

    ZA_ERROR_NOT_INSERTED,              /**< The card (SD Card / TF Card) is not inserted. */

    ZA_ERROR_APP = 0x3000,

    ZA_ERROR_DEVICE_INIT = (ZA_ERROR_APP | 0x100),
    ZA_ERROR_DEVICE_INIT_BLUETOOTH, 
    ZA_ERROR_DEVICE_INIT_WIFI,

    ZA_ERROR_SERVICE_INIT = (ZA_ERROR_APP | 0x200),
    ZA_ERROR_SERVICE_INIT_UPGRADE,

    ZA_ERROR_APP_ENDCODE = (0x4000 -1),

    ZA_ERROR_END
} ZA_Error_t;


/* Exported Macros --------------------------------------------------------- */

/** \def ZHENGAN_TRUE
    \brief For ZHENGAN_BOOL.
*/
#define ZA_TRUE     1

/** \def ZHENGAN_FALSE
    \brief For ZHENGAN_BOOL.
*/
#define ZA_FALSE    0

/** \def ZHENGAN_NULL
    \brief For an null pointer or handle.
*/
#define ZA_NULL     0
#define ZANULL     ((void *)ZA_NULL)


#define ZA_ESP_PASS pdPASS
#define ZA_ESP_FAIL pdFAIL
#define za_configASSERT configASSERT
#define ZA_ESPERROR_CHECK ESP_ERROR_CHECK

#define za_esp_err_t esp_err_t
#define za_esp_err_to_name esp_err_to_name
#define za_esp_log_buffer_hex esp_log_buffer_hex
#define za_esp_log_buffer_char     esp_log_buffer_char


ZA_VOID za_debug();

ZA_UINT32 za_common_first_before_place(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput);
ZA_UINT32 za_common_middle_value_between(const ZA_CHAR *pcStrInput, const ZA_CHAR *pcStrStart, const ZA_CHAR *pcStrEnd, ZA_CHAR *pcStrOutput);
ZA_UINT32 za_common_get_bind_id_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput);
ZA_UINT32 za_common_get_localmqtt_url_from_string_only(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput);
ZA_UINT32 za_common_get_localmqtt_url_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput);
ZA_UINT32 za_common_get_localmqtt_uname_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput);
ZA_UINT32 za_common_get_localmqtt_upasswd_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput);
ZA_UINT32 za_common_get_local_udpurl_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput);
ZA_UINT32 za_common_get_local_udpipv4_from_string(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput);
ZA_UINT32 za_common_get_local_udpport_from_string(const ZA_CHAR *pcStrInput, ZA_UINT32 *pnPort);
ZA_UINT32 za_common_get_zadiy_bin_path(const ZA_CHAR *pcStrInput, ZA_CHAR *pcStrOutput, ZA_UINT32 nOutLenMax);

#ifdef __cplusplus
}
#endif

#endif
