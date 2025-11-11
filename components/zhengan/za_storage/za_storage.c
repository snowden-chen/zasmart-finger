/*  nvs and spiffs and others, maybe. */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"

#include "za_storage.h"

#include "esp_spiffs.h"

#include "za_aiot.h"

#include "za_command.h"


static const char *TAG = "ZA_STORAGE";

typedef struct
{
	ZA_BOOL bIfCmdSTMSet; 				// Command Store Mode if set or not - ZA_TRUE : mode setting and cmd file open
	ZA_BOOL bIfCmdSTMGet;
	FILE* fCmdStoreHandle;
	ZA_UINT32 nCmdInputCount;
	ZA_UINT32 nCmdOutputCount;
}za_storage_cmd_store_type_t;

static za_storage_cmd_store_type_t tCmdStoreContext;

/** Public **/

ZA_BOOL za_storage_nvs_init()
{
   ZA_ESP_LOGI(TAG, "ZA STORAGE NVS INIT ENTER");

   za_esp_err_t err = nvs_flash_init();

   if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
   {
      // NVS partition was truncated and needs to be erased
      // Retry nvs_flash_init
      ZA_ESPERROR_CHECK(nvs_flash_erase());

      err = nvs_flash_init();
   }

   ZA_ESPERROR_CHECK( err );

   return ZA_TRUE;
}

