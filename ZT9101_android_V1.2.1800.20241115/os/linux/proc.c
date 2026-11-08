/*
 * proc.c
 *
 * used for print debugging information
 *
 * Author: pansiwei
 *
 * Copyright (c) 2021 Shandong ZTop Microelectronics Co., Ltd
 *
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "ndev_linux.h"
#include "proc.h"
#include "common.h"
#include "hif.h"

#ifdef CONFIG_MP_MODE
#include "proc_trx.h"
#endif

#define __user

static zt_s32 zt_get_version_info(struct seq_file *m, void *v)
{
    hif_mngent_st *hif_mngent = hif_mngent_get();
#ifdef COMPILE_TIME
    zt_print_seq(m, "Driver Ver:%s, Compile time:%s\n", ZT_VERSION, COMPILE_TIME);
    zt_print_seq(m, "Firmware Ver:%s\n", hif_mngent->fw_path);
#else
    zt_print_seq(m, "Driver Ver:%s\n", ZT_VERSION);
    zt_print_seq(m, "Firmware Ver:%s\n", hif_mngent->fw_path);
#endif
    return 0;
}

static zt_s32 zt_wiphy_unregister_info(struct seq_file *m, void *v)
{
    nic_info_st *pnic_info;
    hif_node_st *hif_info  = m->private;
    zt_u8 i = 0;

    if (NULL == hif_info)
    {
        LOG_W("[%s] hif_info is null", __func__);
        return -1;
    }

    for (i = 0; i < hif_info->nic_number; i++)
    {
        pnic_info = hif_info->nic_info[i];
        if (NULL == pnic_info)
        {
            LOG_W("[%s] pnic_info is null", __func__);
            continue;
        }

        ndev_shutdown(pnic_info);
        ndev_unregister(pnic_info);
    }

    return 0;
}

static zt_s32 zt_get_connect_info(struct seq_file *m, void *v)
{
    nic_info_st *pnic_info;
    hif_node_st *hif_info  = m->private;
    mlme_info_t *pmlme_info;

    if (NULL == hif_info)
    {
        LOG_W("[%s] hif_info is null", __func__);
        return -1;
    }

    pnic_info = hif_info->nic_info[0];
    if (NULL == pnic_info)
    {
        LOG_W("[%s] pnic_info is null", __func__);
        return -1;
    }
    pmlme_info = pnic_info->mlme_info;
    if (pmlme_info == NULL)
    {
        LOG_W("[%s] pmlme_info is null", __func__);
        return -1;
    }

    if (pmlme_info->connect)
    {
        zt_print_seq(m, "state=0x%x\n", pmlme_info->connect);
    }
    else
    {
        zt_print_seq(m, "no connection\n");
    }

    return 0;
}

static zt_s32 zt_get_wlan_mgmt_info(struct seq_file *m, void *v)
{
    nic_info_st *pnic_info;
    hif_node_st *hif_info  = m->private;
    zt_wlan_mgmt_info_t *pwlan_mgmt_info;

    if (NULL == hif_info)
    {
        LOG_W("[%s] hif_info is null", __func__);
        return -1;
    }

    pnic_info = hif_info->nic_info[0];
    if (NULL == pnic_info)
    {
        LOG_W("[%s] pnic_info is null", __func__);
        return -1;
    }

    pwlan_mgmt_info = pnic_info->wlan_mgmt_info;
    /* ap message free queue */
    if (NULL == pwlan_mgmt_info)
    {
        LOG_W("[%s] pwlan_mgmt_info is null", __func__);
        return -1;
    }

    {
        zt_wlan_mgmt_scan_que_t *pscan_que = &pwlan_mgmt_info->scan_que;
        zt_print_seq(m, "pscan_que->free.count=%d\n",
                     zt_que_count(&pscan_que->free));
        zt_print_seq(m, "pscan_que->ready.count=%d\n",
                     zt_que_count(&pscan_que->ready));
        zt_print_seq(m, "pscan_que->read_cnt=%d\n", pscan_que->read_cnt);
        if (0)
        {
            zt_wlan_mgmt_scan_que_node_t *pscan_que_node;
            zt_wlan_mgmt_scan_que_for_rst_e scan_que_for_rst;
            zt_print_seq(m, "-------------------------\n");
            zt_wlan_mgmt_scan_que_for_begin(pnic_info, pscan_que_node)
            {
                zt_print_seq(m, "sig_str: %d, ssid: %s\n",
                             pscan_que_node->signal_strength_scale,
                             pscan_que_node->ssid.data);
            }
            zt_wlan_mgmt_scan_que_for_end(scan_que_for_rst);
        }
    }

    return 0;
}


static zt_s32 zt_get_mlme_info(struct seq_file *m, void *v)
{
    nic_info_st *pnic_info;
    hif_node_st *hif_info = m->private;
    mlme_info_t *pmlme_info;

    if (NULL == hif_info)
    {
        LOG_W("[%s] hif_info is null", __func__);
        return -1;
    }

    pnic_info = hif_info->nic_info[0];
    if (NULL == pnic_info)
    {
        LOG_W("[%s] pnic_info is null", __func__);
        return -1;
    }

    pmlme_info = pnic_info->mlme_info;
    /* ap message free queue */
    if (NULL == pmlme_info)
    {
        LOG_W("[%s] pnic_info->mlme_info is null", __func__);
        return -1;
    }

    {
        zt_print_seq(m, "pmlme_info->link_info.busy_traffic=%d\n",
                     pmlme_info->link_info.busy_traffic);
    }

    return 0;
}

static uint8_t proc_bgn_rate_prase(char *param)
{
    uint8_t i;
    static const char * const bgn_rate_string[] = {
        "1",
        "2",
        "5.5",
        "11",
        "6",
        "9",
        "12",
        "18",
        "24",
        "36",
        "48",
        "54",
        "mcs0",
        "mcs1",
        "mcs2",
        "mcs3",
        "mcs4",
        "mcs5",
        "mcs6",
        "mcs7",
    };

    for (i = 0; i < ARRAY_SIZE(bgn_rate_string); i++)
    {
        if (zt_strncmp(param, bgn_rate_string[i], zt_strlen(bgn_rate_string[i])) == 0)
            return i;
    }

    LOG_E("[%s][%d]the comparison fails, the default value is used", __func__, __LINE__);

    return 0;
}

static ssize_t zt_set_rate(struct file *file, const char __user *buffer,
                             size_t count, loff_t *pos, void *data)
{
    hif_node_st *hif_info           = data;
    nic_info_st *pnic_info          = NULL;
    char *pch = NULL;
    zt_s8 *extra = NULL;
    char cmd[60] = {0};
    hw_info_st *hw_info;

    if (copy_from_user(cmd, buffer, count))
    {
        LOG_E("copy_from_user fail");

        return -ZT_RETURN_FAIL;
    }

    pnic_info = hif_info->nic_info[0];
    hw_info = pnic_info->hw_info;

    pch = cmd;
    LOG_D("input: %s", cmd);

    if (strncmp(pch, "auto", 4) == 0)
    {
        hw_info->use_fixRate = zt_false;
        hw_info->fix_tx_rate = 0;
    }
    else
    {
        extra = strsep(&pch, "=");
        hw_info->use_fixRate = zt_true;
        hw_info->fix_tx_rate = proc_bgn_rate_prase(pch);
        LOG_D("hw_info->fix_tx_rate = %d", hw_info->fix_tx_rate);
    }

    return count;
}

static ssize_t zt_chip_reset(struct file *file, const char __user *buffer,
                             size_t count, loff_t *pos, void *data)
{
    hif_node_st *hif_info           = data;
    nic_info_st *pnic_info          = NULL;
    zt_s32 i                        = 0;
    char tmp[32] = { 0 };

    if (count < 1)
    {
        return -EINVAL;
    }

    if (NULL == hif_info)
    {
        return -EINVAL;
    }
    LOG_I("node:%d,type:%d", hif_info->node_id, hif_info->hif_type);

    if (count > sizeof(tmp))
    {
        LOG_E("input param len is out of range");
        return -EFAULT;
    }

    if (buffer && !copy_from_user(tmp, buffer, count))
    {
        if (!zt_strcmp(tmp, "bb" ZT_FILE_EOF))
        {
            LOG_I("bb reset");
            zt_mcu_reset_bb(hif_info->nic_info[0]);
        }
        else if (!zt_strcmp(tmp, "mac" ZT_FILE_EOF))
        {
            LOG_I("mac reset");
            for (i = 0; i < hif_info->nic_number; i++)
            {
                pnic_info = hif_info->nic_info[i];
                if (NULL == pnic_info)
                {
                    continue;
                }

                if (ZT_CANNOT_RUN(pnic_info))
                {
                    return -2;
                }

                pnic_info->is_driver_critical = zt_true;
                zt_mlme_suspend(pnic_info);
#ifdef CFG_ENABLE_AP_MODE
                zt_ap_suspend(pnic_info);
#endif
            }

            hif_chip_reset(hif_info);

            for (i = 0; i < hif_info->nic_number; i++)
            {
                pnic_info = hif_info->nic_info[i];
                if (NULL == pnic_info)
                {
                    continue;
                }
                pnic_info->is_driver_critical = zt_false;
                zt_mlme_resume(pnic_info);
#ifdef CFG_ENABLE_AP_MODE
                zt_ap_resume(pnic_info);
#endif
            }
        }
        else
        {
            LOG_I("unknown cmd \"%s\"", tmp);
        }
    }
    else
    {
        return -EFAULT;
    }

    return count;
}

