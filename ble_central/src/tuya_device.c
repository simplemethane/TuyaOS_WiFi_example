/**
* @file tuya_device.c
* @author www.tuya.com
* @brief tuya_device module is used to 
* @version 0.1
* @date 2022-05-20
*
* @copyright Copyright (c) tuya.inc 2022
*
*/

#include "tuya_cloud_types.h"
#include "tuya_error_code.h"
#include "uni_log.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_iot_wifi_api.h"

#include "tal_bluetooth.h"

#include "tal_thread.h"
#include "tal_system.h"
#include "tal_sleep.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
#define TASK_BLUETOOTH_PRIORITY             THREAD_PRIO_1
#define TASK_BLUETOOTH_SIZE                 1024

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC THREAD_CFG_T sg_task = {
    .priority = TASK_BLUETOOTH_PRIORITY,
    .stackDepth = TASK_BLUETOOTH_SIZE,
    .thrdname = "bluetooth"
};
STATIC THREAD_HANDLE sg_bluetooth_handle;

STATIC UCHAR_T adv_data_const[31] =
{
    0x02,
    0x01,
    0x06,
    0x03,
    0x02,
    0x50, 0xFD,
    0x17,
    0x16,
    0x50, 0xFD,
    0x41, 0x00,       //Frame Control
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

STATIC UCHAR_T scan_rsp_data_const[31] =
{   
    0x17,             // length
    0xFF,
    0xD0,
    0x07,
    0x00, //Encry Mode(8)
    0x00,0x00, //communication way bit0-mesh bit1-wifi bit2-zigbee bit3-NB
    0x00, //FLAG
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x03, //24
    0x09,
    0x54, 0x59,
};
STATIC TAL_BLE_PEER_INFO_T sg_ble_peripheral_info;
/***********************************************************
***********************function define**********************
***********************************************************/
/**
* @brief bluebooth event callback function
*
* @param[in] p_event: bluebooth event
* @return none
*/
VOID_T __bluetooth_event_callback(TAL_BLE_EVT_PARAMS_T *p_event)
{
    PR_DEBUG("----------bluetooth event callback-------");
    PR_DEBUG("bluetooth event is : %d", p_event->type);
    switch(p_event->type) {
        case TAL_BLE_EVT_ADV_REPORT: {
            INT_T i = 0;

            /*printf peer addr and addr type*/
            PR_DEBUG_RAW("Scanf device peer addr: ");
            for(i = 0;i<6;i++) {
                PR_DEBUG_RAW("  %d", p_event->ble_event.adv_report.peer_addr.addr[i]);
            }
            PR_DEBUG_RAW(" \r\n");

            if(TAL_BLE_ADDR_TYPE_RANDOM == p_event->ble_event.adv_report.peer_addr.type) {
                PR_DEBUG("Peer addr type is random address");
            } else {
                PR_DEBUG("Peer addr type is public address");
            }

            /*printf ADV type*/
            switch(p_event->ble_event.adv_report.adv_type) {
                case TAL_BLE_ADV_DATA: {
                    PR_DEBUG("ADV data only!");
                    break;
                }

                case TAL_BLE_RSP_DATA: {
                    PR_DEBUG("Scan Response Data only!");
                    break;
                }

                case TAL_BLE_ADV_RSP_DATA: {
                    PR_DEBUG("ADV data and Scan Response Data!");
                    break;
                }
            }

            /*printf ADV rssi*/
            PR_DEBUG("Received Signal Strength Indicator : %d", p_event->ble_event.adv_report.rssi);

            /*printf ADV data*/
            PR_DEBUG("Advertise packet data length : %d", p_event->ble_event.adv_report.data_len);
            PR_DEBUG_RAW("Advertise packet data: ");
            for(i = 0;i<p_event->ble_event.adv_report.data_len;i++) {
                PR_DEBUG_RAW("  0x%02X", p_event->ble_event.adv_report.p_data[i]);
            }
            PR_DEBUG_RAW(" \r\n");

            break;
        }
    }
}

/**
* @brief Initialization device
*
* @param[in] none
* @return none
*/
STATIC VOID_T user_main(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    TY_INIT_PARAMS_S init_param = {0};
    init_param.init_db = TRUE;

    strcpy(init_param.sys_env, TARGET_PLATFORM);
    /*Platform dependent function initialization*/
    TUYA_CALL_ERR_LOG(tuya_iot_init_params(NULL, &init_param));
    PR_DEBUG("----------------user main----------------");
   
    return;
}

/**
* @brief bluetooth task
*
* @param[in] param:Task parameters
* @return none
*/
VOID __bluetooth_task(VOID* param)
{
    OPERATE_RET op_ret = OPRT_OK;
    TAL_BLE_SCAN_PARAMS_T scan_cfg = {0};
    TAL_BLE_PEER_INFO_T connect_peer = {0};
    TAL_BLE_CONN_PARAMS_T connect_peer_param = {0};

    /*bluetooth init*/
    tal_system_sleep(5000);
    op_ret = tal_ble_bt_init(TAL_BLE_ROLE_CENTRAL, __bluetooth_event_callback);
    if(OPRT_OK != op_ret) {
        PR_ERR("err<%d>, ble init fail!", op_ret);
    }

    /*start scan*/
    scan_cfg.type = TAL_BLE_SCAN_TYPE_ACTIVE;
    scan_cfg.scan_interval = 0x400;
    scan_cfg.scan_window = 0x400;
    scan_cfg.timeout = 0xFFFF;
    scan_cfg.filter_dup = TLS_DISABLE;
    op_ret = tal_ble_scan_start(&scan_cfg);
    if(OPRT_OK != op_ret) {
        PR_ERR("err<%d>, ble scan fail!", op_ret);
    }

    while (1) {
        
        PR_DEBUG("ble task running!");
        tal_system_sleep(2000);
    }   
}

/**
* @brief   user entry function
*
* @param[in] none
* @return none
*/
VOID_T tuya_app_main(VOID_T)
{
    OPERATE_RET op_ret = OPRT_OK;

    user_main();
    PR_DEBUG("-----------------app main----------------");
    op_ret = tal_thread_create_and_start(&sg_bluetooth_handle, NULL, NULL, __bluetooth_task, NULL, &sg_task);
    if(OPRT_OK != op_ret) {
        PR_ERR("err<%d>,bluetooth task create fail", op_ret);
        return;
    }

    return;
}