ZA_BOOL za_storage_spiffs_init()
{
   ZA_ESP_LOGI(TAG, "ZA STORAGE SPIFFS INIT ENTER");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
	{
        if (ret == ESP_FAIL)
		{
            ZA_ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
		else if (ret == ESP_ERR_NOT_FOUND)
		{
            ZA_ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
		else
		{
            ZA_ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }

        return ZA_FALSE;
    }

    size_t total = 0, used = 0;

    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
	{
        ZA_ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
	else
	{
        ZA_ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

	za_memset(&tCmdStoreContext, 0, sizeof(za_storage_cmd_store_type_t));

   	return ZA_TRUE;
}

ZA_BOOL za_storage_init()
{
	ZA_BOOL bIfWrite = ZA_FALSE;

	za_storage_sys_mode_t tStoreSysModeReadW = za_storage_get_system_mode(ZA_NULL);
	za_storage_cmdstore_config_t tStoreCmdConfReadW = za_storage_get_cmd_store_mode(ZA_NULL);
	za_storage_aiot_local_server_t tStoreLoServReadW = za_storage_get_local_server(ZA_NULL);

	ZA_ESP_LOGI(TAG, "za_storage_init ENTER");

	if (tStoreSysModeReadW.tWirelessMode == ZA_STORAGE_OPRATE_NULL)
	{
		tStoreSysModeReadW.tWirelessMode = ZA_STORAGE_BLE_HOST_MODE;
		//tStoreSysModeReadW.tWirelessMode = ZA_STORAGE_BLEHOST_WIFICLIENT_MODE;

		bIfWrite = ZA_TRUE;
	}

	if (tStoreSysModeReadW.tUpgradeMode == ZA_STORAGE_UPGRADE_NULL)
	{
		tStoreSysModeReadW.tUpgradeMode = ZA_STORAGE_UPGRADE_IGNORE_MODE;
		//tStoreSysModeReadW.tUpgradeMode = ZA_STORAGE_UPGRADE_FORCE_MODE;
		bIfWrite = ZA_TRUE;
	}

	//if (tStoreSysModeReadW.tOutmoduleType == ZA_STORAGE_OUT_OPRATE_NULL)
	if (tStoreSysModeReadW.tOutmoduleType != ZA_STORAGE_OUT_MARK_DIY)
	{
		tStoreSysModeReadW.tOutmoduleType = ZA_STORAGE_OUT_MARK_DIY;

		bIfWrite = ZA_TRUE;
	}

	if (tStoreSysModeReadW.tAiotServerMode == ZA_STORAGE_AIOT_SERVER_NULL)
	{
		tStoreSysModeReadW.tAiotServerMode = ZA_STORAGE_AIOT_SERV_INTERNET;

		bIfWrite = ZA_TRUE;
	}

	if (tStoreSysModeReadW.nFastBootCount < 3)
	{
		tStoreSysModeReadW.nFastBootCount ++;

		bIfWrite = ZA_TRUE;
	}

	if (bIfWrite)
	{
		za_storage_set_system_mode(tStoreSysModeReadW);
	}

	if (tStoreCmdConfReadW.tCmdstoreBtpMode == ZA_CMDSTORE_BOOTUP_OPRATE_NULL)
	{
		tStoreCmdConfReadW.tCmdstoreBtpMode = ZA_CMDSTORE_BOOTUP_OFF;

		za_storage_set_cmd_store_mode(tStoreCmdConfReadW);
	}

	if (tStoreLoServReadW.tLocalServMode == ZA_LOCALSERV_IP_MODE_NULL)
	{
		tStoreLoServReadW.tLocalServMode = ZA_LOCALSERV_IP_MODE_DEFAULT;

		za_strcpy((ZA_CHAR *)(tStoreLoServReadW.aSetLinkUrl), "ws://172.23.1.14:18830");
	
		za_storage_set_local_server(tStoreLoServReadW);
	}

	return ZA_TRUE;
}

ZA_BOOL za_storage_set_system_mode(za_storage_sys_mode_t tSysMode)
{
	nvs_handle hSysMode = ZA_NULL;
	za_esp_err_t errESPCode = ESP_OK;

	errESPCode = nvs_open(ZA_STORAGE_LOCALSTORAGE_PATH, NVS_READWRITE, &hSysMode);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (0x%x) opening NVS handle!", errESPCode);
		return ZA_FALSE;
	}

	za_storage_sys_mode_t *pSysParamsStore = za_malloc(sizeof(za_storage_sys_mode_t));

   	za_memset(pSysParamsStore, ZA_NULL, sizeof(za_storage_sys_mode_t));

	za_memcpy(pSysParamsStore, &tSysMode, sizeof(za_storage_sys_mode_t));

	errESPCode = nvs_set_blob(hSysMode, ZA_STORAGE_SYSTEM_MODE_KEY, pSysParamsStore, sizeof(za_storage_sys_mode_t));
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (0x%x) NVS SET NVS BLOB Wrong!", errESPCode);
		nvs_close(hSysMode);
		za_free(pSysParamsStore);
		
		return ZA_FALSE;
	}

	errESPCode = nvs_commit(hSysMode);

	za_free(pSysParamsStore);
	nvs_close(hSysMode);
	
	if (ESP_OK != errESPCode)
	{
		ZA_ESP_LOGE(TAG,"Error (0x%x) NVS Commit BLOB Wrong!", errESPCode);
		return ZA_FALSE;
	}

   return ZA_TRUE;
}

za_storage_sys_mode_t za_storage_get_system_mode(ZA_VOID *pvParameter)
{
	(void) pvParameter;

	za_esp_err_t errESPCode = ESP_OK;

	nvs_handle hSysMode = ZA_NULL;
	za_storage_sys_mode_t tSysParams;

	size_t nLength = sizeof(za_storage_sys_mode_t);

	za_memset(&tSysParams, ZA_NULL, sizeof(za_storage_sys_mode_t));

	errESPCode = nvs_open(ZA_STORAGE_LOCALSTORAGE_PATH, NVS_READONLY, &hSysMode);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (0x%s) opening NVS handle!", za_esp_err_to_name(errESPCode));

		return tSysParams;
	}

	errESPCode = nvs_get_blob(hSysMode, ZA_STORAGE_SYSTEM_MODE_KEY, &tSysParams, &nLength);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (%s) NVS GET NVS BLOB Wrong!", za_esp_err_to_name(errESPCode));
		nvs_close(hSysMode);

		za_memset(&tSysParams, ZA_NULL, sizeof(za_storage_sys_mode_t));
		
		return tSysParams;
	}

	if (nLength != sizeof(za_storage_sys_mode_t))
	{
		ZA_ESP_LOGW(TAG,"Warning : NVS Params Length (%d) is not right!", nLength);
		za_memset(&tSysParams, ZA_NULL, sizeof(za_storage_sys_mode_t));
		
		//ZA_ESP_LOGI(TAG, "Get the Path: %s in key %s . len: %d", ZA_STORAGE_LOCALSTORAGE_PATH, ZA_STORAGE_SYSTEM_MODE_KEY, nLength);
	}

	nvs_close(hSysMode);

	return tSysParams;
}