static zt_s32 zt_get_rx_info(struct seq_file *m, void *v)
{
    hif_node_st *hif_info           = m->private;
    wdn_net_info_st *wdn_net_info   = NULL;
    data_queue_node_st *data_node   = NULL;
    zt_s32 i                        = 0;
    if (NULL == hif_info)
    {
        LOG_E("[%s] hif_info is null", __func__);
        return -1;
    }


    /*hif debug info*/
    zt_print_seq(m, "node_id:%d\n", hif_info->node_id);
    zt_print_seq(m, "hif_type:%d\n", hif_info->hif_type);
    zt_print_seq(m, "rx_queue_cnt:%lld\n", hif_info->trx_pipe.rx_queue_cnt);
    zt_print_seq(m, "free rx data queue node num:%d\n",
                 hif_info->trx_pipe.free_rx_queue.cnt);

    for (i = 0; i < ZT_RX_MAX_DATA_QUEUE_NODE_NUM; i++)
    {
        data_node = hif_info->trx_pipe.all_rx_queue + i;
        if (0 != data_node->state)
        {
            zt_print_seq(m, "[%d] state:%d, pg_num:%d,agg_num:%d\n",
                         data_node->node_id, data_node->state,
                         data_node->pg_num, data_node->agg_num);
        }
    }

    zt_print_seq(m, "rx skb queue_len:%d\n",
                 skb_queue_len(&hif_info->trx_pipe.rx_queue));
    zt_print_seq(m, "free rx skb queue_len:%d\n",
                 skb_queue_len(&hif_info->trx_pipe.free_rx_queue_skb));

    if (NULL != wdn_net_info)
    {
        zt_s32 tid = 0;
        if (wdn_net_info->ba_ctl != NULL)
        {
            for (tid = 0; tid < TID_NUM; tid++)
            {
                recv_ba_ctrl_st *ba_ctl = &wdn_net_info->ba_ctl[tid];
                if (NULL != ba_ctl && zt_true == ba_ctl->enable)
                {
                    zt_print_seq(m, "[%d] rx reorder drop:%lld\n",
                                 tid, ba_ctl->drop_pkts);
                    zt_print_seq(m, "[%d] timeout_cnt:%u\n",
                                 tid, ba_ctl->timeout_cnt);
                }
            }
        }
    }

    return 0;
}

