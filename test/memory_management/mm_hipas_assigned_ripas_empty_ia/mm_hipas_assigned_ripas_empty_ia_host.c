/*
 * Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "test_database.h"
#include "val_host_rmi.h"

#define PROTECTED_IPA 0x800000

void mm_hipas_assigned_ripas_empty_ia_host(void)
{
    val_host_realm_ts realm;
    val_host_rmifeatureregister0_ts features_0;
    uint64_t ret;
    val_host_rec_entry_ts *rec_entry = NULL;
    val_host_rec_exit_ts *rec_exit = NULL;
    uint64_t ripas_ipa, ripas_size;
    uint64_t end;
    uint64_t phys;
    val_data_create_ts data_create;

    val_memset(&realm, 0, sizeof(realm));
    val_memset(&features_0, 0, sizeof(features_0));

    features_0.s2sz = 40;
    val_memcpy(&realm.realm_feat_0, &features_0, sizeof(features_0));

    realm.hash_algo = RMI_HASH_SHA_256;
    realm.s2_starting_level = 0;
    realm.num_s2_sl_rtts = 1;
    realm.vmid = 0;
    realm.rec_count = 1;

    /* Populate realm with one REC*/
    if (val_host_realm_setup(&realm, false))
    {
        LOG(ERROR, "\tRealm setup failed\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(1)));
        goto destroy_realm;
    }

    data_create.size = PAGE_SIZE;
    phys = (uint64_t)val_host_mem_alloc(PAGE_SIZE, (2 * data_create.size));
    if (!phys)
    {
        LOG(ERROR, "\tval_host_mem_alloc failed\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(2)));
        goto destroy_realm;
    }

    data_create.src_pa = phys;
    data_create.target_pa = phys + data_create.size;
    data_create.ipa = PROTECTED_IPA;
    data_create.rtt_alignment = PAGE_SIZE;
    ret = val_host_map_protected_data_to_realm(&realm, &data_create);
    if (ret)
    {
        LOG(ERROR, "\tval_host_map_protected_data_to_realm failed\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(3)));
        goto destroy_realm;
    }

    /* Activate realm */
    if (val_host_realm_activate(&realm))
    {
        LOG(ERROR, "\tRealm activate failed\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(4)));
        goto destroy_realm;
    }

    rec_entry = &(((val_host_rec_run_ts *)realm.run[0])->entry);
    /* REC enter REC[0] execution */
    ret = val_host_rmi_rec_enter(realm.rec[0], realm.run[0]);
    if (ret)
    {
        LOG(ERROR, "\tRec enter failed, ret=%x\n", ret, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(5)));
        goto destroy_realm;
    } else if (val_host_check_realm_exit_host_call((val_host_rec_run_ts *)realm.run[0]))
    {
        LOG(ERROR, "\tREC_EXIT: HOST_CALL params mismatch\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(6)));
        goto destroy_realm;
    }

    /* Resume back REC[0] execution */
    ripas_ipa = PROTECTED_IPA;
    ripas_size = PAGE_SIZE;
    rec_entry->gprs[1] = ripas_ipa;
    rec_entry->gprs[2] = ripas_size;
    ret = val_host_rmi_rec_enter(realm.rec[0], realm.run[0]);
    if (ret)
    {
        LOG(ERROR, "\tRec enter failed, ret=%x\n", ret, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(7)));
        goto destroy_realm;
    } else if (val_host_check_realm_exit_ripas_change((val_host_rec_run_ts *)realm.run[0]))
    {
        LOG(ERROR, "\tRipas change req failed\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(8)));
        goto destroy_realm;
    }
    rec_exit = &(((val_host_rec_run_ts *)realm.run[0])->exit);
    if (!(rec_exit->base == ripas_ipa) || !(rec_exit->size == ripas_size))
    {
        LOG(ERROR, "\tRipas return params mismatch\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(9)));
        goto destroy_realm;
    }

    end = rec_exit->base;
    while (end < (rec_exit->base + rec_exit->size))
    {
        ret = val_host_rmi_rtt_set_ripas(realm.rd, realm.rec[0], end,
                                        VAL_RTT_MAX_LEVEL, rec_exit->ripas_value);
        if (ret)
        {
            LOG(ERROR, "\tRMI_RTT_SET_RIPAS failed, ret=%x\n", ret, 0);
            val_set_status(RESULT_FAIL(VAL_ERROR_POINT(10)));
            goto destroy_realm;
        }
        end += PAGE_SIZE;
    }

    /* Test Intent: Protected IPA, HIPAS=ASSIGNED, RIPAS=EMPTY insrtuction access
     * => SEA to Realm
     */
    ret = val_host_rmi_rec_enter(realm.rec[0], realm.run[0]);
    if (ret)
    {
        LOG(ERROR, "\tRec enter failed, ret=%x\n", ret, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(11)));
        goto destroy_realm;
    } else if (val_host_check_realm_exit_host_call((val_host_rec_run_ts *)realm.run[0]))
    {
        LOG(ERROR, "\tREC_EXIT: HOST_CALL params mismatch\n", 0, 0);
        val_set_status(RESULT_FAIL(VAL_ERROR_POINT(12)));
        goto destroy_realm;
    }

    val_set_status(RESULT_PASS(VAL_SUCCESS));

    /* Free test resources */
destroy_realm:
    return;
}