ZA_BOOL za_storage_set_local_server(za_storage_aiot_local_server_t tLocalServer)
{
	nvs_handle hLocalServer = ZA_NULL;
	za_esp_err_t errESPCode = ESP_OK;

	errESPCode = nvs_open(ZA_STORAGE_LOCALSTORAGE_PATH, NVS_READWRITE, &hLocalServer);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (0x%x) opening NVS handle!", errESPCode);
		return ZA_FALSE;
	}

	za_storage_aiot_local_server_t *pLoServParamsStore = za_malloc(sizeof(za_storage_aiot_local_server_t));

   	za_memset(pLoServParamsStore, ZA_NULL, sizeof(za_storage_aiot_local_server_t));

	za_memcpy(pLoServParamsStore, &tLocalServer, sizeof(za_storage_aiot_local_server_t));

	errESPCode = nvs_set_blob(hLocalServer, ZA_STORAGE_LOCAL_SERVER_KEY, pLoServParamsStore, sizeof(za_storage_aiot_local_server_t));
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (0x%x) NVS SET NVS BLOB Wrong!", errESPCode);
		nvs_close(hLocalServer);
		za_free(pLoServParamsStore);
		
		return ZA_FALSE;
	}

	errESPCode = nvs_commit(hLocalServer);

	za_free(pLoServParamsStore);
	nvs_close(hLocalServer);
	
	if (ESP_OK != errESPCode)
	{
		ZA_ESP_LOGE(TAG,"Error (0x%x) NVS Commit BLOB Wrong!", errESPCode);
		return ZA_FALSE;
	}

   return ZA_TRUE;
}

za_storage_aiot_local_server_t za_storage_get_local_server(ZA_VOID *pvParameter)
{
	(void) pvParameter;

	za_esp_err_t errESPCode = ESP_OK;

	nvs_handle hLocalServer = ZA_NULL;
	za_storage_aiot_local_server_t tLoServParams;

	size_t nLength = sizeof(za_storage_aiot_local_server_t);

	za_memset(&tLoServParams, ZA_NULL, sizeof(za_storage_aiot_local_server_t));

	errESPCode = nvs_open(ZA_STORAGE_LOCALSTORAGE_PATH, NVS_READONLY, &hLocalServer);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (0x%s) opening NVS handle!", za_esp_err_to_name(errESPCode));

		return tLoServParams;
	}

	errESPCode = nvs_get_blob(hLocalServer, ZA_STORAGE_LOCAL_SERVER_KEY, &tLoServParams, &nLength);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (%s) NVS GET NVS BLOB Wrong!", za_esp_err_to_name(errESPCode));
		nvs_close(hLocalServer);

		za_memset(&tLoServParams, ZA_NULL, sizeof(za_storage_aiot_local_server_t));
		
		return tLoServParams;
	}

	if (nLength != sizeof(za_storage_aiot_local_server_t))
	{
		ZA_ESP_LOGW(TAG,"Warning : key - %s , NVS Params Length (%d) is not right!", ZA_STORAGE_LOCAL_SERVER_KEY, nLength);
		za_memset(&tLoServParams, ZA_NULL, sizeof(za_storage_aiot_local_server_t));
		
		//ZA_ESP_LOGI(TAG, "Get the Path: %s in key %s . len: %d", ZA_STORAGE_LOCALSTORAGE_PATH, ZA_STORAGE_LOCAL_SERVER_KEY, nLength);
	}

	nvs_close(hLocalServer);

	return tLoServParams;
}