static ssize_t zt_set_tx_info(struct file *file, const char __user *buffer,
                              size_t count, loff_t *pos, void *data)
{
#if LINUX_VERSION_CODE == KERNEL_VERSION(4, 4, 13)
#define MAX_NIC 5
    hif_node_st *hif_info = data;
    nic_info_st *pnic_info = NULL;
    char tmp[32];
    zt_s32 ndev_id;
    zt_s32 set_id;
    zt_s32 val;
    if (count < 1)
    {
        return -EINVAL;
    }

    if (NULL == hif_info)
    {
        return -EINVAL;
    }
    else
    {
        LOG_I("node:%d,type:%d", hif_info->node_id, hif_info->hif_type);
    }

    if (count > sizeof(tmp))
    {
        LOG_E("input param len is out of range");
        return -EFAULT;
    }

    if (buffer && !copy_from_user(tmp, buffer, count))
    {
        zt_s32 num = sscanf(tmp, "%d %d %d", &ndev_id, &set_id, &val);
        if (num == 3)
        {
            LOG_I("ndev_id:%d, set_id:%d, val:%d\n", ndev_id, set_id, val);
            if (ndev_id < MAX_NIC)
            {
                pnic_info = hif_info->nic_info[ndev_id];
                if (pnic_info)
                {
                    if (zt_wdn_get_cnt(pnic_info)) // assosicated
                    {
                        zt_list_t *pos = NULL;
                        zt_list_t *next = NULL;
                        wdn_node_st *tmp_node = NULL;
                        wdn_list *wdn = (wdn_list *)pnic_info->wdn;
                        if (0 == set_id) //tx_rate
                        {
                            zt_list_for_each_safe(pos, next, &wdn->head)
                            {
                                tmp_node = zt_list_entry(pos, wdn_node_st, list);
                                if (tmp_node)
                                {
                                    tmp_node->info.tx_rate = val;
                                    LOG_I("wdn_id:%d, tx_rate:%d",
                                          tmp_node->info.wdn_id, val);
                                }
                                tmp_node = NULL;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        return -EFAULT;
    }
#endif
    return count;
}

static zt_s32 zt_get_tx_info(struct seq_file *m, void *v)
{
    hif_node_st *hif_info           = m->private;
    nic_info_st *pnic_info          = NULL;
    tx_info_st *tx_info             = NULL;
    data_queue_node_st *data_node   = NULL;
    zt_s32 i                        = 0;
    if (NULL == hif_info)
    {
        LOG_E("[%s] hif_info is null", __func__);
        return -1;
    }


    /*hif debug info*/
    zt_print_seq(m, "node_id:%d\n", hif_info->node_id);
    zt_print_seq(m, "hif_type:%d\n", hif_info->hif_type);
    zt_print_seq(m, "tx_queue_cnt:%lld\n", hif_info->trx_pipe.tx_queue_cnt);
    zt_print_seq(m, "free tx data queue node num:%d\n",
                 hif_info->trx_pipe.free_tx_queue.cnt);
    zt_print_seq(m, "tx data queue node num:%d\n",
                 hif_info->trx_pipe.tx_queue.cnt);

    for (i = 0; i < ZT_TX_MAX_DATA_QUEUE_NODE_NUM; i++)
    {
        data_node = hif_info->trx_pipe.all_tx_queue + i;
        if ((TX_STATE_COMPETE != data_node->state) &&
                (TX_STATE_IDL != data_node->state))
        {
            zt_print_seq(m, "[%d] state:%d, pg_num:%d,agg_num:%d, addr:0x%x\n",
                         data_node->node_id, data_node->state, data_node->pg_num,
                         data_node->agg_num, data_node->addr);
        }
    }

    if (HIF_SDIO == hif_info->hif_type)
    {
        hif_sdio_st *sd = &hif_info->u.sdio;

        zt_print_seq(m, "tx_fifo_ppg_num    :%d\n", sd->tx_fifo_ppg_num);
        zt_print_seq(m, "tx_fifo_hpg_num    :%d\n", sd->tx_fifo_hpg_num);
        zt_print_seq(m, "tx_fifo_lpg_num    :%d\n", sd->tx_fifo_lpg_num);
    }

    pnic_info = hif_info->nic_info[0];
    if (NULL == pnic_info)
    {

        return 0;
    }

    tx_info = pnic_info->tx_info;
    zt_print_seq(m, "free tx frame num:%d,free_xmitbuf_cnt:%d\n",
                 tx_info->free_xmitframe_cnt, tx_info->free_xmitbuf_cnt);
    zt_print_seq(m, "data_queue_check:%d",
                 zt_io_write_data_queue_check(pnic_info));
    {
        zt_u8 is_empty;
        zt_mcu_check_tx_buff(pnic_info, &is_empty);
        zt_print_seq(m, "tx queue is empty: %d", is_empty);
    }
    return 0;
}


#ifdef CFG_ENABLE_AP_MODE
static zt_s32 zt_get_ap_info(struct seq_file *m, void *v)
{
    hif_node_st *hif_info  = m->private;
    nic_info_st *pnic_info = NULL;
    zt_wlan_mgmt_info_t *pwlan_info;
    zt_wlan_network_t *pcur_network;
    wdn_list *pwdn;
    wdn_net_info_st *pwdn_info;
    zt_list_t *pos, *pos_next;
    sec_info_st *psec_info = NULL;
    zt_s32 i = 0;

    if (NULL == hif_info)
    {
        LOG_E("[%s] hif_info is null", __func__);
        return -1;
    }

    for (i = 0; i < hif_info->nic_number; i++)
    {
        pnic_info = hif_info->nic_info[i];
        if (NULL == pnic_info)
        {
            continue;
        }

        if (ZT_CANNOT_RUN(pnic_info))
        {
            break;
        }

        zt_print_seq(m, "--------------nic[%d] ----------\n ", i);

        /* ap message free queue */
        pwlan_info = pnic_info->wlan_mgmt_info;
        if (pwlan_info)
        {
            pcur_network = &pwlan_info->cur_network;
            zt_print_seq(m, "ap_msg_free[ZT_AP_MSG_TAG_AUTH_FRAME].count=%d\n",
                         pcur_network->ap_msg_free[ZT_AP_MSG_TAG_AUTH_FRAME].cnt);
            zt_print_seq(m, "ap_msg_free[ZT_AP_MSG_TAG_DEAUTH_FRAME].count=%d\n",
                         pcur_network->ap_msg_free[ZT_AP_MSG_TAG_DEAUTH_FRAME].cnt);
            zt_print_seq(m, "ap_msg_free[ZT_AP_MSG_TAG_ASSOC_REQ_FRAME].count=%d\n",
                         pcur_network->ap_msg_free[ZT_AP_MSG_TAG_ASSOC_REQ_FRAME].cnt);
            zt_print_seq(m, "ap_msg_free[ZT_AP_MSG_TAG_DISASSOC_FRAME].count=%d\n",
                         pcur_network->ap_msg_free[ZT_AP_MSG_TAG_DISASSOC_FRAME].cnt);
            zt_print_seq(m, "ap_msg_free[ZT_AP_MSG_TAG_BA_REQ_FRAME].count=%d\n",
                         pcur_network->ap_msg_free[ZT_AP_MSG_TAG_BA_REQ_FRAME].cnt);
            zt_print_seq(m, "pcur_network->channel=%d\n", pcur_network->channel);
        }

        psec_info = pnic_info->sec_info;
        if (psec_info)
        {
            zt_print_seq(m, "psec_info->dot11AuthAlgrthm=%d\n",
                         psec_info->dot11AuthAlgrthm);
        }

        /* wdn message queue */
        pwdn = pnic_info->wdn;
        pwdn_info = pnic_info->wdn;
        if (pwdn)
        {
            zt_print_seq(m, "\npwdn->cnt=%d", pwdn->cnt);
            zt_list_for_each_safe(pos, pos_next, &pwdn->head)
            {
                pwdn_info = &zt_list_entry(pos, wdn_node_st, list)->info;
                zt_print_seq(m, "pwdn_info->wdn_id=%d\n", pwdn_info->wdn_id);
                zt_print_seq(m, "         ->channel=%d\n", pwdn_info->channel);
                zt_print_seq(m, "         ->mac="ZT_MAC_FMT"\n",
                             ZT_MAC_ARG(pwdn_info->mac));
                zt_print_seq(m, "         ->ieee8021x_blocked=%d\n",
                             pwdn_info->ieee8021x_blocked);
                zt_print_seq(m, "         ->dot118021XPrivacy=%d\n",
                             pwdn_info->dot118021XPrivacy);
                zt_print_seq(m, "         ->ap_msg.count=%d\n",
                             pwdn_info->ap_msg.cnt);
                zt_print_seq(m, "         ->ap_msg.rx_pkt_stat=%d\n",
                             pwdn_info->rx_pkt_stat);
                zt_print_seq(m, "         ->psm=%d\n",
                             pwdn_info->psm);
                zt_print_seq(m, "         ->psm_data_que.cnt=%d\n",
                             zt_que_count(&pwdn_info->psm_data_que));
            }
        }
    }
    return 0;
}


#endif
static zt_s32 zt_get_sta_info(struct seq_file *m, void *v)
{
    nic_info_st *pnic_info = NULL;
    hif_node_st *hif_info  = m->private;
    wdn_list *pwdn;
    wdn_net_info_st *pwdn_info;
    zt_list_t *pos, *pos_next;
    sec_info_st *psec_info = NULL;

    if (NULL == hif_info)
    {
        LOG_E("[%s] hif_info is null", __func__);
        return -1;
    }

    pnic_info = hif_info->nic_info[0];
    if (NULL == pnic_info)
    {
        LOG_E("[%s] pnic_info is null", __func__);
        return -1;
    }

    psec_info = pnic_info->sec_info;
    /* ap message free queue */
    if (psec_info)
    {
        zt_print_seq(m, "psec_info->dot11AuthAlgrthm=%d\n",
                     psec_info->dot11AuthAlgrthm);
    }

    /* wdn message queue */
    pwdn = pnic_info->wdn;
    pwdn_info = pnic_info->wdn;
    if (pwdn)
    {
        zt_print_seq(m, "\npwdn->cnt=%d", pwdn->cnt);
        zt_list_for_each_safe(pos, pos_next, &pwdn->head)
        {
            pwdn_info = &zt_list_entry(pos, wdn_node_st, list)->info;
            zt_print_seq(m, "pwdn_info->wdn_id=%d\n", pwdn_info->wdn_id);
            zt_print_seq(m, "         ->mac="ZT_MAC_FMT"\n",
                         ZT_MAC_ARG(pwdn_info->mac));
            zt_print_seq(m, "         ->ieee8021x_blocked=%d\n",
                         pwdn_info->ieee8021x_blocked);
            zt_print_seq(m, "         ->dot118021XPrivacy=%d\n",
                         pwdn_info->dot118021XPrivacy);
        }
    }

    return 0;
}

static zt_s32 zt_get_hif_info(struct seq_file *m, void *v)
{
    hif_node_st *hif_info           = m->private;
    zt_s32 i = 0;
    data_queue_node_st *data_node   = NULL;

    if (NULL == hif_info)
    {
        LOG_E("[%s] hif_info is null", __func__);
        return -1;
    }

    /*hif debug info*/
    zt_print_seq(m, "node_id : %d, nic_num:%d\n", hif_info->node_id,
                 hif_info->nic_number);
    zt_print_seq(m, "hif_type: %s\n",
                 hif_info->hif_type == 1 ? "HIF_USB" : "HIF_SDIO");

    /*hif--rx info*/
    zt_print_seq(m, "[rx] all  queue cnt:%lld\n",
                 hif_info->trx_pipe.rx_queue_cnt);
    zt_print_seq(m, "[rx] free queue node num:%d\n",
                 hif_info->trx_pipe.free_rx_queue.cnt);

    for (i = 0; i < ZT_RX_MAX_DATA_QUEUE_NODE_NUM; i++)
    {
        data_node = hif_info->trx_pipe.all_rx_queue + i;
        if (0 != data_node->state)
        {
            zt_print_seq(m, "[rx] qnode(%d) state:%d, pg_num:%d,agg_num:%d\n",
                         data_node->node_id, data_node->state,
                         data_node->pg_num, data_node->agg_num);
        }
    }

    /*hif--tx info*/
    zt_print_seq(m, "[tx] all queue cnt:%lld\n", hif_info->trx_pipe.tx_queue_cnt);
    zt_print_seq(m, "[tx] free  tx data queue node num:%d\n",
                 hif_info->trx_pipe.free_tx_queue.cnt);
    zt_print_seq(m, "[tx] using tx data queue node num:%d\n",
                 hif_info->trx_pipe.tx_queue.cnt);
    for (i = 0; i < ZT_TX_MAX_DATA_QUEUE_NODE_NUM; i++)
    {
        data_node = hif_info->trx_pipe.all_tx_queue + i;
        if ((TX_STATE_COMPETE != data_node->state) &&
                (TX_STATE_IDL != data_node->state))
        {
            zt_print_seq(m, "[tx] qnode(%d) state:%d, pg_num:%d,agg_num:%d, addr:0x%x\n",
                         data_node->node_id, data_node->state,
                         data_node->pg_num, data_node->agg_num, data_node->addr);
        }
    }
    if (HIF_SDIO == hif_info->hif_type)
    {
        hif_sdio_st *sd = &hif_info->u.sdio;

        zt_print_seq(m, "[tx] fifo_ppg_num    :%d\n", sd->tx_fifo_ppg_num);
        zt_print_seq(m, "[tx] fifo_hpg_num    :%d\n", sd->tx_fifo_hpg_num);
        zt_print_seq(m, "[tx] fifo_lpg_num    :%d\n", sd->tx_fifo_lpg_num);
        zt_print_seq(m, "[tx] tx_state:%d\n", sd->tx_state);
    }

    /*register info*/
    {
        nic_info_st *pnic_info = hif_info->nic_info[0];
        if (NULL == pnic_info)
        {
            LOG_E("[%s] pnic_info is null", __func__);
            return -1;
        }
    }
    return 0;
}

static zt_s32 mp_proc_num0fstr(zt_s8 *Mstr, zt_s8 *substr)
{
    zt_s32 number = 0;
    zt_s8 *p;
    zt_s8 *q;

    while (*Mstr != '\0')
    {
        p = Mstr;
        q = substr;

        while ((*p == *q) && (*p != '\0') && (*q != '\0'))
        {
            p++;
            q++;
        }
        if (*q == '\0')
        {
            number++;
        }
        Mstr++;
    }

    return number;
}

static zt_u8 mp_proc_key_of_char2num_func(zt_u8 ch)
{
    if ((ch >= '0') && (ch <= '9'))
    {
        return ch - '0';
    }
    else if ((ch >= 'a') && (ch <= 'f'))
    {
        return ch - 'a' + 10;
    }
    else if ((ch >= 'A') && (ch <= 'F'))
    {
        return ch - 'A' + 10;
    }
    else
    {
        return 0xff;
    }
}

static zt_u8 mp_proc_key_of_2char2num_func(zt_u8 hch, zt_u8 lch)
{
    return ((mp_proc_key_of_char2num_func(hch) << 4) |
            mp_proc_key_of_char2num_func(lch));
}

static zt_s32 mp_proc_rfthermalmeter_get(nic_info_st *pnic_info, char *data,
        size_t count)
{
    zt_u32 value1;

    mcu_cmd_communicate(pnic_info, UMSG_OPS_HAL_TEMP_SET, NULL, 0, NULL, 0);
    zt_msleep(1000);
    mcu_cmd_communicate(pnic_info, UMSG_OPS_HAL_TEMP_GET, NULL, 0, &value1, 1);
    LOG_D("%x", value1);
    LOG_D("temp:%x", value1);
    LOG_D("thermal=%02x", value1);

    return 0;
}

zt_s32 mp_proc_hw_init(nic_info_st *pnic_info)
{
    zt_s32 ret;

    ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_INIT, NULL, 0, NULL, 0);
    if (ret == ZT_RETURN_FAIL)
    {
        LOG_W("set reg fail");

        return -1;
    }

    return ZT_RETURN_OK;
}

#ifdef CONFIG_MP_MODE
/* mp */

static zt_s32 mp_proc_set_rate(nic_info_st *pnic_info, char *data, size_t count)
{
    zt_mp_info_st *pmp_info = pnic_info->mp_info;
    zt_u32 rate = 1;
    zt_u32 ret = 0;

    rate = zt_atoi(data);
    LOG_D("set rate=%d", rate);

    pmp_info->rateidx = zt_mp_proc_rate_to_rateidx(rate);

    LOG_D("Set data rate rate:0x%x -> rateIdx:%d  %s", rate,
          pmp_info->rateidx, ret == 0 ? "ok" : "fail");

    return 0;
}

static zt_s32 mp_proc_set_gi(nic_info_st *pnic_info, char *data, size_t count)
{
    zt_mp_info_st *pmp_info = pnic_info->mp_info;

    zt_u32 gi = 0;
    zt_u32 ret;

    gi = zt_atoi(data);
    LOG_D("set gi=%d", gi);

    if (pmp_info == NULL)
    {
        LOG_W("mp_info NULL");
        return ZT_RETURN_FAIL;
    }
    if (gi == 0)
    {
        pmp_info->preamble = PREAMBLE_SHORT;
    }
    else if (gi == 1)
    {
        pmp_info->preamble = PREAMBLE_LONG;
    }

    ret = zt_mcu_set_preamble(pnic_info, pmp_info->preamble);
    if (ret == ZT_RETURN_FAIL)
    {
        LOG_D("set gi fail");
    }
    else
    {
        LOG_D("set gi ok");
    }

    return 0;
}

static zt_s32 mp_proc_set_bw(nic_info_st *pnic_info, char *data, size_t count)
{
    local_info_st *local_info = pnic_info->local_info;
    zt_mp_info_st *mp_info = pnic_info->mp_info;

    zt_u32 ret;
    zt_u32 bandwidth = 0;

    bandwidth = zt_atoi(data);
    LOG_D("set bw=%d", bandwidth);

    if (bandwidth == 1)
    {
        mp_info->bandwidth = CHANNEL_WIDTH_40;

    }
    else if (bandwidth == 0)
    {
        mp_info->bandwidth = CHANNEL_WIDTH_20;
    }

    ret = zt_hw_info_set_channel_bw(pnic_info, mp_info->channel,
                                     mp_info->bandwidth, HAL_PRIME_CHNL_OFFSET_DONT_CARE);

    LOG_D("change bw %d to bw %d  %s\n", local_info->bw, bandwidth,
          ret == 0 ? "ok" : "fail");

    return 0;
}

static zt_s32 mp_proc_set_channel(nic_info_st *pnic_info, char *data,
                                  size_t count)
{
    zt_wlan_mgmt_info_t *pwlan_info = pnic_info->wlan_mgmt_info;
    zt_wlan_network_t *pcur_network = &pwlan_info->cur_network;
    zt_mp_info_st *mp_info = pnic_info->mp_info;
    zt_u32 channel = 1;
    zt_u32 ret;

    if ((pnic_info->is_surprise_removed) || (pnic_info->is_driver_stopped))
    {
        return -1;
    }

    channel = zt_atoi(data);
    LOG_D("set  channel=%d", channel);

    LOG_D("Change channel %d to channel %d", pcur_network->channel, channel);

    mp_info->channel = channel;
    ret = zt_hw_info_set_channel_bw(pnic_info, mp_info->channel,
                                     mp_info->bandwidth, HAL_PRIME_CHNL_OFFSET_DONT_CARE);
    LOG_D("set channel %s\n", ret == 0 ? "ok" : "fail");

    return 0;
}

static zt_s32 mp_proc_phy_set_txpower(nic_info_st *pnic_info, zt_u32 txpower)
{
    zt_u32 inbuf = txpower;

    return mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_PROSET_TXPWR_1, &(inbuf), 1,
                               NULL, 0);
}

static zt_s32 mp_proc_set_tx_power(nic_info_st *pnic_info, char *data,
                                   size_t count)
{
    zt_mp_info_st *pmp_info = pnic_info->mp_info;

    zt_u32 idx_a = 0;
    zt_s32 MsetPower = 1;
    zt_s32 ret = 0;

    MsetPower = zt_strncmp(data, "off", 3);

    if (MsetPower == 0)
    {
        pmp_info->bSetTxPower = 0;
        LOG_D("Test Set power off");
    }
    else
    {
        idx_a = zt_atoi(data);

        pmp_info->txpoweridx = (zt_u8) idx_a;

        LOG_D("============> %d\n", idx_a);
        pmp_info->bSetTxPower = 1;
        LOG_D("set tx_power0=%d", idx_a);

        ret = mp_proc_phy_set_txpower(pnic_info, idx_a);
    }
    LOG_D("Set power:%d  %s", idx_a, ret == 0 ? "ok" : "fail");

    return 0;
}

static zt_u32 mp_proc_init(nic_info_st *pnic_info)
{
    zt_mp_info_st *pmppriv;
    zt_wlan_mgmt_info_t *pwlan_info = pnic_info->wlan_mgmt_info;
    zt_wlan_network_t *pcur_network = &pwlan_info->cur_network;
    zt_proc_mp_tx *pmptx ;
    struct xmit_frame *pattrib;
    rx_info_t *rx_info = pnic_info->rx_info;

    pmppriv = zt_kzalloc(sizeof(zt_mp_info_st));
    if (pmppriv == NULL)
    {
        LOG_E("[SCAN] malloc scan_param_st failed");

        return -1;
    }
    else
    {
        pnic_info->mp_info = pmppriv;
    }

    rx_info->rx_crcerr_pkt = 0;
    rx_info->rx_pkts = 0;
    rx_info->rx_total_pkts = 0;
    zt_memset(pmppriv, 0, sizeof(zt_mp_info_st));
    //phw_info->use_fixRate = zt_true;

    pmppriv->mode = MP_OFF;

    pmppriv->channel = 1;
    pmppriv->bandwidth = CHANNEL_WIDTH_20;
    pmppriv->prime_channel_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
    pmppriv->rateidx = 1;
    pmppriv->txpoweridx = 0x2A;

    pmppriv->antenna_tx = 8;
    pmppriv->antenna_rx = 8;

    pmppriv->check_mp_pkt = 0;

    pmppriv->tx_pktcount = 0;

    pmppriv->rx_bssidpktcount = 0;
    pmppriv->rx_pktcount = 0;
    pmppriv->rx_crcerrpktcount = 0;

    pmppriv->network_macaddr[0] = 0xb4;
    pmppriv->network_macaddr[1] = 0x04;
    pmppriv->network_macaddr[2] = 0x18;
    pmppriv->network_macaddr[3] = 0x00;
    pmppriv->network_macaddr[4] = 0x00;
    pmppriv->network_macaddr[5] = 0x02;

    pmppriv->bSetRxBssid = zt_false;
    pmppriv->bWLSmbCfg = zt_true;


    zt_memcpy(pcur_network->mac_addr, pmppriv->network_macaddr,
              ZT_80211_MAC_ADDR_LEN);

    pcur_network->ssid.length = 8;
    zt_memcpy(pcur_network->ssid.data, "mp_testadd", pcur_network->ssid.length);

    pmppriv->tx.payload = 2;

    pmppriv->mp_dm = 0;
    pmppriv->tx.stop = 1;
    pmppriv->bSetTxPower = 0;
    pmppriv->pktInterval = 0;
    pmppriv->pktLength = 1000;
    pmptx = &pmppriv->tx;

    pattrib = &pmptx->attrib;
    zt_memset(pattrib, 0, sizeof(struct xmit_frame));
    zt_memset(pmptx->desc, 0, TXDESC_SIZE);
#ifdef CONFIG_80211N_HT
    pattrib.ht_en = 1;
#endif

    pattrib->ether_type = 0x8712;
    //zt_memset(pmppriv->dst, 0xFF, ZT_80211_MAC_ADDR_LEN);

    pattrib->hdrlen = WLAN_HDR_A3_LEN;
    //pattrib->subtype = WIFI_DATA;
    pattrib->priority = 0;
    pattrib->qsel = pattrib->priority;
    pattrib->nr_frags = 1;
    pattrib->encrypt_algo = _NO_PRIVACY_;
    //pattrib->qos_en = zt_false;

    pattrib->pktlen = 0;
    pattrib->nic_info = pnic_info;
    pmppriv->antenna_tx = 8;
    pmppriv->antenna_rx = 8;
    pnic_info->is_up = 1;

    pmppriv->pnic_info = pnic_info;

    mp_proc_hw_init(pnic_info);

    //mlme_set_connect(pnic_info, zt_true);
    zt_mcu_set_op_mode(pnic_info, ZT_INFRA_MODE);
#ifdef CONFIG_SOFT_RX_AGGREGATION
    zt_mcu_set_usb_agg_normal(pnic_info, WIRELESS_11BG_24N);
#endif

    return 0;
}

static zt_s32 mp_proc_test_start(nic_info_st *pnic_info)
{
    zt_s32 ret;
    hw_info_st *phw_info = pnic_info->hw_info;
    zt_mp_info_st *pmp_info;
    zt_u32 inbuff;

    if ((pnic_info->is_surprise_removed) || (pnic_info->is_driver_stopped))
    {
        return -1;
    }

    if (phw_info->mp_mode == 0)
    {
        phw_info->mp_mode = 1;
        mp_proc_init(pnic_info);
    }

    pmp_info = pnic_info->mp_info;
    if (pmp_info->mode == MP_OFF)
    {
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_DIS_DM, NULL, 0, NULL, 0);
        if (ret == ZT_RETURN_FAIL)
        {
            LOG_W("set reg fail");
            return -1;
        }

        pmp_info->antenna_tx = 8;
        pmp_info->antenna_rx = 8;
        pmp_info->bStartContTx = zt_false;
        pmp_info->bCarrierSuppression = zt_false;
        pmp_info->bSingleTone = zt_false;

        pmp_info->mode = MP_ON;
    }

    pmp_info->bmac_filter = zt_false;

    inbuff = 0x26;
    ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MSG_WRITE_DIG, &inbuff, 1, NULL,
                              0);
    if (ret == ZT_RETURN_FAIL)
    {
        LOG_W("set reg fail");
        return ZT_RETURN_FAIL;
    }

    return 0;
}

static ssize_t mp_proc_wifi_set(struct file *file, const char __user *buffer,
                                size_t count, loff_t *pos, void *data)
{
    hif_node_st *hif_info = data;
    nic_info_st *pnic_info = NULL;
    zt_proc_st *proc_info = NULL;

    char *pch = NULL;
    zt_s8 *extra = NULL;
    char cmd[60] = {0};
    char cmd1[8][20] = {0};
    char data1[8][20] = {0};
    zt_s32 set_len = 0;
    zt_u16 cnts = 0;
    zt_s32 ret, i = 0;
    zt_u32 jj, kk;
    zt_s32 efuse_code = 1000;
    zt_bool EnableMAC = zt_false;
    char *ptmp, *token1, *value1 = NULL;
    zt_u8 *inbuff = NULL;
    zt_u32 outbuff = 0;
    zt_u8 *buff = NULL;
    pnic_info = hif_info->nic_info[0];
    proc_info = (zt_proc_st *)hif_info->proc_info;

    if (copy_from_user(cmd, buffer, count))
    {
        LOG_E("copy_from_user fail");

        return -ZT_RETURN_FAIL;
    }

    cmd[count] = '\0';
    extra = cmd;
    LOG_D("input: %s", cmd);
    pch = extra;

    if (proc_info->mp_proc_test_enable == zt_false &&
            zt_strncmp(cmd, "start", 5) != 0)
    {
        LOG_E("Please <echo \"start\" > /proc/net/wlan0/mp> first");

        return ZT_RETURN_FAIL;
    }

    if (zt_strncmp(cmd, "start", 5) == 0)
    {
        if (proc_info->mp_proc_test_enable == zt_true)
        {
            LOG_W("It's already started");

            return count;
        }
        ret = mp_proc_test_start(pnic_info);
        LOG_D("test_start %s\n", ret == 0 ? "ok" : "fail");
        proc_info->mp_proc_test_enable = zt_true;

        return count;
    }

    if (zt_strncmp(cmd, "mac_r", 5) == 0)
    {
        inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }
        efuse_code = WLAN_EEPORM_MAC;
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_GET, &efuse_code, 1,
                                  (zt_u32 *) inbuff, MAILBOX_MAX_TXLEN);
        if (ZT_RETURN_FAIL == ret)
        {
            LOG_W("[%s] read reg failed,check!!!", __func__);
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        LOG_D("efuse_code = %d, cnts = %d", efuse_code, inbuff[0]);

        for (i = 0; i < inbuff[0]; i++)
        {
            LOG_D("0x%02X", inbuff[i + 1]);
        }
        if (inbuff)
        {
            zt_kfree(inbuff);
        }

        return count;
    }
    else if (zt_strncmp(cmd, "mac_w", 5) == 0)
    {
        set_len = mp_proc_num0fstr(extra, "=");
        EnableMAC = zt_true;
        inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }
        if (set_len > 1)
        {
            LOG_E("set_len error");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        while (set_len--)
        {
            value1 = zt_strchr(cmd, '=');
            if (value1)
            {
                *value1++ = '\0';
                ptmp = cmd;
                token1 = value1;
                strcpy(cmd1[i], ptmp);
                zt_strncpy(data1[i], token1, zt_strlen(token1) - 1);
                LOG_D("cmd = %s", cmd1[i]);
                LOG_D("data1 = %s, test", data1[i]);
                i++;
            }
            else
            {
                LOG_W("[%s]:param error", __func__);

                return ZT_RETURN_FAIL;
            }
        }
        if (data1[0] != 0)
        {
            cnts = zt_strlen(data1[0]);
            LOG_D("cnts: %s, %d", data1[0], cnts);
            if (cnts % 2)
            {
                LOG_W("cnts error");

                return ZT_RETURN_FAIL;
            }

            cnts /= 2;
            LOG_D("%s: cnts=%d", __func__, cnts);
            LOG_D("%s: data=%s", __func__, data1[0]);

            inbuff[1] = 6;

            for (jj = 0, kk = 0; jj < cnts; jj++, kk += 2)
            {
                inbuff[jj + 2] = mp_proc_key_of_2char2num_func(data1[0][kk],
                                 data1[0][kk + 1]);
            }
        }
        inbuff[0] = 1;

        for (i = 0; i < 8; i++)
        {
            LOG_D("%x", inbuff[i]);
        }
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_SET, (zt_u32 *) inbuff,
                                  (cnts + 6) / 4, &outbuff, 1);
        if (ret == ZT_RETURN_FAIL)
        {
            LOG_W("set reg fail");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        if (outbuff == 0)
        {
            LOG_W("fail");
        }
        else
        {
            LOG_D("ok");
        }
        if (EnableMAC)
        {
            inbuff[0] = EFUSE_EnMAC;
            inbuff[1] = 1;
            inbuff[2] = 1;
            ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_SET, (zt_u32 *) inbuff,
                                      3, &outbuff, 1);
        }
        if (inbuff)
        {
            zt_kfree(inbuff);
        }

        return count;
    }
    else if (zt_strncmp(cmd, "thermal", 7) == 0)
    {
        set_len = mp_proc_num0fstr(extra, "=");
        if (set_len == 0)
        {
            ptmp = cmd;
            LOG_D("get case temp\n");
            mp_proc_rfthermalmeter_get(pnic_info, ptmp, count);
            efuse_code = EFUSE_TEMPCAL;
            buff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
            if (buff == NULL)
            {
                LOG_D("alloc recv buff fail");

                return  ZT_RETURN_FAIL;
            }
            ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_GET, &efuse_code, 1,
                                      (zt_u32 *) buff, MAILBOX_MAX_TXLEN);
            if (ZT_RETURN_FAIL == ret)
            {
                LOG_W("[%s] read reg failed,check!!!", __func__);
                if (buff)
                {
                    zt_kfree(buff);
                }

                return ZT_RETURN_FAIL;
            }

            for (i = 0; i < buff[0]; i++)
            {
                LOG_D("Set temperature = 0x%02X", buff[i + 1]);
            }
            if (buff)
            {
                zt_kfree(buff);
            }

            return count;
        }
        else if (set_len > 1)
        {
            LOG_E("set_len error");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }

        while (set_len--)
        {
            value1 = zt_strchr(cmd, '=');
            if (value1)
            {
                *value1++ = '\0';
                ptmp = cmd;
                token1 = value1;
                strcpy(cmd1[i], ptmp);
                zt_strncpy(data1[i], token1, zt_strlen(token1) - 1);
                LOG_D("cmd = %s", cmd1[i]);
                LOG_D("data1 = %s, test", data1[i]);
                i++;
            }
            else
            {
                LOG_W("[%s]:param error", __func__);

                return ZT_RETURN_FAIL;
            }
        }
        if (data1[0] != 0)
        {
            cnts = zt_strlen(data1[0]);
            LOG_D("cnts: %s, %d", data1[0], cnts);
            if (cnts % 2)
            {
                LOG_W("cnts error");

                return ZT_RETURN_FAIL;
            }

            cnts /= 2;
            LOG_D("%s: cnts=%d", __func__, cnts);
            LOG_D("%s: data=%s", __func__, data1[0]);

            inbuff[1] = cnts;

            for (jj = 0, kk = 0; jj < cnts; jj++, kk += 2)
            {
                inbuff[jj + 2] = mp_proc_key_of_2char2num_func(data1[0][kk],
                                 data1[0][kk + 1]);
            }
        }
        inbuff[0] = EFUSE_TEMPCAL;

        for (i = 0; i < 3; i++)
        {
            LOG_D("%x", inbuff[i]);
        }
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_SET, (zt_u32 *) inbuff,
                                  (cnts + 6) / 4, &outbuff, 1);
        if (ret == ZT_RETURN_FAIL)
        {
            LOG_W("set reg fail");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        if (outbuff == 0)
        {
            LOG_W("fail");
        }
        else
        {
            LOG_D("ok");
        }
        if (inbuff)
        {
            zt_kfree(inbuff);
        }
    }
    else if (zt_strncmp(cmd, "freq_efuse", 10) == 0)
    {
        set_len = mp_proc_num0fstr(extra, "=");
        if (set_len == 0)
        {
            inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
            efuse_code = EFUSE_FREQCAL;
            LOG_D("get case freq\n");
            ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_GET, &efuse_code, 1,
                                      (zt_u32 *) inbuff, MAILBOX_MAX_TXLEN);
            if (ZT_RETURN_FAIL == ret)
            {
                LOG_W("[%s] read reg failed,check!!!", __func__);
                if (inbuff)
                {
                    zt_kfree(inbuff);
                }

                return ZT_RETURN_FAIL;
            }
            LOG_D("efuse_code = %d, cnts = %d", efuse_code, inbuff[0]);

            for (i = 0; i < inbuff[0]; i++)
            {
                LOG_D("0x%02X", inbuff[i + 1]);
            }
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return count;
        }
        else if (set_len > 1)
        {
            LOG_E("set_len error");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }
        while (set_len--)
        {
            value1 = zt_strchr(cmd, '=');
            if (value1)
            {
                *value1++ = '\0';
                ptmp = cmd;
                token1 = value1;
                strcpy(cmd1[i], ptmp);
                zt_strncpy(data1[i], token1, zt_strlen(token1) - 1);
                LOG_D("cmd = %s", cmd1[i]);
                LOG_D("data1 = %s, test", data1[i]);
                i++;
            }
            else
            {
                LOG_W("[%s]:param error", __func__);

                return ZT_RETURN_FAIL;
            }
        }
        if (data1[0] != 0)
        {
            cnts = zt_strlen(data1[0]);
            LOG_D("cnts: %s, %d", data1[0], cnts);
            if (cnts % 2)
            {
                LOG_W("cnts error");

                return ZT_RETURN_FAIL;
            }

            cnts /= 2;
            LOG_D("%s: cnts=%d", __func__, cnts);
            LOG_D("%s: data=%s", __func__, data1[0]);

            inbuff[1] = cnts;

            for (jj = 0, kk = 0; jj < cnts; jj++, kk += 2)
            {
                inbuff[jj + 2] = mp_proc_key_of_2char2num_func(data1[0][kk],
                                 data1[0][kk + 1]);
            }
        }
        inbuff[0] = EFUSE_FREQCAL;

        for (i = 0; i < 3; i++)
        {
            LOG_D("%x", inbuff[i]);
        }
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_SET, (zt_u32 *) inbuff,
                                  (cnts + 6) / 4, &outbuff, 1);
        if (ret == ZT_RETURN_FAIL)
        {
            LOG_W("set reg fail");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        if (outbuff == 0)
        {
            LOG_W("fail");
        }
        else
        {
            LOG_D("ok");
        }
        if (inbuff)
        {
            zt_kfree(inbuff);
        }
    }
    else if (zt_strncmp(cmd, "Frequency", 9) == 0)
    {
        zt_u32 send_buf[3] = {0}, v;

        set_len = mp_proc_num0fstr(extra, "=");
        if (set_len == 0)
        {
            zt_u32 h2m_msg;
            LOG_D("get case freq\n");
            ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_HAL_FREQ_GET, NULL, 0,
                                      &h2m_msg, 1);
            if (ZT_RETURN_FAIL == ret)
            {
                LOG_W("[%s] read reg failed,check!!!", __func__);
                return ZT_RETURN_FAIL;
            }

            LOG_D("freq=0x%02x", h2m_msg);

            return count;
        }
        else if (set_len > 1)
        {
            LOG_E("set_len error");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }

        value1 = zt_strchr(cmd, '=');
        token1 = value1 + 1;
        inbuff = zt_kzalloc(zt_strlen(token1));
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }

        zt_memcpy(inbuff, token1, zt_strlen(token1) - 1);
        zt_sprintf(inbuff, "0x" "%s", token1);
        sscanf(inbuff, "%x", &v);
        LOG_D("0x%x\n", v);
        send_buf[0] = v;

        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_HAL_FREQ_SET, send_buf, 1,
                                        NULL, 0);
        if (ret == 0)
        {
            LOG_D("set freq ok");
        }
        else
        {
            LOG_D("set freq fail");
        }
        zt_kfree(inbuff);
    }
    else if (zt_strncmp(cmd, "powercal", 8) == 0)
    {

        set_len = mp_proc_num0fstr(extra, "=");
        if (set_len == 0)
        {
            efuse_code = EFUSE_POWERCAL;
            inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
            if (inbuff == NULL)
            {
                LOG_D("alloc recv buff fail");

                return  ZT_RETURN_FAIL;
            }

            ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_GET, &efuse_code, 1,
                                      (zt_u32 *) inbuff, MAILBOX_MAX_TXLEN);
            if (ZT_RETURN_FAIL == ret)
            {
                LOG_W("[%s] read reg failed,check!!!", __func__);
                if (inbuff)
                {
                    zt_kfree(inbuff);
                }

                return ZT_RETURN_FAIL;
            }
            LOG_D("efuse_code = %d, cnts = %d", efuse_code, inbuff[0]);

            for (i = 0; i < inbuff[0]; i++)
            {
                LOG_D("0x%02X", inbuff[i + 1]);
            }
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return count;
        }
        else if (set_len > 1)
        {
            LOG_E("set_len error");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }
        while (set_len--)
        {
            value1 = zt_strchr(cmd, '=');
            if (value1)
            {
                *value1++ = '\0';
                ptmp = cmd;
                token1 = value1;
                strcpy(cmd1[i], ptmp);
                zt_strncpy(data1[i], token1, zt_strlen(token1) - 1);
                LOG_D("cmd = %s", cmd1[i]);
                LOG_D("data1 = %s, test", data1[i]);
                i++;
            }
            else
            {
                LOG_W("[%s]:param error", __func__);

                return ZT_RETURN_FAIL;
            }
        }
        if (data1[0] != 0)
        {
            cnts = zt_strlen(data1[0]);
            LOG_D("cnts: %s, %d", data1[0], cnts);
            if (cnts % 2)
            {
                LOG_W("cnts error");

                return ZT_RETURN_FAIL;
            }

            cnts /= 2;
            LOG_D("%s: cnts=%d", __func__, cnts);
            LOG_D("%s: data=%s", __func__, data1[0]);

            inbuff[1] = cnts;

            for (jj = 0, kk = 0; jj < cnts; jj++, kk += 2)
            {
                inbuff[jj + 2] = mp_proc_key_of_2char2num_func(data1[0][kk],
                                 data1[0][kk + 1]);
            }
        }
        inbuff[0] = EFUSE_POWERCAL;

        for (i = 0; i < 24; i++)
        {
            LOG_D("%x", inbuff[i]);
        }
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_SET, (zt_u32 *) inbuff,
                                  (cnts + 6) / 4, &outbuff, 1);
        if (ret == ZT_RETURN_FAIL)
        {
            LOG_W("set powercal fail");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        if (outbuff == 0)
        {
            LOG_W("fail");
        }
        else
        {
            LOG_D("ok");
        }
        if (inbuff)
        {
            zt_kfree(inbuff);
        }
    }
    else if (zt_strncmp(cmd, "channelplan", 11) == 0)
    {
        set_len = mp_proc_num0fstr(extra, "=");
        if (set_len == 0)
        {
            efuse_code = EFUSE_CHANNELPLAN;
            inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
            if (inbuff == NULL)
            {
                LOG_D("alloc recv buff fail");

                return  ZT_RETURN_FAIL;
            }
            ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_GET, &efuse_code, 1,
                                      (zt_u32 *) inbuff, MAILBOX_MAX_TXLEN);
            if (ZT_RETURN_FAIL == ret)
            {
                LOG_W("[%s] read reg failed,check!!!", __func__);
                if (inbuff)
                {
                    zt_kfree(inbuff);
                }

                return ZT_RETURN_FAIL;
            }
            LOG_D("efuse_code = %d, cnts = %d", efuse_code, inbuff[0]);

            for (i = 0; i < inbuff[0]; i++)
            {
                LOG_D("0x%02X", inbuff[i + 1]);
            }
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return count;
        }
        else if (set_len > 1)
        {
            LOG_E("set_len error");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }
        while (set_len--)
        {
            value1 = zt_strchr(cmd, '=');
            if (value1)
            {
                *value1++ = '\0';
                ptmp = cmd;
                token1 = value1;
                strcpy(cmd1[i], ptmp);
                zt_strncpy(data1[i], token1, zt_strlen(token1) - 1);
                LOG_D("cmd = %s", cmd1[i]);
                LOG_D("data1 = %s, test", data1[i]);
                i++;
            }
            else
            {
                LOG_W("[%s]:param error", __func__);

                return ZT_RETURN_FAIL;
            }
        }
        if (data1[0] != 0)
        {
            cnts = zt_strlen(data1[0]);
            LOG_D("cnts: %s, %d", data1[0], cnts);
            if (cnts % 2)
            {
                LOG_W("cnts error");

                return ZT_RETURN_FAIL;
            }

            cnts /= 2;
            LOG_D("%s: cnts=%d", __func__, cnts);
            LOG_D("%s: data=%s", __func__, data1[0]);

            inbuff[1] = cnts;

            for (jj = 0, kk = 0; jj < cnts; jj++, kk += 2)
            {
                inbuff[jj + 2] = mp_proc_key_of_2char2num_func(data1[0][kk],
                                 data1[0][kk + 1]);
            }
        }
        inbuff[0] = EFUSE_FREQCAL;

        for (i = 0; i < 3; i++)
        {
            LOG_D("%x", inbuff[i]);
        }
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_SET, (zt_u32 *) inbuff,
                                  (cnts + 6) / 4, &outbuff, 1);
        if (ret == ZT_RETURN_FAIL)
        {
            LOG_W("set reg fail");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        if (outbuff == 0)
        {
            LOG_W("fail");
        }
        else
        {
            LOG_D("ok");
        }
        if (inbuff)
        {
            zt_kfree(inbuff);
        }
    }
    else if (zt_strncmp(cmd, "tx_power0", 9) == 0)
    {
        strsep(&pch, "=");
        mp_proc_set_tx_power(pnic_info, pch, count);

    }
    else if (zt_strncmp(cmd, "channel", 7) == 0)
    {
        strsep(&pch, "=");
        mp_proc_set_channel(pnic_info, pch, count);
    }
    else if (zt_strncmp(cmd, "bw", 2) == 0)
    {
        strsep(&pch, "=");
        mp_proc_set_bw(pnic_info, pch, count);
    }
    else if (zt_strncmp(cmd, "gi", 2) == 0)
    {
        strsep(&pch, "=");
        mp_proc_set_gi(pnic_info, pch, count);
    }
    else if (zt_strncmp(cmd, "rate", 2) == 0)
    {
        strsep(&pch, "=");
        mp_proc_set_rate(pnic_info, pch, count);
    }
    else if (zt_strncmp(cmd, "tx", 2) == 0)
    {
        strsep(&pch, "=");
        zt_mp_proc_test_tx(pnic_info, pch, count);
    }
    else if (zt_strncmp(cmd, "rx", 2) == 0)
    {
        strsep(&pch, "=");
        zt_mp_proc_test_rx(pnic_info, pch, count);
    }
    else if (zt_strncmp(cmd, "stats", 5) == 0)
    {
        strsep(&pch, "=");
        zt_mp_proc_stats(pnic_info, pch, count);
    }
    else
    {
        LOG_D("other token: %s", cmd);
    }

    return count;
}
#endif

