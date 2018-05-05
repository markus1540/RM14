/**
 ****************************************************************************************
 *
 * @file streamdatad_task.c
 *
 * @brief StreamData Device profile task
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev: $
 *
 ****************************************************************************************
 */

/**
****************************************************************************************
*
* @file streamdatad_task.c
*
* @brief StreamData Device profile task.
*
* Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer
* program includes Confidential, Proprietary Information and is a Trade Secret of
* Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited
* unless authorized in writing. All Rights Reserved.
*
* <bluetooth.support@diasemi.com> and contributors.
*
****************************************************************************************
*/

/**
 ****************************************************************************************
 * @addtogroup STREAMDATADTASK
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwble_config.h"

#if (BLE_STREAMDATA_DEVICE)

#include "gap.h"
#include "gattc_task.h"
#include "attm_util.h"
#include "atts_util.h"
#include "attm_cfg.h"
#include "prf_utils.h"
#include "streamdatad_task.h"
#include "streamdatad.h"
#include "llc_task.h" // llc_nb_of_pkt_evt_complete
#include "roidmi_includes.h"
#include "spi_flash.h"
#include "app_log.h"

#if defined (CFG_PRINTF)
#include "app_console.h"
#endif

#define version	("v_0.0.0.1")   //firmware version
extern bool     b_Force_Flag;   
extern bool     ChangeFilterFlag;
extern uint8_t  SystemStatus;
extern uint8_t  WorkMode;
extern uint32_t RoidmiUserID;
extern uint8_t  BLEAddress[6];
extern uint32_t ul_First_Time;
extern uint8    uc_Car_Style;
// Pong Service
static const att_svc_desc_t streamdatad_svc = STREAMDATAD_SERVICE_UUID;

static const struct att_char_desc streamdatad_d1_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR_NO_RESP | ATT_CHAR_PROP_NTF, 0, STREAMDATAD_D1_UUID);

static const struct att_char_desc streamdatad_d2_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_NTF, 0, STREAMDATAD_D2_UUID);

static const struct att_char_desc streamdatad_d3_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR_NO_RESP | ATT_CHAR_PROP_NTF, 0, STREAMDATAD_D3_UUID);

static const struct att_char_desc streamdatad_d4_char = ATT_CHAR(ATT_CHAR_PROP_RD, 0, STREAMDATAD_D4_UUID);

static const struct att_char_desc streamdatad_d5_char = ATT_CHAR(ATT_CHAR_PROP_RD, 0, STREAMDATAD_D5_UUID);

static const struct att_char_desc streamdatad_d6_char = ATT_CHAR(ATT_CHAR_PROP_RD, 0, STREAMDATAD_D6_UUID);

static const struct att_char_desc streamdatad_d7_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR_NO_RESP | ATT_CHAR_PROP_NTF, 0, STREAMDATAD_D7_UUID);

static const struct att_char_desc streamdatad_d8_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_NTF, 0, STREAMDATAD_D8_UUID);

static const struct att_char_desc streamdatad_d9_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR_NO_RESP | ATT_CHAR_PROP_NTF, 0, STREAMDATAD_D9_UUID);

static const struct att_char_desc streamdatad_da_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_NTF, 0, STREAMDATAD_DA_UUID);

static const struct att_char_desc streamdatad_db_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_NTF, 0, STREAMDATAD_DB_UUID);

static const struct att_char_desc streamdatad_dc_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR_NO_RESP | ATT_CHAR_PROP_NTF, 0, STREAMDATAD_DC_UUID);

// Data description

static const uint8_t streamdatad_d1_desc[] = STREAMDATAD_D1_DESC;

static const uint8_t streamdatad_d2_desc[] = STREAMDATAD_D2_DESC;

static const uint8_t streamdatad_d3_desc[] = STREAMDATAD_D3_DESC;

static const uint8_t streamdatad_d4_desc[] = STREAMDATAD_D4_DESC;

static const uint8_t streamdatad_d5_desc[] = STREAMDATAD_D5_DESC;

static const uint8_t streamdatad_d6_desc[] = STREAMDATAD_D6_DESC;

static const uint8_t streamdatad_d7_desc[] = STREAMDATAD_D7_DESC;

static const uint8_t streamdatad_d8_desc[] = STREAMDATAD_D8_DESC;

static const uint8_t streamdatad_d9_desc[] = STREAMDATAD_D9_DESC;

static const uint8_t streamdatad_da_desc[] = STREAMDATAD_DA_DESC;

static const uint8_t streamdatad_db_desc[] = STREAMDATAD_DB_DESC;

static const uint8_t streamdatad_dc_desc[] = STREAMDATAD_DC_DESC;


// Full STREAMDATAD Database Description - Used to add attributes into the database

static const struct attm_desc streamdatad_att_db[STREAMDATAD_IDX_NB] =
{
#if (1 == ROIDMI)
    /* StreamData Device service */

    [STREAMDATAD_IDX_PRIM_SVC] = {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), sizeof(streamdatad_svc), sizeof(streamdatad_svc), (uint8_t *) &streamdatad_svc},

    //FFD1:开关机控制(读、写、通知)

    [STREAMDATAD_IDX_STREAMDATAD_D1_CHAR]  = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d1_char), sizeof(streamdatad_d1_char), (uint8_t *) &streamdatad_d1_char},

    [STREAMDATAD_IDX_STREAMDATAD_D1_VAL]   = {STREAMDATAD_D1_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 1, (uint8_t *) &SystemStatus},

    [STREAMDATAD_IDX_STREAMDATAD_D1_EN]    = {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D1_DESC]  = {ATT_DESC_CHAR_USER_DESCRIPTION,  PERM(RD, ENABLE), STREAMDATAD_D1_DESC_LEN, STREAMDATAD_D1_DESC_LEN, (uint8_t *) streamdatad_d1_desc},

    //FFD2:固件版本号(读)

    [STREAMDATAD_IDX_STREAMDATAD_D2_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d2_char), sizeof(streamdatad_d2_char), (uint8_t *) &streamdatad_d2_char},

    [STREAMDATAD_IDX_STREAMDATAD_D2_VAL]  = {STREAMDATAD_D2_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), sizeof(version)-1, (uint8_t *) version},

    [STREAMDATAD_IDX_STREAMDATAD_D2_EN]   = {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D2_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D2_DESC_LEN, STREAMDATAD_D2_DESC_LEN, (uint8_t *) streamdatad_d2_desc},

    //FFD3:档位、蜂鸣器和指示灯开关切换(读、写、通知)

    [STREAMDATAD_IDX_STREAMDATAD_D3_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d3_char), sizeof(streamdatad_d3_char), (uint8_t *) &streamdatad_d3_char},

    [STREAMDATAD_IDX_STREAMDATAD_D3_VAL]  = {STREAMDATAD_D3_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 1, (uint8_t *) &WorkMode},

    [STREAMDATAD_IDX_STREAMDATAD_D3_EN]   = {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D3_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D3_DESC_LEN, STREAMDATAD_D3_DESC_LEN, (uint8_t *) streamdatad_d3_desc},

    //FFD4:设备工作时间(读、通知)

    [STREAMDATAD_IDX_STREAMDATAD_D4_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d4_char), sizeof(streamdatad_d4_char), (uint8_t *) &streamdatad_d4_char},

    [STREAMDATAD_IDX_STREAMDATAD_D4_VAL] =  {STREAMDATAD_D4_UUID, PERM(RD, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D4_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D4_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D4_DESC_LEN, STREAMDATAD_D4_DESC_LEN, (uint8_t *) streamdatad_d4_desc},

    //FFD5:设备净化时间(读、通知)

    [STREAMDATAD_IDX_STREAMDATAD_D5_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d5_char), sizeof(streamdatad_d5_char), (uint8_t *) &streamdatad_d5_char},

    [STREAMDATAD_IDX_STREAMDATAD_D5_VAL]  = {STREAMDATAD_D5_UUID, PERM(RD, ENABLE) | PERM(WR, DISABLE) | PERM(NTF, DISABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D5_EN]   = {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D5_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D5_DESC_LEN, STREAMDATAD_D5_DESC_LEN, (uint8_t *) streamdatad_d5_desc},

    //FFD6:滤芯使用时间(读、通知)

    [STREAMDATAD_IDX_STREAMDATAD_D6_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d6_char), sizeof(streamdatad_d6_char), (uint8_t *) &streamdatad_d6_char},

    [STREAMDATAD_IDX_STREAMDATAD_D6_VAL]  = {STREAMDATAD_D6_UUID, PERM(RD, ENABLE) | PERM(WR, DISABLE) | PERM(NTF, DISABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D6_EN]   = {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D6_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D6_DESC_LEN, STREAMDATAD_D6_DESC_LEN, (uint8_t *) streamdatad_d6_desc},

    //FFD7:滤芯到期相关操作(读、写、通知)

    [STREAMDATAD_IDX_STREAMDATAD_D7_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d7_char), sizeof(streamdatad_d7_char), (uint8_t *) &streamdatad_d7_char},

    [STREAMDATAD_IDX_STREAMDATAD_D7_VAL]  = {STREAMDATAD_D7_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 1, (uint8_t *) &ChangeFilterFlag},

    [STREAMDATAD_IDX_STREAMDATAD_D7_EN]   = {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D7_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D7_DESC_LEN, STREAMDATAD_D7_DESC_LEN, (uint8_t *) streamdatad_d7_desc},

    //FFD8:空气质量(读、通知)

    [STREAMDATAD_IDX_STREAMDATAD_D8_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d8_char), sizeof(streamdatad_d8_char), (uint8_t *) &streamdatad_d8_char},

    [STREAMDATAD_IDX_STREAMDATAD_D8_VAL] =  {STREAMDATAD_D8_UUID, PERM(RD, ENABLE) | PERM(WR, DISABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 1, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D8_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D8_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D8_DESC_LEN, STREAMDATAD_D8_DESC_LEN, (uint8_t *) streamdatad_d8_desc},

    //FFD9:用户ID(读、写)

    [STREAMDATAD_IDX_STREAMDATAD_D9_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d9_char), sizeof(streamdatad_d9_char), (uint8_t *) &streamdatad_d9_char},

    [STREAMDATAD_IDX_STREAMDATAD_D9_VAL] =  {STREAMDATAD_D9_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 4, (uint8_t *) &RoidmiUserID},

    [STREAMDATAD_IDX_STREAMDATAD_D9_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D9_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D9_DESC_LEN, STREAMDATAD_D9_DESC_LEN, (uint8_t *) streamdatad_d9_desc},

    //FFDA:设备异常相关数据(读、通知)

    [STREAMDATAD_IDX_STREAMDATAD_DA_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_da_char), sizeof(streamdatad_da_char), (uint8_t *) &streamdatad_da_char},

    [STREAMDATAD_IDX_STREAMDATAD_DA_VAL] =  {STREAMDATAD_DA_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DA_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DA_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_DA_DESC_LEN, STREAMDATAD_DA_DESC_LEN, (uint8_t *) streamdatad_da_desc},



    //FFDB:蓝牙地址(读、通知)

    [STREAMDATAD_IDX_STREAMDATAD_DB_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_db_char), sizeof(streamdatad_db_char), (uint8_t *) &streamdatad_db_char},

    [STREAMDATAD_IDX_STREAMDATAD_DB_VAL] =  {STREAMDATAD_DB_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DB_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DB_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_DB_DESC_LEN, STREAMDATAD_DB_DESC_LEN, (uint8_t *) streamdatad_db_desc},
		
	//FFDC:保留
	[STREAMDATAD_IDX_STREAMDATAD_DC_CHAR]  = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_dc_char), sizeof(streamdatad_dc_char), (uint8_t *) &streamdatad_dc_char},

    [STREAMDATAD_IDX_STREAMDATAD_DC_VAL]   = {STREAMDATAD_DC_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 1, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DC_EN]    = {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DC_DESC]  = {ATT_DESC_CHAR_USER_DESCRIPTION,  PERM(RD, ENABLE), STREAMDATAD_DC_DESC_LEN, STREAMDATAD_DC_DESC_LEN, (uint8_t *) streamdatad_dc_desc},

		