ZA_UINT32 za_storage_get_calibra_param1(ZA_VOID *pvParameter)
{
	(void) pvParameter;

	za_esp_err_t errESPCode = ESP_OK;

	nvs_handle hCalibraParam = ZA_NULL;
	int32_t value;

	errESPCode = nvs_open("oidcalibration", NVS_READONLY, &hCalibraParam);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (0x%s) opening NVS handle!", za_esp_err_to_name(errESPCode));

		return 0xc80;
	}

	errESPCode = nvs_get_i32(hCalibraParam, "param1", &value);

    if (errESPCode != ESP_OK) 
	{
		ZA_ESP_LOGE(TAG,"Error (0x%s) get param1 value!", za_esp_err_to_name(errESPCode));

		nvs_close(hCalibraParam);
        return 0xc80;
    }

	ZA_ESP_LOGI(TAG,"param1 value is %4lx", value);

	nvs_close(hCalibraParam);

    return (ZA_UINT32)value;
}


ZA_UINT32 za_storage_get_calibra_param2(ZA_VOID *pvParameter)
{
	(void) pvParameter;

	za_esp_err_t errESPCode = ESP_OK;

	nvs_handle hCalibraParam = ZA_NULL;
	int32_t value;

	errESPCode = nvs_open("oidcalibration", NVS_READONLY, &hCalibraParam);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (0x%s) opening NVS handle!", za_esp_err_to_name(errESPCode));

		return 0xc80;
	}

	errESPCode = nvs_get_i32(hCalibraParam, "param2", &value);

    if (errESPCode != ESP_OK) 
	{
		ZA_ESP_LOGE(TAG,"Error (0x%s) get param2 value!", za_esp_err_to_name(errESPCode));

		nvs_close(hCalibraParam);
        return 0xc80;
    }

	nvs_close(hCalibraParam);

	ZA_ESP_LOGI(TAG,"param2 value is %4lx", value);

    return (ZA_UINT32)value;
}

ZA_BOOL za_storage_erase_path_space(const char* strPathSpace)
{
	nvs_handle hSysMode = ZA_NULL;
	za_esp_err_t errESPCode = ESP_OK;

	errESPCode = nvs_open(strPathSpace, NVS_READWRITE, &hSysMode);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (%d) opening NVS handle!", errESPCode);

		return ZA_FALSE;
	}

	errESPCode = nvs_erase_all(hSysMode);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (%d) NVS ERASE THE PATH: %s Wrong!", errESPCode, strPathSpace);
		nvs_close(hSysMode);
		
		return ZA_FALSE;
	}

	errESPCode = nvs_commit(hSysMode);

	nvs_close(hSysMode);
	
	if (ESP_OK != errESPCode)
	{
		ZA_ESP_LOGE(TAG,"Error (%d) NVS Commit Wrong!", errESPCode);

		return ZA_FALSE;
	}

	return ZA_TRUE;
}

ZA_BOOL za_storage_erase_key_space(const char* strPathSpace, const char* strKeySpace)
{
	nvs_handle hSysMode = ZA_NULL;
	za_esp_err_t errESPCode = ESP_OK;

	errESPCode = nvs_open(strPathSpace, NVS_READWRITE, &hSysMode);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (%d) opening NVS handle!", errESPCode);
		
		return ZA_FALSE;
	}
	errESPCode = nvs_erase_key(hSysMode, strKeySpace);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"Error (%d) NVS ERASE THE KEY %s of PATH: %s Wrong!", errESPCode,strKeySpace,strPathSpace);
		nvs_close(hSysMode);
		
		return ZA_FALSE;
	}

	errESPCode = nvs_commit(hSysMode);

	nvs_close(hSysMode);
	
	if (ESP_OK != errESPCode)
	{
		ZA_ESP_LOGE(TAG,"Error (%d) NVS Commit Wrong!", errESPCode);

		return ZA_FALSE;
	}

	return ZA_TRUE;
}