static ssize_t zt_proc_wifi_set(struct file *file, const char __user *buffer,
                                size_t count, loff_t *pos, void *data)
{
    hif_node_st *hif_info = data;
    nic_info_st *pnic_info = NULL;

    char *pch = NULL;
    zt_s8 *extra = NULL;
    char cmd[60] = {0};
    char cmd1[8][20] = {0};
    char data1[8][20] = {0};
    zt_s32 set_len = 0;
    zt_u16 cnts = 0;
    zt_s32 ret, i = 0;
    zt_u32 jj, kk;
    zt_s32 efuse_code = 1000;
    zt_bool EnableMAC = zt_false;
    char *ptmp, *token1, *value1 = NULL;
    zt_u8 *inbuff = NULL;
    zt_u32  outbuff = 0;
    zt_u8 *buff = NULL;
    pnic_info = hif_info->nic_info[0];

    if (copy_from_user(cmd, buffer, count))
    {
        LOG_E("copy_from_user fail");

        return -ZT_RETURN_FAIL;
    }

    cmd[count] = '\0';
    extra = cmd;
    LOG_D("input: %s", cmd);
    pch = extra;

    if (zt_strncmp(cmd, "mac_r", 5) == 0)
    {
        inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }
        efuse_code = WLAN_EEPORM_MAC;
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_GET, &efuse_code, 1,
                                  (zt_u32 *) inbuff, MAILBOX_MAX_TXLEN);
        if (ZT_RETURN_FAIL == ret)
        {
            LOG_W("[%s] read reg failed,check!!!", __func__);
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        LOG_D("efuse_code = %d, cnts = %d", efuse_code, inbuff[0]);

        for (i = 0; i < inbuff[0]; i++)
        {
            LOG_D("0x%02X", inbuff[i + 1]);
        }
        if (inbuff)
        {
            zt_kfree(inbuff);
        }

        return count;
    }
    else if (zt_strncmp(cmd, "mac_w", 5) == 0)
    {
        set_len = mp_proc_num0fstr(extra, "=");
        EnableMAC = zt_true;
        inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }
        if (set_len > 1)
        {
            LOG_E("set_len error");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        while (set_len--)
        {
            value1 = zt_strchr(cmd, '=');
            if (value1)
            {
                *value1++ = '\0';
                ptmp = cmd;
                token1 = value1;
                strcpy(cmd1[i], ptmp);
                zt_strncpy(data1[i], token1, zt_strlen(token1) - 1);
                LOG_D("cmd = %s", cmd1[i]);
                LOG_D("data1 = %s", data1[i]);
                i++;
            }
            else
            {
                LOG_W("[%s]:param error", __func__);

                return ZT_RETURN_FAIL;
            }
        }
        if (data1[0] != 0)
        {
            cnts = zt_strlen(data1[0]);
            LOG_D("cnts: %s, %d", data1[0], cnts);
            if (cnts % 2)
            {
                LOG_W("cnts error");

                return ZT_RETURN_FAIL;
            }

            cnts /= 2;
            LOG_D("%s: cnts=%d", __func__, cnts);
            LOG_D("%s: data=%s", __func__, data1[0]);

            inbuff[1] = 6;

            for (jj = 0, kk = 0; jj < cnts; jj++, kk += 2)
            {
                inbuff[jj + 2] = mp_proc_key_of_2char2num_func(data1[0][kk],
                                 data1[0][kk + 1]);
            }
        }
        inbuff[0] = 1;

        for (i = 0; i < 8; i++)
        {
            LOG_D("%x", inbuff[i]);
        }
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_SET, (zt_u32 *) inbuff,
                                  (cnts + 6) / 4, &outbuff, 1);
        if (ret == ZT_RETURN_FAIL)
        {
            LOG_W("set reg fail");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        if (outbuff == 0)
        {
            LOG_W("fail");
        }
        else
        {
            LOG_D("ok");
        }
        if (EnableMAC)
        {
            inbuff[0] = EFUSE_EnMAC;
            inbuff[1] = 1;
            inbuff[2] = 1;
            ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_SET, (zt_u32 *) inbuff,
                                      3, &outbuff, 1);
        }
        if (inbuff)
        {
            zt_kfree(inbuff);
        }

        return count;
    }
    else if (zt_strncmp(cmd, "thermal", 7) == 0)
    {
        set_len = mp_proc_num0fstr(extra, "=");
        if (set_len == 0)
        {
            pch = cmd;
            LOG_D("get case temp\n");
            mp_proc_rfthermalmeter_get(pnic_info, pch, count);
            efuse_code = EFUSE_TEMPCAL;
            buff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
            if (buff == NULL)
            {
                LOG_D("alloc recv buff fail");

                return  ZT_RETURN_FAIL;
            }
            ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_GET, &efuse_code, 1,
                                      (zt_u32 *) buff, MAILBOX_MAX_TXLEN);
            if (ZT_RETURN_FAIL == ret)
            {
                LOG_W("[%s] read reg failed,check!!!", __func__);
                if (buff)
                {
                    zt_kfree(buff);
                }

                return ZT_RETURN_FAIL;
            }

            for (i = 0; i < buff[0]; i++)
            {
                LOG_D("Set temperature = 0x%02X", buff[i + 1]);
            }
            if (buff)
            {
                zt_kfree(buff);
            }

            return count;
        }
        else if (set_len > 1)
        {
            LOG_E("set_len error");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }

        while (set_len--)
        {
            value1 = zt_strchr(cmd, '=');
            if (value1)
            {
                *value1++ = '\0';
                ptmp = cmd;
                token1 = value1;
                strcpy(cmd1[i], ptmp);
                zt_strncpy(data1[i], token1, zt_strlen(token1) - 1);
                LOG_D("cmd = %s", cmd1[i]);
                LOG_D("data1 = %s, test", data1[i]);
                i++;
            }
            else
            {
                LOG_W("[%s]:param error", __func__);

                return ZT_RETURN_FAIL;
            }
        }
        if (data1[0] != 0)
        {
            cnts = zt_strlen(data1[0]);
            LOG_D("cnts: %s, %d", data1[0], cnts);
            if (cnts % 2)
            {
                LOG_W("cnts error");

                return ZT_RETURN_FAIL;
            }

            cnts /= 2;
            LOG_D("%s: cnts=%d", __func__, cnts);
            LOG_D("%s: data=%s", __func__, data1[0]);

            inbuff[1] = cnts;

            for (jj = 0, kk = 0; jj < cnts; jj++, kk += 2)
            {
                inbuff[jj + 2] = mp_proc_key_of_2char2num_func(data1[0][kk],
                                 data1[0][kk + 1]);
            }
        }
        inbuff[0] = EFUSE_TEMPCAL;

        for (i = 0; i < 3; i++)
        {
            LOG_D("%x", inbuff[i]);
        }
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_SET, (zt_u32 *) inbuff,
                                  (cnts + 6) / 4, &outbuff, 1);
        if (ret == ZT_RETURN_FAIL)
        {
            LOG_W("set reg fail");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        if (outbuff == 0)
        {
            LOG_W("fail");
        }
        else
        {
            LOG_D("ok");
        }
        if (inbuff)
        {
            zt_kfree(inbuff);
        }
    }
    else if (zt_strncmp(cmd, "freq_efuse", 10) == 0)
    {
        set_len = mp_proc_num0fstr(extra, "=");
        if (set_len == 0)
        {
            inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
            efuse_code = EFUSE_FREQCAL;
            pch = cmd;
            LOG_D("get case freq\n");
            ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_GET, &efuse_code, 1,
                                      (zt_u32 *) inbuff, MAILBOX_MAX_TXLEN);
            if (ZT_RETURN_FAIL == ret)
            {
                LOG_W("[%s] read reg failed,check!!!", __func__);
                if (inbuff)
                {
                    zt_kfree(inbuff);
                }

                return ZT_RETURN_FAIL;
            }
            LOG_D("efuse_code = %d, cnts = %d", efuse_code, inbuff[0]);

            for (i = 0; i < inbuff[0]; i++)
            {
                LOG_D("0x%02X", inbuff[i + 1]);
            }
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return count;
        }
        else if (set_len > 1)
        {
            LOG_E("set_len error");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }
        while (set_len--)
        {
            value1 = zt_strchr(cmd, '=');
            if (value1)
            {
                *value1++ = '\0';
                ptmp = cmd;
                token1 = value1;
                strcpy(cmd1[i], ptmp);
                zt_strncpy(data1[i], token1, zt_strlen(token1) - 1);
                LOG_D("cmd = %s", cmd1[i]);
                LOG_D("data1 = %s, test", data1[i]);
                i++;
            }
            else
            {
                LOG_W("[%s]:param error", __func__);

                return ZT_RETURN_FAIL;
            }
        }
        if (data1[0] != 0)
        {
            cnts = zt_strlen(data1[0]);
            LOG_D("cnts: %s, %d", data1[0], cnts);
            if (cnts % 2)
            {
                LOG_W("cnts error");

                return ZT_RETURN_FAIL;
            }

            cnts /= 2;
            LOG_D("%s: cnts=%d", __func__, cnts);
            LOG_D("%s: data=%s", __func__, data1[0]);

            inbuff[1] = cnts;

            for (jj = 0, kk = 0; jj < cnts; jj++, kk += 2)
            {
                inbuff[jj + 2] = mp_proc_key_of_2char2num_func(data1[0][kk],
                                 data1[0][kk + 1]);
            }
        }
        inbuff[0] = EFUSE_FREQCAL;

        for (i = 0; i < 3; i++)
        {
            LOG_D("%x", inbuff[i]);
        }
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_SET, (zt_u32 *) inbuff,
                                  (cnts + 6) / 4, &outbuff, 1);
        if (ret == ZT_RETURN_FAIL)
        {
            LOG_W("set reg fail");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        if (outbuff == 0)
        {
            LOG_W("fail");
        }
        else
        {
            LOG_D("ok");
        }
        if (inbuff)
        {
            zt_kfree(inbuff);
        }
    }
    else if (zt_strncmp(cmd, "Frequency", 9) == 0)
    {
        zt_u32 send_buf[3] = {0}, v;

        set_len = mp_proc_num0fstr(extra, "=");
        if (set_len == 0)
        {
            zt_u32 h2m_msg;
            LOG_D("get case freq\n");
            ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_HAL_FREQ_GET, NULL, 0,
                                      &h2m_msg, 1);
            if (ZT_RETURN_FAIL == ret)
            {
                LOG_W("[%s] read reg failed,check!!!", __func__);
                return ZT_RETURN_FAIL;
            }

            LOG_D("freq=0x%02x", h2m_msg);

            return count;
        }
        else if (set_len > 1)
        {
            LOG_E("set_len error");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }


        value1 = zt_strchr(cmd, '=');
        token1 = value1 + 1;
        inbuff = zt_kzalloc(zt_strlen(token1));
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }

        zt_memcpy(inbuff, token1, zt_strlen(token1) - 1);
        zt_sprintf(inbuff, "0x" "%s", token1);
        sscanf(inbuff, "%x", &v);
        LOG_D("0x%x\n", v);
        send_buf[0] = v;
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_HAL_FREQ_SET, send_buf, 1,
                                        NULL, 0);
        if (ret == 0)
        {
            LOG_D("set freq ok");
        }
        else
        {
            LOG_D("set freq fail");
        }
        zt_kfree(inbuff);
    }
    else if (zt_strncmp(cmd, "powercal", 8) == 0)
    {

        set_len = mp_proc_num0fstr(extra, "=");
        if (set_len == 0)
        {
            efuse_code = EFUSE_POWERCAL;
            inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
            if (inbuff == NULL)
            {
                LOG_D("alloc recv buff fail");

                return  ZT_RETURN_FAIL;
            }

            ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_GET, &efuse_code, 1,
                                      (zt_u32 *) inbuff, MAILBOX_MAX_TXLEN);
            if (ZT_RETURN_FAIL == ret)
            {
                LOG_W("[%s] read reg failed,check!!!", __func__);
                if (inbuff)
                {
                    zt_kfree(inbuff);
                }

                return ZT_RETURN_FAIL;
            }
            LOG_D("efuse_code = %d, cnts = %d", efuse_code, inbuff[0]);

            for (i = 0; i < inbuff[0]; i++)
            {
                LOG_D("0x%02X", inbuff[i + 1]);
            }
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return count;
        }
        else if (set_len > 1)
        {
            LOG_E("set_len error");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
        if (inbuff == NULL)
        {
            LOG_D("alloc recv buff fail");

            return  ZT_RETURN_FAIL;
        }
        while (set_len--)
        {
            value1 = zt_strchr(cmd, '=');
            if (value1)
            {
                *value1++ = '\0';
                ptmp = cmd;
                token1 = value1;
                strcpy(cmd1[i], ptmp);
                zt_strncpy(data1[i], token1, zt_strlen(token1) - 1);
                LOG_D("cmd = %s", cmd1[i]);
                LOG_D("data1 = %s, test", data1[i]);
                i++;
            }
            else
            {
                LOG_W("[%s]:param error", __func__);

                return ZT_RETURN_FAIL;
            }
        }
        if (data1[0] != 0)
        {
            cnts = zt_strlen(data1[0]);
            LOG_D("cnts: %s, %d", data1[0], cnts);
            if (cnts % 2)
            {
                LOG_W("cnts error");

                return ZT_RETURN_FAIL;
            }

            cnts /= 2;
            LOG_D("%s: cnts=%d", __func__, cnts);
            LOG_D("%s: data=%s", __func__, data1[0]);

            inbuff[1] = cnts;

            for (jj = 0, kk = 0; jj < cnts; jj++, kk += 2)
            {
                inbuff[jj + 2] = mp_proc_key_of_2char2num_func(data1[0][kk],
                                 data1[0][kk + 1]);
            }
        }
        inbuff[0] = EFUSE_POWERCAL;

        for (i = 0; i < 24; i++)
        {
            LOG_D("%x", inbuff[i]);
        }
        ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_SET, (zt_u32 *) inbuff,
                                  (cnts + 6) / 4, &outbuff, 1);
        if (ret == ZT_RETURN_FAIL)
        {
            LOG_W("set powercal fail");
            if (inbuff)
            {
                zt_kfree(inbuff);
            }

            return ZT_RETURN_FAIL;
        }
        if (outbuff == 0)
        {
            LOG_W("fail");
        }
        else
        {
            LOG_D("ok");
        }
        if (inbuff)
        {
            zt_kfree(inbuff);
        }
    }
    else
    {
        LOG_D("other token: %s", cmd);
    }

    return count;
}

