/*
 * ndev_linux.c
 *
 * impliment linux framework net device regiest
 *
 * Author: renhaibo
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

/* use linux netdev ioctl framework */
#include <net/ieee80211_radiotap.h>
#include "ndev_linux.h"
#include "zt_cfg80211.h"
#include "hif.h"
#include "rx.h"
#include "common.h"
#ifdef CONFIG_IOCTL_CFG80211
#include <linux/nl80211.h>
#include <net/cfg80211.h>
#endif


#ifndef IEEE80211_BAND_2GHZ
#define IEEE80211_BAND_2GHZ NL80211_BAND_2GHZ
#endif

#define NDEV_DBG(fmt, ...)      LOG_D("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)
#define NDEV_INFO(fmt, ...)     LOG_I("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)
#define NDEV_WARN(fmt, ...)     LOG_W("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)
#define NDEV_ERROR(fmt, ...)    LOG_E("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)

enum
{
    ZT_HOSTAPD_FLUSH                    = 1,
    ZT_HOSTAPD_ADD_STA                  = 2,
    ZT_HOSTAPD_REMOVE_STA               = 3,
    ZT_HOSTAPD_GET_INFO_STA             = 4,
    ZT_HOSTAPD_GET_WPAIE_STA            = 5,
    ZT_SET_ENCRYPTION                   = 6,
    ZT_GET_ENCRYPTION                   = 7,
    ZT_HOSTAPD_SET_FLAGS_STA            = 8,
    ZT_HOSTAPD_GET_RID                  = 9,
    ZT_HOSTAPD_SET_RID                  = 10,
    ZT_HOSTAPD_SET_ASSOC_AP_ADDR        = 11,
    ZT_HOSTAPD_SET_GENERIC_ELEMENT      = 12,
    ZT_HOSTAPD_MLME                     = 13,
    ZT_HOSTAPD_SCAN_REQ                 = 14,
    ZT_HOSTAPD_STA_CLEAR_STATS          = 15,
    ZT_HOSTAPD_SET_BEACON               = 16,
    ZT_HOSTAPD_SET_WPS_BEACON           = 17,
    ZT_HOSTAPD_SET_WPS_PROBE_RESP       = 18,
    ZT_HOSTAPD_SET_WPS_ASSOC_RESP       = 19,
    ZT_HOSTAPD_SET_HIDDEN_SSID          = 20,
    ZT_HOSTAPD_SET_MACADDR_ACL          = 21,
    ZT_HOSTAPD_ACL_ADD_STA              = 22,
    ZT_HOSTAPD_ACL_REMOVE_STA           = 23,
};


static zt_s32 ndev_init(struct net_device *ndev)
{
    ndev_priv_st *ndev_priv;
    hw_info_st *hw_info;

    NDEV_DBG("[NDEV]%p ndev_init ", ndev);

    ndev_priv = netdev_priv(ndev);

    if (nic_init(ndev_priv->nic) < 0)
    {
        return -1;
    }

    tx_work_init(ndev);

    hw_info = (hw_info_st *)ndev_priv->nic->hw_info;
    if (hw_info)
    {
        if (!is_valid_ether_addr(hw_info->macAddr))
        {
            NDEV_ERROR("[NDEV]%p mac addr is invalid ", ndev);
            //return -1;
        }

        NDEV_INFO("efuse_macaddr:"ZT_MAC_FMT, ZT_MAC_ARG(hw_info->macAddr));
        zt_memcpy(ndev->dev_addr, hw_info->macAddr, ZT_80211_MAC_ADDR_LEN);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0))
        zt_memcpy(ndev->dev_addr_shadow, hw_info->macAddr, ZT_80211_MAC_ADDR_LEN);
#endif
        NDEV_INFO("[%d] macaddr:"ZT_MAC_FMT, ndev_priv->nic->ndev_id,
                  ZT_MAC_ARG(hw_info->macAddr));
    }

    return 0;
}

static void ndev_uninit(struct net_device *ndev)
{
    ndev_priv_st *ndev_priv;

    NDEV_DBG("[NDEV] ndev_uninit - start");

    ndev_priv = netdev_priv(ndev);

    tx_work_term(ndev);

    nic_term(ndev_priv->nic);

    NDEV_DBG("[NDEV] ndev_uninit - end");
}