#else//==========================================================================================================================================================================
		
		
    /* StreamData Device service */

    [STREAMDATAD_IDX_PRIM_SVC] = {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), sizeof(streamdatad_svc), sizeof(streamdatad_svc), (uint8_t *) &streamdatad_svc},

    //ffd1:

    [STREAMDATAD_IDX_STREAMDATAD_D1_CHAR]  = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d1_char), sizeof(streamdatad_d1_char), (uint8_t *) &streamdatad_d1_char},

    [STREAMDATAD_IDX_STREAMDATAD_D1_VAL] =   {STREAMDATAD_D1_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D1_EN] =    {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D1_DESC] =  {ATT_DESC_CHAR_USER_DESCRIPTION,  PERM(RD, ENABLE), STREAMDATAD_D1_DESC_LEN, STREAMDATAD_D1_DESC_LEN, (uint8_t *) streamdatad_d1_desc},

    //ffd2:

    [STREAMDATAD_IDX_STREAMDATAD_D2_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d2_char), sizeof(streamdatad_d2_char), (uint8_t *) &streamdatad_d2_char},

    [STREAMDATAD_IDX_STREAMDATAD_D2_VAL] =  {STREAMDATAD_D2_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D2_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D2_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D2_DESC_LEN, STREAMDATAD_D2_DESC_LEN, (uint8_t *) streamdatad_d2_desc},

    //ffd3:

    [STREAMDATAD_IDX_STREAMDATAD_D3_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d3_char), sizeof(streamdatad_d3_char), (uint8_t *) &streamdatad_d3_char},

    [STREAMDATAD_IDX_STREAMDATAD_D3_VAL] =  {STREAMDATAD_D3_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D3_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D3_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D3_DESC_LEN, STREAMDATAD_D3_DESC_LEN, (uint8_t *) streamdatad_d3_desc},

    //ffd4:

    [STREAMDATAD_IDX_STREAMDATAD_D4_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d4_char), sizeof(streamdatad_d4_char), (uint8_t *) &streamdatad_d4_char},

    [STREAMDATAD_IDX_STREAMDATAD_D4_VAL] =  {STREAMDATAD_D4_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D4_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D4_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D4_DESC_LEN, STREAMDATAD_D4_DESC_LEN, (uint8_t *) streamdatad_d4_desc},

    //ffd5:

    [STREAMDATAD_IDX_STREAMDATAD_D5_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d5_char), sizeof(streamdatad_d5_char), (uint8_t *) &streamdatad_d5_char},

    [STREAMDATAD_IDX_STREAMDATAD_D5_VAL] =  {STREAMDATAD_D5_UUID, PERM(RD, ENABLE) | PERM(WR, DISABLE) | PERM(NTF, DISABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D5_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D5_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D5_DESC_LEN, STREAMDATAD_D5_DESC_LEN, (uint8_t *) streamdatad_d5_desc},

    //ffd6:

    [STREAMDATAD_IDX_STREAMDATAD_D6_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d6_char), sizeof(streamdatad_d6_char), (uint8_t *) &streamdatad_d6_char},

    [STREAMDATAD_IDX_STREAMDATAD_D6_VAL] =  {STREAMDATAD_D6_UUID, PERM(RD, ENABLE) | PERM(WR, DISABLE) | PERM(NTF, DISABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D6_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D6_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D6_DESC_LEN, STREAMDATAD_D6_DESC_LEN, (uint8_t *) streamdatad_d6_desc},

    //ffd7:

    [STREAMDATAD_IDX_STREAMDATAD_D7_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d7_char), sizeof(streamdatad_d7_char), (uint8_t *) &streamdatad_d7_char},

    [STREAMDATAD_IDX_STREAMDATAD_D7_VAL] =  {STREAMDATAD_D7_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D7_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D7_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D7_DESC_LEN, STREAMDATAD_D7_DESC_LEN, (uint8_t *) streamdatad_d7_desc},

    //ffd8:

    [STREAMDATAD_IDX_STREAMDATAD_D8_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d8_char), sizeof(streamdatad_d8_char), (uint8_t *) &streamdatad_d8_char},

    [STREAMDATAD_IDX_STREAMDATAD_D8_VAL] =  {STREAMDATAD_D8_UUID, PERM(RD, ENABLE) | PERM(WR, DISABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D8_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D8_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D8_DESC_LEN, STREAMDATAD_D8_DESC_LEN, (uint8_t *) streamdatad_d8_desc},

    //ffd9:

    [STREAMDATAD_IDX_STREAMDATAD_D9_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_d9_char), sizeof(streamdatad_d9_char), (uint8_t *) &streamdatad_d9_char},

    [STREAMDATAD_IDX_STREAMDATAD_D9_VAL] =  {STREAMDATAD_D9_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D9_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_D9_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_D9_DESC_LEN, STREAMDATAD_D9_DESC_LEN, (uint8_t *) streamdatad_d9_desc},

    //ffda:

    [STREAMDATAD_IDX_STREAMDATAD_DA_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_da_char), sizeof(streamdatad_da_char), (uint8_t *) &streamdatad_da_char},

    [STREAMDATAD_IDX_STREAMDATAD_DA_VAL] =  {STREAMDATAD_DA_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DA_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DA_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_DA_DESC_LEN, STREAMDATAD_DA_DESC_LEN, (uint8_t *) streamdatad_da_desc},

    //ffdb

    [STREAMDATAD_IDX_STREAMDATAD_DB_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_db_char), sizeof(streamdatad_db_char), (uint8_t *) &streamdatad_db_char},

    [STREAMDATAD_IDX_STREAMDATAD_DB_VAL] =  {STREAMDATAD_DB_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DB_EN] =   {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DB_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_DB_DESC_LEN, STREAMDATAD_DB_DESC_LEN, (uint8_t *) streamdatad_db_desc},
	
	//ffdc
	[STREAMDATAD_IDX_STREAMDATAD_DC_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(streamdatad_dc_char), sizeof(streamdatad_dc_char), (uint8_t *) &streamdatad_dc_char},

    [STREAMDATAD_IDX_STREAMDATAD_DC_VAL]  = {STREAMDATAD_DC_UUID, PERM(RD, ENABLE) | PERM(WR, DISABLE) | PERM(NTF, ENABLE), (sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE), 1, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DC_EN]   = {ATT_DESC_CLIENT_CHAR_CFG, (PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint16_t), 0, (uint8_t *) NULL},

    [STREAMDATAD_IDX_STREAMDATAD_DC_DESC] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), STREAMDATAD_DC_DESC_LEN, STREAMDATAD_DC_DESC_LEN, (uint8_t *) streamdatad_dc_desc},