static zt_s32 zt_get_mac(struct seq_file *m, void *v)
{
    hif_node_st *hif_info			= m->private;
    zt_u8 *inbuff = NULL;
    zt_s32 efuse_code = 1000;
    zt_s32 ret, i = 0;
    nic_info_st *pnic_info = NULL;

    if (NULL == hif_info)
    {
        zt_print_seq(m, "[%s] hif_info is null\n", __func__);
        return -1;
    }
    pnic_info = hif_info->nic_info[0];
    inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
    if (inbuff == NULL)
    {
        zt_print_seq(m, "alloc recv buff fail\n");

        return ZT_RETURN_FAIL;
    }
    efuse_code = WLAN_EEPORM_MAC;
    ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_GET, &efuse_code, 1,
                              (zt_u32 *) inbuff, MAILBOX_MAX_TXLEN);
    if (ZT_RETURN_FAIL == ret)
    {
        zt_print_seq(m, "[%s] read reg failed,check!!!\n", __func__);
        if (inbuff)
        {
            zt_kfree(inbuff);
        }

        return ZT_RETURN_FAIL;
    }
//	  zt_print_seq(m, "efuse_code = %d, cnts = %d\n", efuse_code, inbuff[0]);

    for (i = 0; i < inbuff[0]; i++)
    {
        zt_print_seq(m, "%02X ", inbuff[i + 1]);
    }
    zt_print_seq(m, "\n");
    if (inbuff)
    {
        zt_kfree(inbuff);
    }

    return 0;
}