ZA_BOOL za_storage_cmd_store_mode_if_rec(ZA_BOOL bIfCmdStmSet)
{
	if (bIfCmdStmSet)
	{
		if (tCmdStoreContext.bIfCmdSTMSet == ZA_TRUE)
		{
			ZA_ESP_LOGE(TAG, "/spiffs/cmdrecord.txt -> has opened, so close && delete it and reopen.");
		
			if (tCmdStoreContext.fCmdStoreHandle != NULL)
			{
				fclose(tCmdStoreContext.fCmdStoreHandle);
			}
			
			za_memset(&tCmdStoreContext, 0, sizeof(za_storage_cmd_store_type_t));
		}

		// Check if destination file exists before renaming
		struct stat st;

		if (stat(DEFAULT_CMD_STORE_SPIFFS_PATH, &st) == 0)
		{
			// Delete it if it exists
			unlink(DEFAULT_CMD_STORE_SPIFFS_PATH);
		}

		tCmdStoreContext.fCmdStoreHandle = fopen(DEFAULT_CMD_STORE_SPIFFS_PATH, "w");

		if (tCmdStoreContext.fCmdStoreHandle == NULL)
		{
			ZA_ESP_LOGE(TAG, "Failed to open file for writing: /spiffs/cmdrecord.txt");

			tCmdStoreContext.bIfCmdSTMSet = ZA_FALSE;

			return ZA_FALSE;
		}

		za_storage_cmdstore_config_t tCmdStoreConf = za_storage_get_cmd_store_mode(ZA_NULL);

		za_sprintf(tCmdStoreConf.cCmdFilePath, DEFAULT_CMD_STORE_SPIFFS_PATH);

		if (!za_storage_set_cmd_store_mode(tCmdStoreConf))
        {
            ZA_ESP_LOGI(TAG, " za_storage_cmd_store_mode_if_rec : za_storage_set_cmd_store_mode ERROR. ");
        }

		tCmdStoreContext.bIfCmdSTMSet = ZA_TRUE;
	}
	else
	{
		if (tCmdStoreContext.bIfCmdSTMSet == ZA_TRUE && tCmdStoreContext.fCmdStoreHandle != NULL)
		{
			fclose(tCmdStoreContext.fCmdStoreHandle);

			za_storage_cmdstore_config_t tCmdStoreConf = za_storage_get_cmd_store_mode(ZA_NULL);

			tCmdStoreConf.nCmdNumber = tCmdStoreContext.nCmdInputCount;

			if (!za_storage_set_cmd_store_mode(tCmdStoreConf))
			{
				ZA_ESP_LOGI(TAG, " za_storage_cmd_store_mode_if_rec : za_storage_set_cmd_store_mode ERROR. ");
			}
		}
		else if (tCmdStoreContext.fCmdStoreHandle != NULL)
		{
			ZA_ESP_LOGE(TAG, "Error happened when close : /spiffs/cmdrecord.txt, but still try close it.");

			fclose(tCmdStoreContext.fCmdStoreHandle);
		}

		za_memset(&tCmdStoreContext, 0, sizeof(za_storage_cmd_store_type_t));
	}

	return ZA_TRUE;
}