#endif
};

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref STREAMDATAD_CREATE_DB_REQ message.
 * The handler adds STREAMDATAD Service into the database using the database
 * configuration value given in param.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int streamdatad_create_db_req_handler(ke_msg_id_t const msgid,
        struct streamdatad_create_db_req const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
	//Service Configuration Flag
    uint32_t cfg_flag = 0xFFFFFFFF;
    //Database Creation Status
    uint8_t status;

    //Save Application ID
    streamdatad_env.appid = src_id;


    // set start handle or automatically set it when creating database (start_hdl = 0)
    streamdatad_env.shdl = param->start_hdl;

    //Add Service Into Database
    status = attm_svc_create_db(&streamdatad_env.shdl, (uint8_t *)&cfg_flag,  STREAMDATAD_IDX_NB, NULL,
                                dest_id, &streamdatad_att_db[0]);

    //Disable GLS
    attmdb_svc_set_permission(streamdatad_env.shdl, PERM(SVC, DISABLE));

    //Go to Idle State
    if (status == ATT_ERR_NO_ERROR){
			//If we are here, database has been fulfilled with success, go to idle test
			ke_state_set(TASK_STREAMDATAD, STREAMDATAD_IDLE);
    }
    //Send response to application
    struct streamdatad_create_db_cfm *cfm = KE_MSG_ALLOC(STREAMDATAD_CREATE_DB_CFM, streamdatad_env.appid,
                                            TASK_STREAMDATAD, streamdatad_create_db_cfm);
    cfm->status = status;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/*
 * MESSAGE HANDLERS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Handles reception of the @ref STREAMDATAD_ENABLE_REQ message.
 * The handler enables the StreamData Device profile.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int streamdatad_enable_req_handler(ke_msg_id_t const msgid,
        struct streamdatad_enable_req const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
	uint16_t disable_val = 0x00;
    //Save Application ID
    //streamdatad_env.con_info.appid = src_id;
    streamdatad_env.con_info.prf_id = dest_id;
    streamdatad_env.con_info.appid  = src_id;
    streamdatad_env.appid = src_id;
    //streamdatad_env.con_info.prf_id = dest_id;
    // Save the connection handle associated to the profile
    streamdatad_env.con_info.conidx = gapc_get_conidx(param->conhdl);
    // Save the connection handle associated to the profile
    streamdatad_env.conhdl = param->conhdl;
    streamdatad_env.next_attribute_idx = 0;
    streamdatad_env.nr_enabled_attributes = 0;
    streamdatad_env.stream_enabled = 0;
    // get tx buffers available
    nb_buf_av = l2cm_get_nb_buffer_available() - 6;

    attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_D1_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));

    attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_D2_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));

    attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_D3_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));

    attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_D4_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));

    attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_D5_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));

    attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_D6_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));

    attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_D7_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));

    attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_D8_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));

    attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_D9_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));

    attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_DA_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));

    attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_DB_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));
		
	attmdb_att_set_value(STREAMDATAD_HANDLE(STREAMDATAD_IDX_STREAMDATAD_DC_EN), sizeof(uint16_t), (uint8_t *) & (disable_val));

    //Enable Service
    attmdb_svc_set_permission(streamdatad_env.shdl, PERM(SVC, ENABLE));
    // Go to active state
    ke_state_set(TASK_STREAMDATAD, STREAMDATAD_ACTIVE);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref STREAMDATAD_DISABLE_REQ message.
 * The handler disables the streamdataderometer profile.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int streamdatad_disable_req_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    // Go to idle state
    ke_state_set(TASK_STREAMDATAD, STREAMDATAD_IDLE);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref STREAMDATAD_SEND_DATA_PACKETS_REQ message.
 * The handler sends up to l2cm_get_nb_buffer_available() packets of data from param.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
extern	void set_pxact_gpio(void);

int streamdatad_send_data_packets_req_handler(ke_msg_id_t const msgid,
        struct streamdatad_send_data_packets_req const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
	uint16_t next_packet;
    uint16_t nr_packets;
//    uint16_t nr_buffers_available;
    uint16_t *packet_buffer_enabled;
    uint16_t len = 0;

    if (!streamdatad_env.stream_enabled) return KE_MSG_CONSUMED;

    next_packet = 0;
    nr_packets = param->nr_packets;

#if 0
    if (streamdatad_env.nr_overflow_packets > 0)
        nr_buffers_available = streamdatad_send_overflow_packets(); // send overflow packets first.
    else
        nr_buffers_available = l2cm_get_nb_buffer_available() - 6;  //by ED TBC
#endif

    nb_buf_av = l2cm_get_nb_buffer_available();

    for (int li = 0; (li < STREAMDATAD_MAX) && (nr_packets > 0) && (nb_buf_av > 0); li++)
    {
        packet_buffer_enabled = NULL;
        attmdb_att_get_value(STREAMDATAD_DIR_EN_HANDLE(streamdatad_env.next_attribute_idx), &(len), (uint8_t **) & (packet_buffer_enabled));

        if ((packet_buffer_enabled && (*packet_buffer_enabled)))
        {
            // Update the value in the attribute database
            attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(streamdatad_env.next_attribute_idx), sizeof(uint8_t) * STREAMDATAD_PACKET_SIZE, (uint8_t *) & (param->packets[next_packet][0]));

            // Send notification
            prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(streamdatad_env.next_attribute_idx));
        }
        else
        {
            len = 2;
        }

        //set_pxact_gpio();
        next_packet++;
        nr_packets--;
        nb_buf_av--;

        streamdatad_env.next_attribute_idx++;
        if (streamdatad_env.next_attribute_idx >= STREAMDATAD_MAX)
        {
            streamdatad_env.next_attribute_idx = 0;
            break; // the for loop
        }
        // else notification at this index was not enabled;
    }
    //set_pxact_gpio();
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATT_WRITE_CMD_IND message.
 * The handler checks if the stream needs to be turned on.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

extern bool b_Hand_Flag;//2016.8.22
extern uint8_t uc_Work_Mode_Set;//2016.9.18
extern bool BeepSet;
extern bool CarbonLedSet; //PM2.5
extern bool CommonLedSet;  //Carbon
extern bool b_Working_Flag;
extern bool b_Power_Low_Flag;//2016.9.21
extern uint16_t SendErrorCode;
extern uint8_t FilterType;
extern bool BeepOneFlag;
extern bool BeepTwoFlag;
extern bool BeepLongFlag;

uint8_t uc_Clean_Filter_State;

static int gattc_write_cmd_ind_handler(ke_msg_id_t const msgid,
                                       struct gattc_write_cmd_ind const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    uint8_t value_buf[20] = {0};
	uint8_t sd_buf[2];
	uint8_t send_buf[4];
//	uint32_t Filer;
    attmdb_att_update_value(param->handle, param->length, param->offset,(uint8_t *) & (param->value[0]));
    switch (STREAMDATAD_IDX(param->handle)){
		case STREAMDATAD_IDX_STREAMDATAD_D1_EN:
		case STREAMDATAD_IDX_STREAMDATAD_D2_EN:
		case STREAMDATAD_IDX_STREAMDATAD_D3_EN:
		case STREAMDATAD_IDX_STREAMDATAD_D4_EN:
		case STREAMDATAD_IDX_STREAMDATAD_D5_EN:
		case STREAMDATAD_IDX_STREAMDATAD_D6_EN:
		case STREAMDATAD_IDX_STREAMDATAD_D7_EN:
		case STREAMDATAD_IDX_STREAMDATAD_D8_EN:
		case STREAMDATAD_IDX_STREAMDATAD_D9_EN:
		case STREAMDATAD_IDX_STREAMDATAD_DA_EN:
		case STREAMDATAD_IDX_STREAMDATAD_DB_EN:
		case STREAMDATAD_IDX_STREAMDATAD_DC_EN:
		  atts_write_rsp_send(streamdatad_env.conhdl, param->handle, PRF_ERR_OK);
		break;
/*******FFD1:设备开关机*******/
		case STREAMDATAD_IDX_STREAMDATAD_D1_VAL:
			memcpy(value_buf, &(param->value), param->length);
			if(b_Power_Low_Flag == 0){
				if(value_buf[0] == 0x01){ 
					if(SystemStatus == STANDBY){
						SystemStatus = WORK;
						BeepTwoFlag  = 1;
						EEPROM_Read();
						b_Hand_Flag=1;							
						b_Working_Flag = 0;
					}
				}
				else if(value_buf[0] == 0x00){ 
					if(SystemStatus == WORK){
						SystemStatus = STANDBY;
						BeepLongFlag = 1;
						BeepOneFlag = 1;
						EEPROM_Write();
						b_Hand_Flag=1;
					}
				}
			}
			break;
/*******FFD2:设备固件版本号*******/
		case STREAMDATAD_IDX_STREAMDATAD_D2_VAL:
			break;
/*******FFD3:档位、蜂鸣器和指示灯切换开关*******/		
		case STREAMDATAD_IDX_STREAMDATAD_D3_VAL:
		#if (ROIDMI == 1)
			memcpy(value_buf, &(param->value), param->length);
			if(SystemStatus == WORK){
				switch(value_buf[0]){
					case 0x00:
						if(roidmi_flash_save_data.FilterTypeEE == 0x00){ //滤芯为普通滤芯
							WorkMode = 0;
							b_Force_Flag = 0;
							BeepOneFlag = 1;
						}
						break;
					case 0x01:
						WorkMode = 1;
						BeepOneFlag = 1;					
					break;
					case 0x02:
						WorkMode = 2;
						BeepOneFlag = 1;					
					break;
					case 0x03:
						WorkMode = 3;
						BeepOneFlag = 1;
					break;
					default:
						break;
				}
				roidmi_flash_save_data.WorkModeEE = WorkMode;
				EEPROM_Write();	
			}
			/*******蜂鸣器开关*******/
			if(value_buf[0] == 0x05){
				if(value_buf[2] == 0x0e){
					BeepOneFlag = 1;
					roidmi_flash_save_data.BeepModeEE = 0;
					BeepSet = 0;
					if(FilterType == 0x01)
						send_buf[3] = (roidmi_flash_save_data.LedModeEE + 24);
					else if(FilterType == 0x00)
						send_buf[3] = CommonLedSet + 24;
				}
				else if(value_buf[2] == 0x0f){
					roidmi_flash_save_data.BeepModeEE = 1;
					BeepSet = 1;
					if(FilterType == 0x01)
						send_buf[3] = (roidmi_flash_save_data.LedModeEE+24);
					else if(FilterType == 0x00)
						send_buf[3] = CommonLedSet + 24;
				}
				EEPROM_Write();
				send_buf[0] = WorkMode;
				send_buf[1] = (roidmi_flash_save_data.WorkModeEE + 10);
				send_buf[2] = (roidmi_flash_save_data.BeepModeEE + 14);
				attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(3), sizeof(send_buf), (uint8_t *)send_buf);
				prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(3));
			}
			/*******LED指示灯开关********/
			else if(value_buf[0] == 0x06){
				if(value_buf[2] == 0x0e){
					roidmi_flash_save_data.LedModeEE = 0;
					CarbonLedSet = 0;
					CommonLedSet = 0;
					send_buf[3] = (CommonLedSet + 24);
				}
				else if(value_buf[2] == 0x0f){
					if(FilterType == 0x01){
						roidmi_flash_save_data.LedModeEE = 1;
						CarbonLedSet = 1;
						send_buf[3] = (roidmi_flash_save_data.LedModeEE + 24);
					}
					else if(FilterType == 0x00){  //PM2.5 filter
						CommonLedSet = 0;
						send_buf[3] = (CommonLedSet+24);
					}
				}
				EEPROM_Write();
				send_buf[0] = WorkMode; 
				send_buf[1] = (roidmi_flash_save_data.WorkModeEE + 10);
				send_buf[2] = (roidmi_flash_save_data.BeepModeEE + 14);
				attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(3), sizeof(send_buf), (uint8_t *)send_buf);
				prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(3));
			}
