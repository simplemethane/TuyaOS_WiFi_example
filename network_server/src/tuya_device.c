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
#include "tuya_cloud_com_defs.h"
#include "tuya_iot_wifi_api.h"
#include "uni_time_queue.h"
#include "gw_intf.h"
#include "uni_log.h"
#include "uni_thread.h"

#include "tal_network.h"
#include "tkl_network.h"
#include "tal_thread.h"
#include "tal_sleep.h"
#include "lwip_init.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
#define PID                                 "uiiyltkjmkhwumga"
#define UUID                                "tuya60dcbf014a5da4c6"
#define AUTHKEY                             "GPwI9eWvUFzYP3iGMnpx2wa18jq61maj"
#define USER_SW_VER                         "1.0.0"

#define TASK_NETWORK_SERVER_PRIORITY        THREAD_PRIO_2
#define TASK_NETWORK_SERVER_SIZE            1024
#define SERVICE_IP                          "192.168.137.95"
#define SERVICE_PORT                        7070
#define SERVER_BACKLOG_CONNECT_MAX          10
#define INET_BUFF_MAX                       30
/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC THREAD_CFG_T sg_task = {
    .priority = TASK_NETWORK_SERVER_PRIORITY,
    .stackDepth = TASK_NETWORK_SERVER_SIZE,
    .thrdname = "network server"
};
STATIC THREAD_HANDLE sg_network_server_handle;

/***********************************************************
***********************function define**********************
***********************************************************/
/**
* @brief  SOC device initialization
*
* @param[in] none
* @return OPRT_OK:success,other:fail.
*/
OPERATE_RET __soc_device_init(VOID_T)
{
    OPERATE_RET op_ret = OPRT_OK;
    WF_GW_PROD_INFO_S prod_info = {UUID, AUTHKEY};
    TY_IOT_CBS_S iot_cbs = {0};

    op_ret = tuya_iot_set_wf_gw_prod_info(&prod_info);
    if (OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_set_gw_prod_info err:%d", op_ret);
        return op_ret;
    }

    op_ret = tuya_iot_wf_soc_dev_init(GWCM_OLD, WF_START_SMART_AP_CONCURRENT, &iot_cbs, PID, USER_SW_VER);
    if (OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_wf_soc_dev_init err:%d", op_ret);
        return op_ret;
    }

    return op_ret;
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
    TUYA_CALL_ERR_LOG(tuya_iot_init_params(NULL, &init_param));
    PR_DEBUG("----------------user main----------------");
    TUYA_LwIP_Init();
    rt = __soc_device_init();
    if(OPRT_OK != rt) {
        PR_ERR("err<%d>, device init fail!", rt);
    }

    return;
}

/**
* @brief network server task
*
* @param[in] param:parameter
* @return none
*/
VOID __network_server_task(VOID* param)
{
    OPERATE_RET op_ret = OPRT_OK;
    INT_T socket_fd;
    INT_T fd;
    TUYA_IP_ADDR_T client_addr;
    UINT16_T client_port;

    /* Waiting for the network to initialize */
    tal_system_sleep(5000);

    /*create TCP socket*/
    socket_fd = tal_net_socket_create(PROTOCOL_TCP);
    PR_DEBUG("socket fd:%d", socket_fd);

    /*Ascii network string address is converted to host ordinal (4B) address*/
    TUYA_IP_ADDR_T host_addr = tal_net_str2addr(SERVICE_IP);
    PR_DEBUG("service host ordinal addr:%d", host_addr);
    
    /*socket bind*/
    op_ret = tal_net_bind(socket_fd, host_addr, SERVICE_PORT);
    if(OPRT_OK != op_ret) {
        PR_ERR("err<%d>, socket bind fail!", op_ret);
    }

    /*socket listen*/
    op_ret = tal_net_listen(socket_fd, SERVER_BACKLOG_CONNECT_MAX);
    if (OPRT_OK != op_ret) {
        PR_ERR("err<%d>, socket listen fail!", op_ret);
    }

    while (1) {
        /*socket accept*/
        tal_net_accept(socket_fd, &client_addr, &client_port);
        CHAR_T *inet_buff = tal_net_addr2str(client_addr);
        PR_DEBUG("fd:%d, connection from:%s, port:%d", fd, inet_buff, client_port);

        /*close network*/
        tal_net_close(socket_fd);
        break;
    }

    op_ret = tal_thread_delete(sg_network_server_handle);
    if(OPRT_OK != op_ret) {
        PR_ERR("err<%d>, delete thread demo task failed!", op_ret);
        return;
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
    op_ret = tal_thread_create_and_start(&sg_network_server_handle, NULL, NULL, __network_server_task, NULL, &sg_task);
    if(OPRT_OK != op_ret) {
        PR_ERR("err<%d>,client task create fail", op_ret);
        return;
    }

    return;
}