ZA_BOOL za_storage_cmd_store_mode_if_play(ZA_BOOL bIfCmdStmGet)
{
	if (tCmdStoreContext.bIfCmdSTMSet == ZA_TRUE)
	{
		ZA_ESP_LOGE(TAG, "Error za_storage_cmd_store_mode_if_play - now can not rec cmd !");

		return ZA_FALSE;
	}

	if (bIfCmdStmGet)
	{
		if (tCmdStoreContext.bIfCmdSTMGet == ZA_TRUE)
		{
			ZA_ESP_LOGE(TAG, "Error za_storage_cmd_store_mode_if_play - cmd file has been opened !");

			return ZA_FALSE;
		}

		// Check if destination file exists before renaming
		struct stat st;

		if (stat(DEFAULT_CMD_STORE_SPIFFS_PATH, &st) != 0)
		{
			ZA_ESP_LOGE(TAG, "Error za_storage_cmd_store_mode_if_play - cmd file has not been exist !");

			return ZA_FALSE;
		}

		tCmdStoreContext.fCmdStoreHandle = fopen(DEFAULT_CMD_STORE_SPIFFS_PATH, "r");

		if (tCmdStoreContext.fCmdStoreHandle == NULL)
		{
			ZA_ESP_LOGE(TAG, "Failed to open file for reading: /spiffs/cmdrecord.txt");

			tCmdStoreContext.bIfCmdSTMGet = ZA_FALSE;

			return ZA_FALSE;
		}

		tCmdStoreContext.bIfCmdSTMGet = ZA_TRUE;
	}
	else
	{
		if (tCmdStoreContext.bIfCmdSTMGet == ZA_TRUE && tCmdStoreContext.fCmdStoreHandle != NULL)
		{
			fclose(tCmdStoreContext.fCmdStoreHandle);
		}
		else if (tCmdStoreContext.fCmdStoreHandle != NULL)
		{
			ZA_ESP_LOGE(TAG, "Error happened when close : /spiffs/cmdrecord.txt, but still try close it.");

			fclose(tCmdStoreContext.fCmdStoreHandle);
		}

		za_memset(&tCmdStoreContext, 0, sizeof(za_storage_cmd_store_type_t));
	}

	return ZA_TRUE;
}

ZA_BOOL za_storage_cmd_store_input_stream(const ZA_CHAR * const pcSrcParams)
{
	if (tCmdStoreContext.bIfCmdSTMSet == ZA_TRUE && tCmdStoreContext.fCmdStoreHandle != NULL && pcSrcParams != NULL)
	{
		ZA_ESP_LOGI(TAG, "go ready to  za_storage_cmd_store_input_stream: /spiffs/cmdrecord.txt");

		if ( za_fprintf(tCmdStoreContext.fCmdStoreHandle, "%s\n", pcSrcParams) > 0 )
		{
			tCmdStoreContext.nCmdInputCount ++;
		}
		else
		{
			ZA_ESP_LOGE(TAG, "Error in fprintf when za_storage_cmd_store_input_stream - cmd: %s", pcSrcParams);

			return ZA_FALSE;
		}
	}
	else
	{
		ZA_ESP_LOGE(TAG, "Error za_storage_cmd_store_input_stream - cmd: %s", pcSrcParams);

		return ZA_FALSE;
	}

	return ZA_TRUE;
}

ZA_BOOL za_storage_cmd_store_output_stream(ZA_CHAR * pcSrcParams)
{
	ZA_CHAR *strRet;
	ZA_CHAR cTempChar[100] = {0};

	if ( pcSrcParams == NULL )
	{
		ZA_ESP_LOGE(TAG, "za_storage_cmd_store_output_stream Failed, cause pcSrcParams is null.");

		return ZA_FALSE;
	}

	if (tCmdStoreContext.bIfCmdSTMGet == ZA_TRUE && tCmdStoreContext.fCmdStoreHandle != NULL )
	{
		strRet = fgets(cTempChar, 100, tCmdStoreContext.fCmdStoreHandle);

		if (strRet == NULL) // file end
		{
			ZA_ESP_LOGE(TAG, "za_storage_cmd_store_output_stream Failed, fgets is null.");

			return ZA_FALSE;
		}

		ZA_CHAR* pos = strchr(cTempChar, '\n');
		if (pos) 
		{
			*pos = '\0';
		}

		za_strcpy(pcSrcParams, cTempChar);

		tCmdStoreContext.nCmdOutputCount ++;
	}
	else
	{
		ZA_ESP_LOGE(TAG, "Error za_storage_cmd_store_output_stream - fCmdStoreHandle: %d", (ZA_UINT32)(tCmdStoreContext.fCmdStoreHandle));

		return ZA_FALSE;
	}

	return ZA_TRUE;
}