#endif
			break;
		/*
		case STREAMDATAD_IDX_STREAMDATAD_D4_VAL://FFD4:设备工作时间
		break;
		
		case STREAMDATAD_IDX_STREAMDATAD_D5_VAL://FFD5:总净化时间
			break;
			
		case STREAMDATAD_IDX_STREAMDATAD_D6_VAL://FFD6:滤芯工作时间
		break;
		*/
/*******FFD7:滤芯到期相关操作*******/
		case STREAMDATAD_IDX_STREAMDATAD_D7_VAL:
#if (ROIDMI == 1)
			memcpy(value_buf, &(param->value), param->length); 
			if(value_buf[0] == 0x01){ //滤芯寿命复位
				roidmi_flash_save_data.FilterWorkTimeEE = 0;
				ChangeFilterFlag = 0;							
				uc_Clean_Filter_State = 3;
				EEPROM_Write();
				sd_buf[0] = ChangeFilterFlag;
				sd_buf[1] = uc_Clean_Filter_State;
				attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(6), 4, (uint8_t *)&roidmi_flash_save_data.FilterWorkTimeEE);
				prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(6));
				attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(7), sizeof(sd_buf), (uint8_t *)sd_buf);
				prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(7));
			}
			else if(value_buf[0] == 0x02){ //更换滤芯后保存的时间
				if(value_buf[4] == 0x00){ //普通滤芯
					sd_buf[0] = 0x20;
					FilterType  = 0x00;
				}
				else if(value_buf[4] == 0x01){ //活性炭滤芯
					sd_buf[0] = 0x21;
					FilterType  = 0x01;
				}
				uc_Clean_Filter_State = 3; //滤芯寿命复位成功
				roidmi_flash_save_data.FilterTypeEE = FilterType; //滤芯型号
				roidmi_flash_save_data.FilterWorkTimeEE = value_buf[1]*65536 + (value_buf[2]*256) + value_buf[3]; //保存净化时间
				EEPROM_Write();
				sd_buf[1] = uc_Clean_Filter_State;
				attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(6), 4, (uint8_t *)&roidmi_flash_save_data.FilterWorkTimeEE);
				prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(6));
				attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(7), sizeof(sd_buf), (uint8_t *)sd_buf);
				prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(7));	
			}
			else if(value_buf[0] == 0x03){ //更换滤芯
				if(value_buf[4] == 0x00){ //PM2.5滤芯
					sd_buf[0] = 0x30;
					FilterType = 0x00;				
				}
				else if (value_buf[4] == 0x01){ //活性炭滤芯
					sd_buf[0] = 0x31;					
					FilterType = 0x01;
				}
				uc_Clean_Filter_State = 3;
				roidmi_flash_save_data.FilterTypeEE = FilterType; //保存滤芯型号
				EEPROM_Write();
				sd_buf[1] = uc_Clean_Filter_State;
				attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(7), sizeof(sd_buf), (uint8_t *)sd_buf);
				prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(7));
			}
			else{ //滤芯寿命复位失败
				uc_Clean_Filter_State = 4;
				sd_buf[0] = ChangeFilterFlag;
				sd_buf[1] = uc_Clean_Filter_State;	  
				attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(6), 4, (uint8_t *)&roidmi_flash_save_data.FilterWorkTimeEE);
				prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(6));
				attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(7), sizeof(sd_buf), (uint8_t *)sd_buf);
				prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(7));						
			}