#ifdef CONFIG_IOCTL_CFG80211
static void wiphy_cap_ext_init(nic_info_st *pnic_info,
                               struct ieee80211_sta_ht_cap *ht_cap,
                               enum nl80211_band band, zt_u8 rf_type)
{
    hw_info_st *phw_info = pnic_info->hw_info;

    if (phw_info->ldpc_support)
    {
        ht_cap->cap |= ZT_80211_MGMT_HT_CAP_LDPC_CODING;
    }

    if (phw_info->tx_stbc_support)
    {
        ht_cap->cap |= ZT_80211_MGMT_HT_CAP_TX_STBC;
    }

    if (phw_info->rx_stbc_support)
    {
        if (NL80211_BAND_2GHZ == band)
        {
            if (rf_type == 3)
            {
                ht_cap->cap |= ZT_80211_MGMT_HT_CAP_RX_STBC_1R;
            }
        }
    }
}

static void wiphy_cap_init(nic_info_st *pnic_info,
                           struct ieee80211_sta_ht_cap *pht_cap,
                           enum nl80211_band band,
                           zt_u8 rf_type)
{
    NDEV_DBG();

    pht_cap->ht_supported = zt_true;
    pht_cap->cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
                   IEEE80211_HT_CAP_SGI_40 |
                   IEEE80211_HT_CAP_SGI_20 |
                   IEEE80211_HT_CAP_DSSSCCK40 |
                   IEEE80211_HT_CAP_MAX_AMSDU;
    wiphy_cap_ext_init(pnic_info, pht_cap, band, rf_type);
    pht_cap->ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K;
    pht_cap->ampdu_density = IEEE80211_HT_MPDU_DENSITY_16;
    pht_cap->mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
    if (rf_type == 3)
    {
        pht_cap->mcs.rx_mask[0] = 0xFF;
        pht_cap->mcs.rx_highest = 150;
    }
    else
    {
        NDEV_INFO("error rf_type=%d\n", rf_type);
    }
}

static void wiphy_apply_flag(nic_info_st *pnic_info)
{
    struct wiphy *pwiphy;
    hw_info_st *phw_info;
    zt_u8 i, j;
    struct ieee80211_supported_band *pbands;
    struct ieee80211_channel *pch;
    zt_u8 max_chan_nums;
    zt_u16 channel;
    zt_channel_info_t *channel_set;

    if (ZT_CANNOT_RUN(pnic_info))
    {
        NDEV_WARN("ZT_CANNOT_RUN!!!!!!!!!!!!!!");
        return;
    }

    pwiphy = pnic_info->pwiphy;
    phw_info = (hw_info_st *)pnic_info->hw_info;
    channel_set = phw_info->channel_set;

    for (i = 0; i < ZT_ARRAY_SIZE(pwiphy->bands); i++)
    {
        pbands = pwiphy->bands[i];
        if (pbands)
        {
            for (j = 0; j < pbands->n_channels; j++)
            {
                pch = &pbands->channels[j];
                if (pch)
                {
                    pch->flags = IEEE80211_CHAN_DISABLED;
                }
            }
        }
    }

    max_chan_nums = phw_info->max_chan_nums;
    for (i = 0; i < max_chan_nums; i++)
    {
        channel = channel_set[i].channel_num;
        pch = ieee80211_get_channel(pwiphy, zt_ch_2_freq(channel));
        if (pch)
        {
            if (channel_set[i].scan_type == SCAN_TYPE_PASSIVE)
            {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
                pch->flags =
                    (IEEE80211_CHAN_NO_IBSS | IEEE80211_CHAN_PASSIVE_SCAN);
#else
                pch->flags = IEEE80211_CHAN_NO_IR;
#endif
            }
            else
            {
                pch->flags = 0;
            }
        }
    }
}

static inline nic_info_st *wiphy_to_nic_info(struct wiphy *pwiphy)
{
    return *(nic_info_st **)wiphy_priv(pwiphy);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0))
zt_s32 wiphy_reg_notifier(struct wiphy *pwiphy,
                          struct regulatory_request *request)
#else
void wiphy_reg_notifier(struct wiphy *pwiphy,
                        struct regulatory_request *request)
#endif
{
    nic_info_st *pnic_info;

    NDEV_DBG();

    pnic_info = wiphy_to_nic_info(pwiphy);
    if (pnic_info)
    {
        wiphy_apply_flag(pnic_info);
    }
    else
    {
        LOG_E("pnic_info null point !!!");
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0))
    return 0;
