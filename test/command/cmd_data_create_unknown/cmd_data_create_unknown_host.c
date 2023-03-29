/*
 * Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"
#include "val_host_rmi.h"
#include "rmi_data_create_unknown_data.h"
#include "command_common_host.h"

#define IPA_WIDTH 40
#define MAX_GRANULES 256

#define L3_SIZE PAGE_SIZE
#define L2_SIZE (512 * L3_SIZE)
#define L1_SIZE (512 * L2_SIZE)

/*      Valid Realm memory layout
 *
 * ipa: 0x0 (1st protected L3 Entry)
 *  ------------------------------------
 * |  HIPAS = UNASSIGNED, RIPAS = RAM   |
 *  ------------------------------------
 * |   HIPAS = ASSIGNED, RIPAS = RAM    |
 *  ------------------------------------
 * |         HIPAS = DESTROYED          |
 *  ------------------------------------
 * |  HIPAS = UNASSIGNED, RIPAS = EMPTY |
 *  ------------------------------------
 * ipa: 0x4000
 *
 *                  (...)
 *
 *
 * ipa: 0x1XXX000 (1st unprotected L3 Entry)
 *  ------------------------------------
 * |         HIPAS = VALID_NS           |
 *  ------------------------------------
 * |        HIPAS = UNASSIGNED          |
 *  ------------------------------------
 * ipa: 0x1XX2000
 *
 */

#define IPA_ADDR_UNASSIGNED 0x0
#define IPA_ADDR_DATA1  (4 * PAGE_SIZE)
#define IPA_ADDR_DATA2  (5 * PAGE_SIZE)

#define MAP_LEVEL 3

#define NUM_REALMS 3
#define VALID_REALM 0
#define ACTIVE_REALM 1
#define NULL_REALM 2

static val_host_realm_ts realm_test[NUM_REALMS];

static struct argument_store {
    uint64_t data_valid;
    uint64_t rd_valid;
    uint64_t ipa_valid;
} c_args;

static struct invalid_argument_store {
    uint64_t rec_gran;
} c_args_invalid;

struct arguments {
    uint64_t data;
    uint64_t rd;
    uint64_t ipa;
};

static uint64_t data_valid_prep_sequence(void)
{
    return g_delegated_prep_sequence();
}