static zt_s32 zt_get_powercal(struct seq_file *m, void *v)
{
    hif_node_st *hif_info           = m->private;
    zt_u8 *inbuff = NULL;
    zt_s32 efuse_code = 1000;
    zt_s32 ret, i = 0;
    nic_info_st *pnic_info = NULL;

    if (NULL == hif_info)
    {
        zt_print_seq(m, "[%s] hif_info is null\n", __func__);
        return -1;
    }
    pnic_info = hif_info->nic_info[0];
    efuse_code = EFUSE_POWERCAL;
    inbuff = zt_kzalloc(MAILBOX_MAX_TXLEN * 4);
    if (inbuff == NULL)
    {
        zt_print_seq(m, "alloc recv buff fail\n");
        return  ZT_RETURN_FAIL;
    }

    ret = mcu_cmd_communicate(pnic_info, UMSG_OPS_MP_EFUSE_GET, &efuse_code, 1,
                              (zt_u32 *) inbuff, MAILBOX_MAX_TXLEN);
    if (ZT_RETURN_FAIL == ret)
    {
        zt_print_seq(m, "[%s] read reg failed,check!!!\n", __func__);
        if (inbuff)
        {
            zt_kfree(inbuff);
        }

        return ZT_RETURN_FAIL;
    }
//    zt_print_seq(m, "efuse_code = %d, cnts = %d\n", efuse_code, inbuff[0]);

    for (i = 0; i < inbuff[0]; i++)
    {
        zt_print_seq(m, "%02X ", inbuff[i + 1]);
    }
    zt_print_seq(m, "\n");
    if (inbuff)
    {
        zt_kfree(inbuff);
    }

    return 0;
}