#endif
			break;
				
		case STREAMDATAD_IDX_STREAMDATAD_D8_VAL://FFD8:空气质量状况
			break;
	
		case STREAMDATAD_IDX_STREAMDATAD_D9_VAL://FFD9:用户ID
#if (ROIDMI == 1)
			memcpy(&RoidmiUserID, &(param->value), param->length);
			spi_flash_block_erase(ROIDMI_FLASH_OFFSET_2, SECTOR_ERASE);
			__nop();
			spi_flash_write_data((uint8_t *)&RoidmiUserID, ROIDMI_FLASH_OFFSET_2, 4);
			__nop();
			attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(9), sizeof(RoidmiUserID), (uint8_t *)&RoidmiUserID);
			prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(9));
#endif
			break;
		
		case STREAMDATAD_IDX_STREAMDATAD_DA_VAL://FFDA:异常相关数据
			break;

		case STREAMDATAD_IDX_STREAMDATAD_DB_VAL://FFDB:电瓶电压
			break;
				
		case STREAMDATAD_IDX_STREAMDATAD_DC_VAL://FFDC device type
			break;
	}
    return (KE_MSG_CONSUMED);
}

uint8_t nb_buf_av;
static int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	if(param->req_type == GATTC_NOTIFY){
		nb_buf_av++;
		//set_pxact_gpio();
    }
	return (KE_MSG_CONSUMED);
}