ZA_BOOL za_storage_set_cmd_store_mode(za_storage_cmdstore_config_t tCmdStoreConf)
{
	nvs_handle hCmdStoreMode = ZA_NULL;
	za_esp_err_t errESPCode = ESP_OK;

	errESPCode = nvs_open(ZA_STORAGE_LOCALSTORAGE_PATH, NVS_READWRITE, &hCmdStoreMode);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"za_storage_set_cmd_store_mode : Error (0x%x) opening NVS handle!", errESPCode);

		return ZA_FALSE;
	}

	za_storage_cmdstore_config_t *pCmdParamsStore = za_malloc(sizeof(za_storage_cmdstore_config_t));

   	za_memset(pCmdParamsStore, ZA_NULL, sizeof(za_storage_cmdstore_config_t));

	za_memcpy(pCmdParamsStore, &tCmdStoreConf, sizeof(za_storage_cmdstore_config_t));

	errESPCode = nvs_set_blob(hCmdStoreMode, ZA_STORAGE_CMDSTORE_MODE_KEY, pCmdParamsStore, sizeof(za_storage_cmdstore_config_t));
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"za_storage_set_cmd_store_mode : Error (0x%x) NVS SET NVS BLOB Wrong!", errESPCode);

		nvs_close(hCmdStoreMode);

		za_free(pCmdParamsStore);
		
		return ZA_FALSE;
	}

	errESPCode = nvs_commit(hCmdStoreMode);

	za_free(pCmdParamsStore);

	nvs_close(hCmdStoreMode);
	
	if (ESP_OK != errESPCode)
	{
		ZA_ESP_LOGE(TAG,"za_storage_set_cmd_store_mode : Error (0x%x) NVS Commit BLOB Wrong!", errESPCode);

		return ZA_FALSE;
	}

   return ZA_TRUE;
}

za_storage_cmdstore_config_t za_storage_get_cmd_store_mode(ZA_VOID *pvParameter)
{
	(void) pvParameter;

	za_esp_err_t errESPCode = ESP_OK;

	nvs_handle hCmdStoreMode = ZA_NULL;

	za_storage_cmdstore_config_t tCmdStoreParams;

	size_t nLength = sizeof(za_storage_cmdstore_config_t);

	za_memset(&tCmdStoreParams, ZA_NULL, sizeof(za_storage_cmdstore_config_t));

	errESPCode = nvs_open(ZA_STORAGE_LOCALSTORAGE_PATH, NVS_READONLY, &hCmdStoreMode);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"za_storage_get_cmd_store_mode : Error (0x%s) opening NVS handle!", za_esp_err_to_name(errESPCode));

		return tCmdStoreParams;
	}

	errESPCode = nvs_get_blob(hCmdStoreMode, ZA_STORAGE_CMDSTORE_MODE_KEY, &tCmdStoreParams, &nLength);
	if (ESP_OK != errESPCode)
    {
    	ZA_ESP_LOGE(TAG,"za_storage_get_cmd_store_mode : Error (%s) NVS GET NVS BLOB Wrong!", za_esp_err_to_name(errESPCode));

		nvs_close(hCmdStoreMode);

		za_memset(&tCmdStoreParams, ZA_NULL, sizeof(za_storage_cmdstore_config_t));
		
		return tCmdStoreParams;
	}

	if (nLength != sizeof(za_storage_cmdstore_config_t))
	{
		ZA_ESP_LOGW(TAG,"za_storage_get_cmd_store_mode Warning : NVS Params Length (%d) is not right!", nLength);
		
		za_memset(&tCmdStoreParams, ZA_NULL, sizeof(za_storage_cmdstore_config_t));
		
		//ZA_ESP_LOGI(TAG, "Get the Path: %s in key %s . len: %d", ZA_STORAGE_LOCALSTORAGE_PATH, ZA_STORAGE_CMDSTORE_MODE_KEY, nLength);
	}

	nvs_close(hCmdStoreMode);

	return tCmdStoreParams;
}