const struct zt_proc_handle proc_hdls[] =
{
    zt_register_proc_interface("version",   zt_get_version_info,    NULL),
    zt_register_proc_interface("tx",        zt_get_tx_info,         zt_set_tx_info),
    zt_register_proc_interface("rx",        zt_get_rx_info,         NULL),
    zt_register_proc_interface("unregister", zt_wiphy_unregister_info, NULL),
    zt_register_proc_interface("ap_info",   zt_get_connect_info,    NULL),
    zt_register_proc_interface("set_rate",   NULL,           zt_set_rate),

#ifdef CFG_ENABLE_AP_MODE
    zt_register_proc_interface("ap",        zt_get_ap_info,         NULL),
#endif
    zt_register_proc_interface("sta",       zt_get_sta_info,        NULL),
    zt_register_proc_interface("hif",       zt_get_hif_info,        NULL),
    zt_register_proc_interface("wlan_mgmt", zt_get_wlan_mgmt_info,  NULL),
    zt_register_proc_interface("mlme",      zt_get_mlme_info,       NULL),
    zt_register_proc_interface("chipreset", NULL,                   zt_chip_reset),
#ifdef CONFIG_MP_MODE
    /* mp */
    zt_register_proc_interface("mp",        NULL,                   mp_proc_wifi_set),
#endif
    zt_register_proc_interface("cob",       NULL,                   zt_proc_wifi_set),
    zt_register_proc_interface("mac_r",     zt_get_mac,             NULL),
    zt_register_proc_interface("powercal",  zt_get_powercal,        NULL),
};