#endif
}

#define WL_2GHZ_CH01_11 \
    REG_RULE(2412-10, 2462+10, 40, 0, 20, 0)

#define WL_2GHZ_CH12_13 \
    REG_RULE(2467-10, 2472+10, 40, 0, 20,   \
             NL80211_RRF_PASSIVE_SCAN)

#define WL_2GHZ_CH14    \
    REG_RULE(2484-10, 2484+10, 40, 0, 20,   \
             NL80211_RRF_PASSIVE_SCAN | NL80211_RRF_NO_OFDM)

static const struct ieee80211_regdomain regdom_rd =
{
    .n_reg_rules = 2,
    .alpha2 = "99",
    .reg_rules =
    {
        WL_2GHZ_CH01_11,
        WL_2GHZ_CH12_13,
    }
};

static void wiphy_regd_init(nic_info_st *pnic_info)
{
    struct wiphy *pwiphy = pnic_info->pwiphy;

    NDEV_DBG();

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
    pwiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;
    pwiphy->flags &= ~WIPHY_FLAG_STRICT_REGULATORY;
    pwiphy->flags &= ~WIPHY_FLAG_DISABLE_BEACON_HINTS;
#else
    pwiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
    pwiphy->regulatory_flags &= ~REGULATORY_STRICT_REG;
    pwiphy->regulatory_flags &= ~REGULATORY_DISABLE_BEACON_HINTS;
#endif

    wiphy_apply_custom_regulatory(pnic_info->pwiphy, &regdom_rd);
}

void zt_wiphy_init(nic_info_st *pnic_info)
{
    struct ieee80211_supported_band *pband;
    struct wiphy *pwiphy = pnic_info->pwiphy;
    hw_info_st *phw_info = pnic_info->hw_info;

    NDEV_DBG();

    pband = pwiphy->bands[IEEE80211_BAND_2GHZ];
    if (pband)
    {
        wiphy_cap_init(pnic_info, &pband->ht_cap,
                       IEEE80211_BAND_2GHZ, phw_info->rf_type);
    }

    pwiphy->reg_notifier = wiphy_reg_notifier;
    wiphy_apply_flag(pnic_info);

    zt_memcpy(pwiphy->perm_addr, nic_to_local_addr(pnic_info),
              sizeof(zt_80211_addr_t));
}
#endif
zt_s32 ndev_open(struct net_device *ndev)
{
    ndev_priv_st *ndev_priv = NULL;
    nic_info_st *pnic_info  = NULL;


    ndev_priv = netdev_priv(ndev);
    pnic_info = ndev_priv->nic;
    if (NULL == pnic_info)
    {
        return -1;
    }

    NDEV_DBG("[%d] ndev_open ", pnic_info->ndev_id);
    if (nic_enable(pnic_info) == ZT_RETURN_FAIL)
    {
        return -1;
    }

#ifdef CONFIG_IOCTL_CFG80211
    zt_wiphy_init(pnic_info);
#endif
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2, 6, 35))
    netif_tx_wake_all_queues(ndev);
#else
    netif_wake_queue(ndev);
#endif

    NDEV_DBG("[%d] ndev_open - end", pnic_info->ndev_id);
    return  0;
}

static zt_s32 ndev_stop(struct net_device *ndev)
{
    ndev_priv_st *ndev_priv = NULL;
    nic_info_st  *pnic_info = NULL;

    ndev_priv = netdev_priv(ndev);
    pnic_info = ndev_priv->nic;

    if (NULL == pnic_info)
    {
        return -1;
    }

    //NDEV_DBG("[%d] ndev_stop", pnic_info->ndev_id);

    if (nic_disable(pnic_info) == ZT_RETURN_FAIL)
    {
        return -1;
    }

#if 0
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2, 6, 35))
    netif_tx_stop_all_queues(ndev);
#else
    netif_stop_queue(ndev);
#endif
#endif

    NDEV_DBG("[%d] ndev_stop - end", pnic_info->ndev_id);

    return 0;
}

void ndev_tx_resource_enable(struct net_device *ndev, zt_pkt *pkt)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    zt_u16 qidx;

    qidx = skb_get_queue_mapping(pkt);

    netif_tx_wake_all_queues(ndev);
#else
    netif_wake_queue(ndev);
#endif
}