ZA_BOOL za_storage_get_if_cmdstore_output_doing(ZA_VOID *pvParameter)
{
	(void) pvParameter;

	return tCmdStoreContext.bIfCmdSTMGet;
}

ZA_BOOL za_storage_get_if_cmdstore_input_doing(ZA_VOID *pvParameter)
{
	(void) pvParameter;

	return tCmdStoreContext.bIfCmdSTMSet;
}

ZA_BOOL za_storage_set_autoupdate_path(const ZA_CHAR *pcSrcParams)
{
	FILE* fSpiffsHandle;
	struct stat st;

	if (stat(STORAGE_AUTO_UPDATE_APP_PATH, &st) == 0)
	{
		//fputs("\0", fSpiffsHandle);

		// Delete it if it exists
		unlink(STORAGE_AUTO_UPDATE_APP_PATH);

		ZA_vTaskDelay(10 / ZA_portTICK_PERIOD_MS);

		fSpiffsHandle = fopen(STORAGE_AUTO_UPDATE_APP_PATH, "w");

		
	}
	else
	{
		fSpiffsHandle = fopen(STORAGE_AUTO_UPDATE_APP_PATH, "w");
	}

	if ( za_fprintf(fSpiffsHandle, "%s\n", pcSrcParams) <= 0 )
	{
		fclose(fSpiffsHandle);

		return ZA_FALSE;
	}

	fclose(fSpiffsHandle);

	return ZA_TRUE;
}

ZA_BOOL za_storage_get_autoupdate_path(ZA_CHAR *pcSrcParams)
{
	FILE* fSpiffsHandle;
	struct stat st;

	ZA_CHAR *strRet;
	ZA_CHAR cTempChar[255] = {0};

	if ( pcSrcParams == NULL )
	{
		ZA_ESP_LOGE(TAG, "za_storage_get_autoupdate_path Failed : pcSrcParams is null.");

		return ZA_FALSE;
	}

	if (stat(STORAGE_AUTO_UPDATE_APP_PATH, &st) == 0)
	{
		fSpiffsHandle = fopen(STORAGE_AUTO_UPDATE_APP_PATH, "r");

		strRet = fgets(cTempChar, 255, fSpiffsHandle);

		fclose(fSpiffsHandle);

		if (strRet == NULL) // file end
		{
			ZA_ESP_LOGE(TAG, "za_storage_get_autoupdate_path Failed, fgets is null.");

			return ZA_FALSE;
		}

		ZA_CHAR* pos = strchr(cTempChar, '\n');
		if (pos) 
		{
			*pos = '\0';
		}

		za_strcpy(pcSrcParams, cTempChar);

		return ZA_TRUE;
	}

	return ZA_FALSE;
}

ZA_BOOL za_storage_delete_file_path(const ZA_CHAR *pcSrcParams)
{
	struct stat st;
	
	if (stat(pcSrcParams, &st) == 0)
	{
		// Delete it if it exists
		unlink(pcSrcParams);
	}

	return ZA_TRUE;
}

ZA_BOOL za_storage_if_file_path_exist(const ZA_CHAR *pcSrcParams)
{
	struct stat st;

	//if (access(pcSrcParams, F_OK) != -1)
	if (stat(pcSrcParams, &st) == 0)
	{
		return ZA_TRUE;
	}

	return ZA_FALSE;
}