const zt_s32 zt_proc_hdls_num =
    sizeof(proc_hdls) / sizeof(struct zt_proc_handle);

inline struct proc_dir_entry *zt_proc_create_dir(const zt_s8 *name,
        struct proc_dir_entry *parents, void *data)
{
    struct proc_dir_entry *entry;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
#if 1
    entry = proc_mkdir_data(name, S_IRUGO | S_IXUGO, parents, data);
#else
    entry = proc_mkdir(name, parents);
    if (!entry)
    {
        LOG_E("[proc_mkdir]1 error!\n");
    }
#endif
#else
    /* entry = proc_mkdir_mode(name, S_IRUGO|S_IXUGO, parent); */
    entry = proc_mkdir(name, parents);
    if (!entry)
    {
        LOG_E("[proc_mkdir]2 error!\n");
    }
    if (entry)
    {
        entry->data = data;
    }
#endif

    return entry;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 6, 0)
inline struct proc_dir_entry *zt_proc_create_entry(const zt_s8 *name,
        struct proc_dir_entry *parents,
        const struct proc_ops *fops, void *data)
#else
inline struct proc_dir_entry *zt_proc_create_entry(const zt_s8 *name,
        struct proc_dir_entry *parents,
        const struct file_operations *fops, void *data)
#endif
{
    struct proc_dir_entry *entry;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26))
    entry = proc_create_data(name,  S_IFREG | S_IRUGO | S_IWUGO, parents, fops,
                             data);
#else
    entry = create_proc_entry(name, S_IFREG | S_IRUGO | S_IWUGO, parents);
    if (entry)
    {
        entry->data = data;
        entry->proc_fops = fops;
    }
#endif

    return entry;
}

static SSIZE_T zt_proc_write(struct file *file, const char __user *buffer,
                             SIZE_T count, loff_t *pos)
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 9, 1))
    return 0;
#else

    ssize_t index = (ssize_t)PDE_DATA(file_inode(file));
    const struct zt_proc_handle *hdl = proc_hdls + index;
    ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *,
                     void *) = hdl->write;

    if (write)
    {
        return write(file, buffer, count, pos,
                     ((struct seq_file *)file->private_data)->private);
    }

    return -EROFS;
#endif
}

static zt_s32 zt_proc_open(struct inode *inode, struct file *file)
{
    ssize_t index = (ssize_t)PDE_DATA(inode);
    const struct zt_proc_handle *hdl = proc_hdls + index;
    void *private = proc_get_parent_data(inode);

    zt_s32(*show)(struct seq_file *, void *) = hdl->show ? hdl->show : 0;

    return single_open(file, show, private);

}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops zt_proc_fops =
{
    //.owner = THIS_MODULE,
    .proc_open = zt_proc_open,
    .proc_read = seq_read,
    .proc_write = zt_proc_write,
    .proc_lseek = default_llseek,
    .proc_release = single_release,
};
#else
static const struct file_operations zt_proc_fops =
{
    .owner = THIS_MODULE,
    .open = zt_proc_open,
    .read = seq_read,
    .write = zt_proc_write,
    .llseek = seq_lseek,
    .release = single_release,
};
#endif
zt_s32 zt_proc_init(void *hif_info)
{
    zt_s32 ret = zt_false;
    SSIZE_T p;
    hif_node_st *hif_node = (hif_node_st *)hif_info;
    zt_proc_st *proc_info = NULL;
    struct proc_dir_entry *entry = NULL;

    proc_info   = zt_kzalloc(sizeof(zt_proc_st));
    if (NULL == proc_info)
    {
        LOG_E("[%s] malloc proc_info failed", __func__);
        return ZT_RETURN_FAIL;
    }


    LOG_D("[%s] start\n", __func__);

    {
        nic_info_st *pnic_info = hif_node->nic_info[0];
        zt_sprintf(proc_info->proc_name, "%s", netdev_name(pnic_info->ndev));
    }

    proc_info->proc_root =
        zt_proc_create_dir(proc_info->proc_name, zt_proc_net, hif_node);
    if (NULL == proc_info->proc_root)
    {
        LOG_E("[%s]proc dir create error", __func__);
    }

    for (p = 0; p < zt_proc_hdls_num; p++)
    {

        entry = zt_proc_create_entry(proc_hdls[p].name, proc_info->proc_root,
                                     &zt_proc_fops, (void *)p);
        if (!entry)
        {
            LOG_E("[%s]proc entry create error", __func__);
        }
    }

    proc_info->hif_info = hif_info;
    hif_node->proc_info = proc_info;
    proc_info->mp_proc_test_enable = zt_false;

    return ret;
}
void zt_proc_term(void *hif_info)
{
    zt_s32 i;
    hif_node_st *hif_node        = hif_info;
    zt_proc_st   *proc_info      = hif_node->proc_info;

    if (proc_info == NULL)
    {
        return;
    }

    if (proc_info->proc_root == NULL)
    {
        return;
    }

    for (i = 0; i < zt_proc_hdls_num; i++)
    {
        remove_proc_entry(proc_hdls[i].name, proc_info->proc_root);
    }

    remove_proc_entry(proc_info->proc_name, zt_proc_net);
    proc_info->proc_root = NULL;

    zt_kfree(proc_info);
    proc_info = NULL;

}

