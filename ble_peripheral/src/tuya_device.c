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
        case TAL_BLE_STACK_INIT: {
            PR_DEBUG("init Ble/Bt stack and start advertising.");
            if(p_event->ble_event.init == 0) {
                TAL_BLE_DATA_T adv_data;
                TAL_BLE_DATA_T rsp_data;

                adv_data.p_data = adv_data_const;
                adv_data.len = sizeof(adv_data_const);
                rsp_data.p_data = scan_rsp_data_const;
                rsp_data.len = sizeof(scan_rsp_data_const);
                
                tal_system_sleep(1000);
                tal_ble_advertising_data_set(&adv_data, &rsp_data);
                tal_ble_advertising_start(TUYAOS_BLE_DEFAULT_ADV_PARAM);
            }
            break;
        }
        case TAL_BLE_EVT_PERIPHERAL_CONNECT: {
            PR_DEBUG("Bluetooth starts to connect...");
            if(p_event->ble_event.connect.result == 0) {
                TAL_BLE_DATA_T read_data;
                UCHAR_T read_buffer[512];

                memcpy(&sg_ble_peripheral_info, &p_event->ble_event.connect.peer, sizeof(TAL_BLE_PEER_INFO_T));
                memcpy(read_buffer, adv_data_const, sizeof(adv_data_const));
                memcpy(&read_buffer[sizeof(adv_data_const)], scan_rsp_data_const, sizeof(scan_rsp_data_const));

                read_data.p_data = read_buffer;
                read_data.len = sizeof(adv_data_const) + sizeof(scan_rsp_data_const);

                // Verify Read Char
                tal_ble_server_common_read_update(&read_data);
            } else {
                memset(&sg_ble_peripheral_info, 0, sizeof(TAL_BLE_PEER_INFO_T));
            }
            break;
        }    
        case TAL_BLE_EVT_DISCONNECT: {
            PR_DEBUG("Bluetooth disconnect.");
            tal_ble_advertising_start(TUYAOS_BLE_DEFAULT_ADV_PARAM);
            break;
        }
         
        case TAL_BLE_EVT_CONN_PARAM_UPDATE: {
            PR_DEBUG("Parameter update successfully!");
            // Show Connect Parameters Info
            PR_DEBUG("Conn Param Update: Min = %f ms, Max = %f ms, Latency = %d, Sup = %d ms", 
                p_event->ble_event.conn_param.conn.min_conn_interval * 1.25, \
                p_event->ble_event.conn_param.conn.max_conn_interval * 1.25,\
                p_event->ble_event.conn_param.conn.latency, \
                p_event->ble_event.conn_param.conn.conn_sup_timeout * 10);
            break;  
        } 
            
        case TAL_BLE_EVT_MTU_REQUEST: {
            PR_DEBUG("MTU exchange request event.");
            PR_DEBUG("Get Response MTU Size = %d", p_event->ble_event.exchange_mtu.mtu);
            break;
        }
         
        case TAL_BLE_EVT_WRITE_REQ: {
            INT_T i = 0;

            PR_DEBUG("Get Device-Write Char Request");
            for ( i = 0; i < p_event->ble_event.write_report.report.len; i++) {
                PR_DEBUG("devicr send  data[%d]: %d", i, p_event->ble_event.write_report.report.p_data[i]);
            }
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

    /*bluetooth init*/
    tal_system_sleep(5000);
    op_ret = tal_ble_bt_init(TAL_BLE_ROLE_PERIPERAL, __bluetooth_event_callback);
    if(OPRT_OK != op_ret) {
        PR_ERR("err<%d>, ble init fail!", op_ret);
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

