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
#include "lwip_init.h"

#include "tal_wifi.h"
#include "tal_sw_timer.h"

#include "tal_thread.h"
#include "tal_system.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
#define TASK_WIFI_AP_PRIORITY        THREAD_PRIO_1
#define TASK_WIFI_AP_SIZE            2048
#define WIFI_CHANNEL                 5

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC THREAD_CFG_T sg_task = {
    .priority = TASK_WIFI_AP_PRIORITY,
    .stackDepth = TASK_WIFI_AP_SIZE,
    .thrdname = "wifi ap"
};
STATIC THREAD_HANDLE sg_wifi_ap_handle;

/***********************************************************
***********************function define**********************
***********************************************************/
/**
* @brief wifi Related event callback function
*
* @param[in] event:event
* @param[in] arg:parameter
* @return none
*/
VOID_T wifi_event_callback(WF_EVENT_E event, VOID_T *arg)
{
    PR_DEBUG("-------------event callback-------------");

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
    //Initialization LWIP
    TUYA_LwIP_Init();
    PR_DEBUG("----------------user main----------------");

    return;
}

/**
* @brief WiFi AP task
*
* @param[in] param:Task parameters
* @return none
*/
VOID __ap_task(VOID* param)
{
    OPERATE_RET op_ret = OPRT_OK;
    FAST_WF_CONNECTED_AP_INFO_T *ap_info = NULL;
    UCHAR_T test_ssid[] = "test_wifi";        //ssid
    UCHAR_T test_pass[] = "12345678";         //passworld
    CHAR_T test_ip[] = "192.168.137.02";      //ip addr
    CHAR_T test_mask[] = "255.255.255.0";     //net mask
    CHAR_T test_gw[] = "192.168.1.1";         //gateway
    NW_IP_S nw_ip = {0};

    /*wifi init*/
    op_ret = tal_wifi_init(wifi_event_callback);
    if(OPRT_OK != op_ret) {
        PR_ERR("err<%d>,wifi init fail!", op_ret);
        return op_ret;
    }

    /*Set WiFi mode to AP*/
    op_ret = tal_wifi_set_work_mode(WWM_SOFTAP);
    if(OPRT_OK != op_ret) {
        PR_ERR("err<%d>,set Wifi AP mode fail!", op_ret);
    }

    /*WiFi Start AP mode*/
    strcpy(nw_ip.ip, test_ip);
    strcpy(nw_ip.mask, test_mask);
    strcpy(nw_ip.gw, test_gw);

    WF_AP_CFG_IF_S wifi_cfg = {
        .s_len = sizeof(test_ssid),     //ssid length
        .p_len = sizeof(test_pass),     //password length
        .chan = WIFI_CHANNEL,           //channel
        .md = WAAM_OPEN,                //encryption type
        .ip = nw_ip,                    //ip information
        .ms_interval = 100,             //broadcast interval,Unit(ms)
        .max_conn = 3                   //max sta connect numbers 
    };

    strcpy(wifi_cfg.ssid, test_ssid);   //ssid
    strcpy(wifi_cfg.passwd, test_pass); //password
    op_ret = tal_wifi_ap_start(&wifi_cfg);
    if(OPRT_OK != op_ret) {
        PR_ERR("err<%d>,wifi ap mode start fail", op_ret);
    }
    
    while (1) {

        PR_DEBUG("AP mode task running!");
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
    /*WiFi AP mode*/
    op_ret = tal_thread_create_and_start(&sg_wifi_ap_handle, NULL, NULL, __ap_task, NULL, &sg_task);
    if(OPRT_OK != op_ret) {
        PR_ERR("err<%d>,wifi ap task create fail", op_ret);
        return;
    }

    return;
}