void ndev_tx_resource_disable(struct net_device *ndev, zt_pkt *pkt)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    zt_u16 qidx;

    qidx = skb_get_queue_mapping(pkt);

    netif_tx_stop_all_queues(ndev);
#else
    netif_stop_queue(ndev);
#endif
}

zt_u32 total_cnt = 0;

static zt_s32 ndev_start_xmit(struct sk_buff *skb, struct net_device *ndev)
{
    ndev_priv_st *ndev_priv;
    zt_bool bRet = zt_false;
    zt_s32 ret;

    if (!skb)
    {
        return 0;
    }

    ndev_priv = netdev_priv(ndev);

    if (ZT_CANNOT_RUN(ndev_priv->nic))
    {
        return 0;
    }

    if (ndev_priv->nic->tx_info == NULL)
    {
        return 0;
    }

#ifdef CFG_ENABLE_MONITOR_MODE
    if (zt_mlme_check_mode(ndev_priv->nic, ZT_MONITOR_MODE) == zt_true)
    {
        //work_monitor_tx_entry(pnetdev, (struct sk_buff *)pkt);
    }
    else
#endif
    {
        if (zt_false == zt_tx_data_check(ndev_priv->nic))
        {
            zt_free_skb(skb);
        }
        else
        {
            /* tx resource check */
            bRet = zt_need_stop_queue(ndev_priv->nic);
            if (bRet == zt_true)
            {
                tx_info_st *tx_info = ndev_priv->nic->tx_info;
                if (tx_info)
                {
                    LOG_W("ndev tx stop queue, free:%d, pending:%d",
                          tx_info->free_xmitframe_cnt, tx_info->pending_frame_cnt);
                }
                ndev_tx_resource_disable(ndev, skb);
            }

            /* actually xmit */
            ret = zt_tx_msdu(ndev_priv->nic, skb->data, skb->len, skb);
            if (ret < 0)
            {
                /* failed xmit, must release the resource */
                zt_free_skb(skb);
            }
            else if (!ret)
            {
                tx_work_wake(ndev);
            }
        }
    }

    return 0;
}

static const zt_u16 select_queue[8] = { 2, 3, 3, 2, 1, 1, 0, 0 };

static zt_u32 classify8021d(struct sk_buff *skb)
{
    zt_u32 dscp;

    if (skb->priority >= 256 && skb->priority <= 263)
    {
        return skb->priority - 256;
    }

    switch (skb->protocol)
    {
        case htons(ETH_P_IP):
            dscp = ip_hdr(skb)->tos & 0xfc;
            break;
        default:
            return 0;
    }

    return dscp >> 5;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
static zt_u16 ndev_select_queue(struct net_device *ndev, struct sk_buff *skb,
                                struct net_device *sb_dev)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
static zt_u16 ndev_select_queue(struct net_device *ndev, struct sk_buff *skb,
                                struct net_device *sb_dev, select_queue_fallback_t fallback)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)
static zt_u16 ndev_select_queue(struct net_device *ndev, struct sk_buff *skb,
                                void *accel_priv, select_queue_fallback_t fallback)
#else
static zt_u16 ndev_select_queue(struct net_device *ndev, struct sk_buff *skb)
#endif
{
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;
    zt_u8 *curBssid;
    wdn_net_info_st *pwdn_info;

    skb->priority = classify8021d(skb);

    if (pnic_info == NULL)
    {
        return 0;
    }

    curBssid = zt_wlan_get_cur_bssid(pnic_info);
    if (curBssid == NULL)
    {
        return 0;
    }

    pwdn_info = zt_wdn_find_info(pnic_info, curBssid);
    if (pwdn_info == NULL)
    {
        return 0;
    }

    if (pwdn_info->acm_mask != 0)
    {
        skb->priority = zt_chk_qos(pwdn_info->acm_mask, skb->priority, 1);
    }

    return select_queue[skb->priority];
}

static zt_s32 ndev_set_mac_addr(struct net_device *pnetdev, void *addr)
{
    struct sockaddr *sock_addr = addr;
    ndev_priv_st *ndev_priv = netdev_priv(pnetdev);
    nic_info_st *pnic_info = ndev_priv->nic;

    NDEV_DBG("ndev_set_mac_addr:" ZT_MAC_FMT, ZT_MAC_ARG(sock_addr->sa_data));

    if (!is_valid_ether_addr(sock_addr->sa_data))
    {
        return -EADDRNOTAVAIL;
    }

    if (pnic_info->is_up)
    {
        NDEV_ERROR("The interface is not in down state");
        return -EADDRNOTAVAIL;
    }

    if (pnic_info->is_driver_critical)
    {
        NDEV_WARN("driver enter crital");
        return -ENOSYS;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0))
    eth_hw_addr_set(pnetdev, sock_addr->sa_data);