/*
 * TASK DESCRIPTOR DEFINITIONS
 ****************************************************************************************
 */
/// Disabled State handler definition.
const struct ke_msg_handler streamdatad_disabled[] =
{
	{STREAMDATAD_CREATE_DB_REQ,   (ke_msg_func_t) streamdatad_create_db_req_handler}
};


/// IDLE State handlers definition
const struct ke_msg_handler streamdatad_idle[] =
{
    {STREAMDATAD_ENABLE_REQ, (ke_msg_func_t)streamdatad_enable_req_handler}
};

/// ACTIVE State handlers definition
const struct ke_msg_handler streamdatad_active[] =
{
    {STREAMDATAD_DISABLE_REQ, (ke_msg_func_t)streamdatad_disable_req_handler},
    {STREAMDATAD_SEND_DATA_PACKETS_REQ, (ke_msg_func_t)streamdatad_send_data_packets_req_handler},
    {GATTC_WRITE_CMD_IND, (ke_msg_func_t)gattc_write_cmd_ind_handler},
    //    {GATTC_CMP_EVT,       (ke_msg_func_t)gattc_cmp_evt_handler},
};

/// Specifies the message handler structure for every input state
const struct ke_state_handler streamdatad_state_handler[STREAMDATAD_STATE_MAX] =
{
    /// DISABLE State message handlers.
    [STREAMDATAD_DISABLED]  = KE_STATE_HANDLER(streamdatad_disabled),
    /// IDLE State message handlers.
    [STREAMDATAD_IDLE]      = KE_STATE_HANDLER(streamdatad_idle),
    /// ACTIVE State message handlers.
    [STREAMDATAD_ACTIVE]    = KE_STATE_HANDLER(streamdatad_active),

};

/// Specifies the message handlers that are common to all states.
const struct ke_state_handler streamdatad_default_handler = KE_STATE_HANDLER_NONE;

/// Defines the placeholder for the states of all the task instances.
ke_state_t streamdatad_state[STREAMDATAD_IDX_MAX]; // __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

#endif /* BLE_STREAMDATA_DEVICE */
/// @} STREAMDATADTASK
