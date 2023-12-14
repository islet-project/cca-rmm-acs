/*
 * Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_rmm.h"

enum test_intent {
    RD_UNALIGNED = 0X0,
    RD_OUTSIDE_OF_PERMITTED_PA = 0X1,
    RD_DEV_MEM = 0X2,
    RD_STATE_UNDELEGATED = 0X3,
    RD_STATE_DELEGATED = 0X4,
    RD_STATE_REC = 0X5,
    RD_STATE_RTT = 0X6,
    RD_STATE_DATA = 0X7,
    LEVEL_STARTING_LEVLE = 0X8,
    LEVEL_OUT_OF_BOUND = 0X9,
    IPA_L2_UNALIGNED = 0XA,
    IPA_OUT_OF_BOUND = 0XB,
    IPA_NOT_MAPPED = 0XC,
    RTTE_UNASSIGED = 0XD,
    RTTE_ASSIGNED = 0XE,
    RTT_NON_HOMOGENEOUS = 0XF,
    LEVEL_BOUND_RTT_WALK = 0X10,
    LEVEL_BOUND_RTTE_STATE = 0X11
};

struct stimulus {
    char msg[100];
    uint64_t abi;
    enum test_intent label;
    uint64_t status;
    uint64_t index;
};

static struct stimulus test_data[] = {
    {.msg = "rd_align",
    .abi = RMI_RTT_FOLD,
    .label = RD_UNALIGNED,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "rd_bound",
    .abi = RMI_RTT_FOLD,
    .label = RD_OUTSIDE_OF_PERMITTED_PA,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "rd_bound",
    .abi = RMI_RTT_FOLD,
    .label = RD_DEV_MEM,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "rd_state",
    .abi = RMI_RTT_FOLD,
    .label = RD_STATE_UNDELEGATED,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "rd_state",
    .abi = RMI_RTT_FOLD,
    .label = RD_STATE_DELEGATED,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "rd_state",
    .abi = RMI_RTT_FOLD,
    .label = RD_STATE_REC,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "rd_state",
    .abi = RMI_RTT_FOLD,
    .label = RD_STATE_RTT,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "rd_state",
    .abi = RMI_RTT_FOLD,
    .label = RD_STATE_DATA,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "level_bound",
    .abi = RMI_RTT_FOLD,
    .label = LEVEL_STARTING_LEVLE,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "level_bound",
    .abi = RMI_RTT_FOLD,
    .label = LEVEL_OUT_OF_BOUND,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "ipa_align",
    .abi = RMI_RTT_FOLD,
    .label = IPA_L2_UNALIGNED,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "ipa_bound",
    .abi = RMI_RTT_FOLD,
    .label = IPA_OUT_OF_BOUND,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "rtt_walk",
    .abi = RMI_RTT_FOLD,
    .label = IPA_NOT_MAPPED,
    .status = RMI_ERROR_RTT,
    .index = 0},
    {.msg = "rtte_state",
    .abi = RMI_RTT_FOLD,
    .label = RTTE_UNASSIGED,
    .status = RMI_ERROR_RTT,
    .index = 2},
    {.msg = "rtte_state",
    .abi = RMI_RTT_FOLD,
    .label = RTTE_ASSIGNED,
    .status = RMI_ERROR_RTT,
    .index = 2},
    {.msg = "rtte_homo",
    .abi = RMI_RTT_FOLD,
    .label = RTT_NON_HOMOGENEOUS,
    .status = RMI_ERROR_RTT,
    .index = 3},
    {.msg = "level_bound_compare_rtt_walk",
    .abi = RMI_RTT_FOLD,
    .label = LEVEL_BOUND_RTT_WALK,
    .status = RMI_ERROR_INPUT,
    .index = 0},
    {.msg = "level_bound_compare_rtte_state",
    .abi = RMI_RTT_FOLD,
    .label = LEVEL_BOUND_RTTE_STATE,
    .status = RMI_ERROR_INPUT,
    .index = 0}
};