#else
    zt_memcpy(pnetdev->dev_addr, sock_addr->sa_data, ZT_80211_MAC_ADDR_LEN);
#endif

    zt_memcpy(nic_to_local_addr(pnic_info), sock_addr->sa_data,
              ZT_80211_MAC_ADDR_LEN);
    zt_mcu_set_macaddr(pnic_info, sock_addr->sa_data);

    return 0;
}

static struct net_device_stats *ndev_get_stats(struct net_device *ndev)
{
    ndev_priv_st *ndev_priv;
    nic_info_st *nic_info;
    tx_info_st *tx_info;
    rx_info_t *rx_info;

    //NDEV_DBG("ndev_get_stats ");

    ndev_priv = netdev_priv(ndev);
    if (ndev_priv == NULL)
    {
        return NULL;
    }

    nic_info = ndev_priv->nic;
    if (nic_info == NULL)
    {
        return &ndev_priv->stats;
    }

    tx_info = nic_info->tx_info;
    if (tx_info == NULL)
    {
        return &ndev_priv->stats;
    }

    rx_info = nic_info->rx_info;
    if (rx_info == NULL)
    {
        return &ndev_priv->stats;
    }

    ndev_priv->stats.tx_packets = tx_info->tx_pkts;
    ndev_priv->stats.rx_packets = rx_info->rx_pkts;
    ndev_priv->stats.tx_dropped = tx_info->tx_drop;
    ndev_priv->stats.rx_dropped = rx_info->rx_drop;
    ndev_priv->stats.tx_bytes = tx_info->tx_bytes;
    ndev_priv->stats.rx_bytes = rx_info->rx_bytes;

    return &ndev_priv->stats;
}

extern int zt_android_priv_cmd_ioctl(struct net_device *net,
                                     struct ifreq *ifr,
                                     int cmd);

static zt_s32 ndev_ioctl(struct net_device *dev, struct ifreq *req, zt_s32 cmd)
{
    ndev_priv_st *ndev_priv;
    zt_s32 ret = 0;

    ndev_priv = netdev_priv(dev);
    if (ZT_CANNOT_RUN(ndev_priv->nic))
    {
        return 0;
    }

    //NDEV_DBG("ndev_ioctl cmd:%x", cmd);

    switch (cmd)
    {
        case IW_IOC_WPA_SUPPLICANT:
            NDEV_DBG("ndev_ioctl IW_PRV_WPA_SUPPLICANT");
            break;

        case IW_IOC_HOSTAPD:
            NDEV_DBG("ndev_ioctl IW_PRV_HOSTAPD");
            break;

        case (SIOCDEVPRIVATE + 1):     /* Android ioctl */
            ret = zt_android_priv_cmd_ioctl(dev, req, cmd);
            break;

        default:
            ret = -EOPNOTSUPP;
            break;
    }

    return ret;
}

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2, 6, 29))
static const struct net_device_ops ndev_ops =
{
    .ndo_init = ndev_init,
    .ndo_uninit = ndev_uninit,
    .ndo_open = ndev_open,
    .ndo_stop = ndev_stop,
    .ndo_start_xmit = ndev_start_xmit,
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2, 6, 35))
    .ndo_select_queue = ndev_select_queue,
#endif
    .ndo_set_mac_address = ndev_set_mac_addr,
    .ndo_get_stats = ndev_get_stats,
    .ndo_do_ioctl = ndev_ioctl,
};
#endif

static void ndev_ops_init(struct net_device *ndev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
    ndev->netdev_ops = &ndev_ops;
#else
    ndev->init = ndev_init;
    ndev->uninit = ndev_uninit;
    ndev->open = ndev_open;
    ndev->stop = ndev_stop;
    ndev->hard_start_xmit = ndev_start_xmit;
    ndev->set_mac_address = ndev_set_mac_addr;
    ndev->get_stats = ndev_get_stats;
    ndev->do_ioctl = ndev_ioctl;
#endif
}

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2, 6, 29))
static const struct net_device_ops ndev_vir_ops =
{
    .ndo_init = ndev_init,
    .ndo_uninit = ndev_uninit,
    .ndo_open = ndev_open,
    .ndo_stop = ndev_stop,
    .ndo_start_xmit = ndev_start_xmit,
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2, 6, 35))
    .ndo_select_queue = ndev_select_queue,