static uint64_t g_rd_new_prep_sequence(uint16_t vmid)
{
    val_host_realm_ts realm_init;
    val_host_rmifeatureregister0_ts features_0;

    val_memset(&realm_init, 0, sizeof(realm_init));
    features_0.s2sz = 40;
    val_memcpy(&realm_init.realm_feat_0, &features_0, sizeof(features_0));

    realm_init.hash_algo = RMI_HASH_SHA_256;
    realm_init.s2_starting_level = 0;
    realm_init.num_s2_sl_rtts = 1;
    realm_init.vmid = vmid;

    if (val_host_realm_create_common(&realm_init))
    {
        LOG(ERROR, "\tRealm create failed\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(1)));
        return VAL_TEST_PREP_SEQ_FAILED;
    }
    realm_test[vmid].rd = realm_init.rd;
    realm_test[vmid].rtt_l0_addr = realm_init.rtt_l0_addr;
    return realm_init.rd;
}

static uint64_t rd_valid_prep_sequence(void)
{
    return g_rd_new_prep_sequence(VALID_REALM);
}

static uint64_t ipa_valid_prep_sequence(void)
{
    // RTTE[ipa].state = UNASSIGNED
    // RTTE[ipa].ripas = RAM
    if (create_mapping(IPA_ADDR_UNASSIGNED, true, c_args.rd_valid))
    {
        LOG(ERROR, "\tCouldn't create the protected mapping\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(2)));
        return VAL_TEST_PREP_SEQ_FAILED;
    }
    return IPA_ADDR_UNASSIGNED;
}

static uint64_t g_rec_ready_prep_sequence(uint64_t rd)
{
    val_host_realm_ts realm;
    val_host_rec_params_ts rec_params;

    realm.rec_count = 1;
    realm.rd = rd;
    rec_params.pc = 0;
    rec_params.flags = RMI_RUNNABLE;
    rec_params.mpidr = 0;

    if (val_host_rec_create_common(&realm, &rec_params))
    {
        LOG(ERROR, "\tCouldn't destroy the Realm\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(3)));
        return VAL_TEST_PREP_SEQ_FAILED;
    }

    return realm.rec[0];
}

static uint64_t valid_input_args_prep_sequence(void)
{
    c_args.data_valid = data_valid_prep_sequence();
    if (c_args.data_valid == VAL_TEST_PREP_SEQ_FAILED)
        return VAL_TEST_PREP_SEQ_FAILED;

    c_args.rd_valid = rd_valid_prep_sequence();
    if (c_args.rd_valid == VAL_TEST_PREP_SEQ_FAILED)
        return VAL_TEST_PREP_SEQ_FAILED;

    c_args.ipa_valid = ipa_valid_prep_sequence();
    if (c_args.ipa_valid == VAL_TEST_PREP_SEQ_FAILED)
        return VAL_TEST_PREP_SEQ_FAILED;

    return VAL_SUCCESS;
}

static uint64_t intent_to_seq(struct stimulus *test_data, struct arguments *args)
{
    enum test_intent label = test_data->label;

    switch (label)
    {
        case DATA_UNALIGNED:
            args->rd = c_args.rd_valid;
            args->ipa = c_args.ipa_valid;
            args->data = g_unaligned_prep_sequence(c_args.data_valid);
            break;

        case DATA_DEV_MEM_MMIO:
            args->rd = c_args.rd_valid;
            args->ipa = c_args.ipa_valid;
            args->data = g_dev_mem_prep_sequence();
            break;

        case DATA_OUTSIDE_OF_PERMITTED_PA:
            args->rd = c_args.rd_valid;
            args->ipa = c_args.ipa_valid;
            args->data = g_outside_of_permitted_pa_prep_sequence();
            break;

        case DATA_STATE_UNDELEGATED:
            args->rd = c_args.rd_valid;
            args->ipa = c_args.ipa_valid;
            args->data = g_undelegated_prep_sequence();
            if (args->data == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            break;

        case DATA_STATE_RD:
            args->rd = c_args.rd_valid;
            args->ipa = c_args.ipa_valid;
            args->data = c_args.rd_valid;
            break;

        case DATA_STATE_REC:
            args->rd = c_args.rd_valid;
            args->ipa = c_args.ipa_valid;
            args->data = g_rec_ready_prep_sequence(c_args.rd_valid);
            if (args->data == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            break;

        case DATA_STATE_RTT:
            args->rd = c_args.rd_valid;
            args->ipa = c_args.ipa_valid;
            args->data = realm_test[VALID_REALM].rtt_l0_addr;
            break;

        case DATA_STATE_DATA:
            args->rd = c_args.rd_valid;
            args->ipa = c_args.ipa_valid;
            args->data = g_data_prep_sequence(c_args.rd_valid, IPA_ADDR_DATA1);
            if (args->data == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            break;

        case RD_UNALIGNED:
            args->rd = g_unaligned_prep_sequence(c_args.rd_valid);
            args->ipa = c_args.ipa_valid;
            args->data = c_args.data_valid;
            break;

        case RD_DEV_MEM_MMIO:
            args->rd = g_dev_mem_prep_sequence();
            args->ipa = c_args.ipa_valid;
            args->data = c_args.data_valid;
            break;

        case RD_OUTSIDE_OF_PERMITTED_PA:
            args->rd = g_outside_of_permitted_pa_prep_sequence();
            args->ipa = c_args.ipa_valid;
            args->data = c_args.data_valid;
            break;

        case RD_STATE_UNDELEGATED:
            args->rd = g_undelegated_prep_sequence();
            if (args->rd == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            args->ipa = c_args.ipa_valid;
            args->data = c_args.data_valid;
            break;

        case RD_STATE_DELEGATED:
            args->rd = g_delegated_prep_sequence();
            if (args->rd == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            args->ipa = c_args.ipa_valid;
            args->data = c_args.data_valid;
            break;

        case RD_STATE_REC:
            args->rd = c_args_invalid.rec_gran;
            args->ipa = c_args.ipa_valid;
            args->data = c_args.data_valid;
            break;

        case RD_STATE_RTT:
            args->rd = realm_test[VALID_REALM].rtt_l0_addr;
            args->ipa = c_args.ipa_valid;
            args->data = c_args.data_valid;
            break;

        case RD_STATE_DATA:
            args->rd = g_data_prep_sequence(c_args.rd_valid, IPA_ADDR_DATA2);
            if (args->rd == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            args->ipa = c_args.ipa_valid;
            args->data = c_args.data_valid;
            break;

        case IPA_UNALIGNED:
            args->rd = c_args.rd_valid;
            args->ipa = g_unaligned_prep_sequence(c_args.ipa_valid);
            args->data = c_args.data_valid;
            break;

        case IPA_UNPROTECTED:
            args->rd = c_args.rd_valid;
            args->ipa = ipa_unprotected_unassigned_prep_sequence(c_args.rd_valid);
            if (args->ipa == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            args->data = c_args.data_valid;
            break;

        case IPA_OUTSIDE_OF_PERMITTED_IPA:
            args->rd = c_args.rd_valid;
            args->ipa = ipa_outside_of_permitted_ipa_prep_sequence();
            if (args->ipa == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            args->data = c_args.data_valid;
            break;

        case IPA_NOT_MAPPED:
            args->rd = c_args.rd_valid;
            args->ipa = ipa_protected_unmapped_prep_sequence();
            if (args->ipa == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            args->data = c_args.data_valid;
            break;

        case RTTE_STATE_ASSIGNED:
            args->rd = c_args.rd_valid;
            args->ipa = ipa_protected_assigned_ram_prep_sequence(c_args.rd_valid);
            if (args->ipa == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            args->data = c_args.data_valid;
            break;

        case RTTE_STATE_DESTROYED:
            args->rd = c_args.rd_valid;
            args->ipa = ipa_protected_destroyed_empty_prep_sequence(c_args.rd_valid);
            if (args->ipa == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            args->data = c_args.data_valid;
            break;

        case IPA_UNPROTECTED_NOT_MAPPED:
            args->rd = c_args.rd_valid;
            args->ipa = ipa_unprotected_unmapped_prep_sequence();
            if (args->ipa == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            args->data = c_args.data_valid;
            break;

        case IPA_UNPROTECTED_RTTE_ASSIGNED:
            args->rd = c_args.rd_valid;
            args->ipa = ipa_unprotected_assinged_prep_sequence(c_args.rd_valid);
            if (args->ipa == VAL_TEST_PREP_SEQ_FAILED)
                return VAL_ERROR;
            args->data = c_args.data_valid;
            break;

        default:
            val_set_status(RESULT_FAIL(VAL_ERROR_POINT(4)));
            LOG(ERROR, "\n\tUnknown intent label encountered\n", 0, 0);
            return VAL_ERROR;
    }
    return VAL_SUCCESS;
}

void cmd_data_create_unknown_host(void)
{
    uint64_t ret, i;
    struct arguments args;
    val_host_rtt_entry_ts rtte;

    if (valid_input_args_prep_sequence() == VAL_TEST_PREP_SEQ_FAILED) {
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(5)));
        goto fail;
    }

    for (i = 0; i < (sizeof(test_data) / sizeof(struct stimulus)); i++)
    {
        LOG(TEST, "\n\tCheck %d : ", i + 1, 0);
        LOG(TEST, test_data[i].msg, 0, 0);
        LOG(TEST, "; intent id : 0x%x \n", test_data[i].label, 0);


        if (intent_to_seq(&test_data[i], &args)) {
            val_set_status(RESULT_FAIL(VAL_ERROR_POINT(6)));
            goto fail;
        }

        ret = val_host_rmi_data_create_unknown(args.data, args.rd, args.ipa);

        if (ret != PACK_CODE(test_data[i].status, test_data[i].index))
        {
            LOG(ERROR, "\tERROR status code : %d index %d\n", test_data[i].status,
                                                              test_data[i].index);
            val_set_status(RESULT_FAIL(VAL_ERROR_POINT(7)));
            goto fail;
        }
    }

    LOG(TEST, "\n\tPositive Observability Check\n", 0, 0);
    ret = val_host_rmi_data_create_unknown(c_args.data_valid, c_args.rd_valid, c_args.ipa_valid);
    if (ret != 0)
    {
        LOG(ERROR, "\n\tData Create command failed.\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(8)));
        goto fail;
    }

    // Check that rtte.addr and rtte.state have changed
    ret = val_host_rmi_rtt_read_entry(c_args.rd_valid, c_args.ipa_valid,
                                      3, &rtte);
    if (ret) {
        LOG(ERROR, "\tREAD_ENTRY failed!\n", 0, 0);
    } else {
        if (rtte.state != RMI_ASSIGNED || OA(rtte.desc) != c_args.data_valid
                                                   || rtte.ripas != RMI_RAM)
        {
            LOG(ERROR, "\tState was: %d & OA was: %x\n", rtte.state, OA(rtte.desc));
            val_set_status(RESULT_FAIL(VAL_ERROR_POINT(9)));
            goto fail;
        }
    }

    val_set_status(RESULT_PASS(VAL_SUCCESS));
    return;

fail:
    val_set_status(RESULT_FAIL(VAL_ERROR_POINT(10)));
    return;
}