#endif
    .ndo_set_mac_address = ndev_set_mac_addr,
    .ndo_get_stats = ndev_get_stats,
    .ndo_do_ioctl = ndev_ioctl,
};
#endif

static void ndev_vir_ops_init(struct net_device *ndev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
    ndev->netdev_ops = &ndev_vir_ops;
#else
    ndev->init = ndev_init;
    ndev->uninit = ndev_uninit;
    ndev->open = ndev_vir_open;
    ndev->stop = ndev_vir_stop;
    ndev->hard_start_xmit = ndev_start_xmit;
    ndev->set_mac_address = ndev_set_mac_addr;
    ndev->get_stats = ndev_get_stats;
    ndev->do_ioctl = ndev_ioctl;
#endif
}

extern const struct iw_handler_def wl_handlers_def;

static zt_s32 _ndev_notifier_cb(struct notifier_block *nb,
                                zt_ptr state, void *ptr)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(3, 11, 0))
    struct net_device *ndev = netdev_notifier_info_to_dev(ptr);
#else
    struct net_device *ndev = ptr;
#endif

    NDEV_DBG("ndev:%p", ndev);

    if (ndev == NULL)
    {
        return NOTIFY_DONE;
    }

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2, 6, 29))
    if (ndev->netdev_ops == NULL)
    {
        return NOTIFY_DONE;
    }

    if (ndev->netdev_ops->ndo_do_ioctl == NULL)
    {
        return NOTIFY_DONE;
    }

    if (ndev->netdev_ops->ndo_do_ioctl != ndev_ioctl)
#else
    if (ndev->do_ioctl == NULL)
    {
        return NOTIFY_DONE;
    }

    if (ndev->do_ioctl != ndev_ioctl)
#endif
    {
        return NOTIFY_DONE;
    }

    NDEV_DBG("state == %lu", state);

    switch (state)
    {
        case NETDEV_CHANGENAME:
            break;
    }

    return NOTIFY_DONE;

}

static struct notifier_block zt_ndev_notifier =
{
    .notifier_call = _ndev_notifier_cb,
};

zt_s32 ndev_notifier_register(void)
{
    return register_netdevice_notifier(&zt_ndev_notifier);
}

void ndev_notifier_unregister(void)
{
    unregister_netdevice_notifier(&zt_ndev_notifier);
}

zt_s32 ndev_register(nic_info_st *pnic_info)
{
    zt_s32 ret = 0;
    struct net_device *pndev;
    zt_u8 *pre_name;
    zt_u8 dev_name[16];
    ndev_priv_st *pndev_priv;
    hif_mngent_st *hif = hif_mngent_get();

    NDEV_DBG("[NDEV] ndev_register --start <node_id:%d> <ndev_id:%d>",
             pnic_info->hif_node_id, pnic_info->ndev_id);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    pndev = alloc_etherdev_mq(sizeof(ndev_priv_st), 4);
#else
    pndev = alloc_etherdev(sizeof(ndev_priv_st));
#endif
    if (pndev == NULL)
    {
        NDEV_WARN("alloc_etherdev error [ret:%d]", ret);
        return -1;
    }

    pndev_priv      = netdev_priv(pndev);
    pndev_priv->nic = pnic_info;
    pnic_info->ndev = pndev;
    pnic_info->widev_priv = &pndev_priv->widev_priv;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
    SET_MODULE_OWNER(pndev);
#endif

    /* regiest ethernet operation */
    if (pnic_info->nic_num == 0)
    {
        ndev_ops_init(pndev);
    }
    else
    {
        ndev_vir_ops_init(pndev);
    }

    /* set watchdog timeout */
    pndev->watchdog_timeo = HZ * 3;

#ifdef CONFIG_WIRELESS_EXT
    /* regiest wireless extension */
    pndev->wireless_handlers = (struct iw_handler_def *)&wl_handlers_def;
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 5, 0)
    SET_NETDEV_DEV(pndev, pnic_info->dev);
#endif

#ifdef CONFIG_IOCTL_CFG80211
    /* alloc nl80211 object */
    ret = zt_cfg80211_alloc(pnic_info);
    if (ret)
    {
        NDEV_WARN("nl80211 device alloc fail [ret:%d]", ret);
        return ret;
    }

    wiphy_regd_init(pnic_info);

    ret = zt_cfg80211_reg(pnic_info->pwiphy);
    if (ret)
    {
        NDEV_WARN("nl80211 wiphy regiest fail [ret:%d]", ret);
        return ret;
    }
#endif

    /* alloc device name */
    {
        pre_name = pnic_info->virNic ? hif->if2name : hif->ifname;

        if (pre_name[0] == '\0')
        {
            zt_sprintf(dev_name, "%s%s%d",
                       pnic_info->virNic ? "vir" : "wlan",
                       pnic_info->nic_type == NIC_USB ? "_u" : "_s",
                       pnic_info->hif_node_id);
        }
        else if (pnic_info->hif_node_id)
        {
            zt_sprintf(dev_name, "%s%d", pre_name, pnic_info->hif_node_id);
        }
        else
        {
            zt_sprintf(dev_name, "%s", pre_name);
        }
    }

    if (dev_alloc_name(pndev, dev_name) < 0)
    {
        NDEV_WARN("dev_alloc_name [%s] fail!", dev_name);
    }
    //    ether_setup(pndev); /* no work ??????? */

    netif_carrier_off(pndev);
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2, 6, 35))
    netif_tx_stop_all_queues(pndev);
#else
    netif_stop_queue(pndev);
#endif

    ret = register_netdev(pndev);
    if (ret)
    {
#ifdef CONFIG_IOCTL_CFG80211
        zt_cfg80211_wiphy_unreg(pnic_info);
        zt_cfg80211_widev_free(pnic_info);
        zt_cfg80211_wiphy_free(pnic_info);
#endif

        free_netdev(pndev);
        pndev = NULL;
        pnic_info->ndev = NULL;
        NDEV_WARN("register_netdev error [ret:%d]", ret);
        return ret;
    }

    NDEV_DBG("[NDEV] ndev_register --end");

    return 0;
}



zt_s32 ndev_unregister(nic_info_st *pnic_info)
{
    if (pnic_info->ndev != NULL)
    {
        NDEV_DBG("[NDEV] --ndev_unregister - start");
#ifdef CONFIG_IOCTL_CFG80211
        zt_cfg80211_widev_unreg(pnic_info);
#endif
        NDEV_DBG("[%d]", pnic_info->ndev_id);
        unregister_netdev(pnic_info->ndev);
#ifdef CONFIG_IOCTL_CFG80211
        zt_cfg80211_wiphy_unreg(pnic_info);
        zt_cfg80211_widev_free(pnic_info);
        zt_cfg80211_wiphy_free(pnic_info);
#endif
        free_netdev(pnic_info->ndev);
        pnic_info->ndev = NULL;

        NDEV_DBG("[NDEV] --ndev_unregister - end");
    }

    return 0;
}


ndev_priv_st *ndev_get_priv(nic_info_st *pnic_info)
{
    struct net_device *ndev         = (struct net_device *)pnic_info->ndev;
    return netdev_priv(ndev);
}


zt_s32 ndev_shutdown(nic_info_st *pnic_info)
{
    NDEV_DBG();

    if (pnic_info == NULL)
    {
        return 0;
    }

    nic_shutdown(pnic_info);

    return 0;
}

zt_s32 ndev_unregister_all(nic_info_st *nic_info[], zt_u8 nic_num)
{
    zt_u8 i;

    for (i = 0; i < nic_num; i++)
    {
        nic_info_st *pnic_info = nic_info[i];
        if (!pnic_info || pnic_info->is_surprise_removed)
        {
            continue;
        }

        LOG_D("ndev unregister: ndev_id: %d", pnic_info->ndev_id);
        pnic_info->is_surprise_removed = zt_true;
        ndev_shutdown(pnic_info);
        if (zt_false == pnic_info->is_init_commplete)
        {
            nic_term(pnic_info);
        }
        ndev_unregister(pnic_info);

        /* if current is real nic, all virtual nic's buddy pointer need clear */
        if (0 == i)
        {
            zt_u8 j = 0;
            for (j = 1; j < nic_num; j++)
            {
                if (nic_info[j])
                {
                    nic_info[j]->buddy_nic = NULL;
                }
            }
        }

        zt_kfree(pnic_info);
        nic_info[i] = NULL;
    }

    return 0;
}


