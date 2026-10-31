/*
 * zt_cfg80211.c
 *
 * used for netlink framework interface
 *
 * Author: houchuang
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
#include <linux/nl80211.h>
#include <net/cfg80211.h>
#include <linux/decompress/mm.h>

//#undef ZT_DEBUG_LEVEL
//#define ZT_DEBUG_LEVEL (~ZT_DEBUG_DEBUG)
#include "common.h"
#include "hif.h"
#include "zt_cfg80211.h"
#include "android/android_priv_cmd.h"
#include "os_priv.h"

#ifdef CONFIG_IOCTL_CFG80211

#define CFG80211_DBG(fmt, ...)      LOG_D("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)
#define CFG80211_ARRAY(data, len)   zt_log_array(data, len)
#define CFG80211_INFO(fmt, ...)     LOG_I("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)
#define CFG80211_WARN(fmt, ...)     LOG_W("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)
#define CFG80211_ERROR(fmt, ...)    LOG_E("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)

#define ZT_80211_CRYPT_ALG_NAME_LEN     16

#define SET_CFG80211_REPORT_MGMT(w, t, v)   (w->report_mgmt |= (v ? ZT_BIT(t >> 4) : 0))
#define GET_CFG80211_REPORT_MGMT(w, t)      ((w->report_mgmt & ZT_BIT(t >> 4)) > 0)
#define CLR_CFG80211_REPORT_MGMT(w, t, v)   (w->report_mgmt &= (~ ZT_BIT(t >> 4)))

struct cfg80211_crypt
{
    zt_80211_addr_t sta_addr;
    zt_u32 alg;
    zt_u8 set_tx;
    zt_u8 idx;
    zt_u16 key_len;
    zt_u8 key[];
} ;

zt_s32 zt_cfg80211_p2p_rx_mgmt(void *nic_info, void *param, zt_u32 param_len);
zt_s32 zt_cfg80211_remain_on_channel_expired(void *nic, void *param,
        zt_u32 param_len);
zt_s32 zt_cfg80211_p2p_ready_on_channel(void *nic_info, void *param,
                                        zt_u32 param_len);

zt_s8 *cfg80211_frame_to_str(zt_80211_frame_e type)
{
    switch (type)
    {
        case ZT_80211_FRM_ASSOC_REQ          :
        {
            return to_str(ZT_80211_FRM_ASSOC_REQ);
        }
        case ZT_80211_FRM_ASSOC_RESP         :
        {
            return to_str(ZT_80211_FRM_ASSOC_RESP);
        }
        case ZT_80211_FRM_REASSOC_REQ        :
        {
            return to_str(ZT_80211_FRM_REASSOC_REQ);
        }
        case ZT_80211_FRM_REASSOC_RESP       :
        {
            return to_str(ZT_80211_FRM_REASSOC_RESP);
        }
        case ZT_80211_FRM_PROBE_REQ          :
        {
            return to_str(ZT_80211_FRM_PROBE_REQ);
        }
        case ZT_80211_FRM_PROBE_RESP         :
        {
            return to_str(ZT_80211_FRM_PROBE_RESP);
        }
        case ZT_80211_FRM_BEACON             :
        {
            return to_str(ZT_80211_FRM_BEACON);
        }
        case ZT_80211_FRM_ATIM               :
        {
            return to_str(ZT_80211_FRM_ATIM);
        }
        case ZT_80211_FRM_DISASSOC           :
        {
            return to_str(ZT_80211_FRM_DISASSOC);
        }
        case ZT_80211_FRM_AUTH               :
        {
            return to_str(ZT_80211_FRM_AUTH);
        }
        case ZT_80211_FRM_DEAUTH             :
        {
            return to_str(ZT_80211_FRM_DEAUTH);
        }
        case ZT_80211_FRM_ACTION             :
        {
            return to_str(ZT_80211_FRM_ACTION);
        }
        /* control frame */
        case ZT_80211_FRM_CTL_EXT            :
        {
            return to_str(ZT_80211_FRM_CTL_EXT);
        }
        case ZT_80211_FRM_BACK_REQ           :
        {
            return to_str(ZT_80211_FRM_BACK_REQ);
        }
        case ZT_80211_FRM_BACK               :
        {
            return to_str(ZT_80211_FRM_BACK);
        }
        case ZT_80211_FRM_PSPOLL             :
        {
            return to_str(ZT_80211_FRM_PSPOLL);
        }
        case ZT_80211_FRM_RTS                :
        {
            return to_str(ZT_80211_FRM_RTS);
        }
        case ZT_80211_FRM_CTS                :
        {
            return to_str(ZT_80211_FRM_CTS);
        }
        case ZT_80211_FRM_ACK                :
        {
            return to_str(ZT_80211_FRM_ACK);
        }
        case ZT_80211_FRM_CFEND              :
        {
            return to_str(ZT_80211_FRM_CFEND);
        }
        case ZT_80211_FRM_CFENDACK           :
        {
            return to_str(ZT_80211_FRM_CFENDACK);
        }
        /* data frame */
        case ZT_80211_FRM_DATA               :
        {
            return to_str(ZT_80211_FRM_DATA);
        }
        case ZT_80211_FRM_DATA_CFACK         :
        {
            return to_str(ZT_80211_FRM_DATA_CFACK);
        }
        case ZT_80211_FRM_DATA_CFPOLL        :
        {
            return to_str(ZT_80211_FRM_DATA_CFPOLL);
        }
        case ZT_80211_FRM_DATA_CFACKPOLL     :
        {
            return to_str(ZT_80211_FRM_DATA_CFACKPOLL);
        }
        case ZT_80211_FRM_NULLFUNC           :
        {
            return to_str(ZT_80211_FRM_NULLFUNC);
        }
        case ZT_80211_FRM_CFACK              :
        {
            return to_str(ZT_80211_FRM_CFACK);
        }
        case ZT_80211_FRM_CFPOLL             :
        {
            return to_str(ZT_80211_FRM_CFPOLL);
        }
        case ZT_80211_FRM_CFACKPOLL          :
        {
            return to_str(ZT_80211_FRM_CFACKPOLL);
        }
        case ZT_80211_FRM_QOS_DATA           :
        {
            return to_str(ZT_80211_FRM_QOS_DATA);
        }
        case ZT_80211_FRM_QOS_DATA_CFACK     :
        {
            return to_str(ZT_80211_FRM_QOS_DATA_CFACK);
        }
        case ZT_80211_FRM_QOS_DATA_CFPOLL    :
        {
            return to_str(ZT_80211_FRM_QOS_DATA_CFPOLL);
        }
        case ZT_80211_FRM_QOS_DATA_CFACKPOLL :
        {
            return to_str(ZT_80211_FRM_QOS_DATA_CFACKPOLL);
        }
        case ZT_80211_FRM_QOS_NULLFUNC       :
        {
            return to_str(ZT_80211_FRM_QOS_NULLFUNC);
        }
        case ZT_80211_FRM_QOS_CFACK          :
        {
            return to_str(ZT_80211_FRM_QOS_CFACK);
        }
        case ZT_80211_FRM_QOS_CFPOLL         :
        {
            return to_str(ZT_80211_FRM_QOS_CFPOLL);
        }
        case ZT_80211_FRM_QOS_CFACKPOLL      :
        {
            return to_str(ZT_80211_FRM_QOS_CFACKPOLL);
        }
        default:
        {
            return "Unknown 80211 frame";
        }
    }
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#define STATION_INFO_INACTIVE_TIME  ZT_BIT(NL80211_STA_INFO_INACTIVE_TIME)
#define STATION_INFO_LLID           ZT_BIT(NL80211_STA_INFO_LLID)
#define STATION_INFO_PLID           ZT_BIT(NL80211_STA_INFO_PLID)
#define STATION_INFO_PLINK_STATE    ZT_BIT(NL80211_STA_INFO_PLINK_STATE)
#define STATION_INFO_SIGNAL         ZT_BIT(NL80211_STA_INFO_SIGNAL)
#define STATION_INFO_TX_BITRATE     ZT_BIT(NL80211_STA_INFO_TX_BITRATE)
#define STATION_INFO_RX_PACKETS     ZT_BIT(NL80211_STA_INFO_RX_PACKETS)
#define STATION_INFO_TX_PACKETS     ZT_BIT(NL80211_STA_INFO_TX_PACKETS)
#define STATION_INFO_TX_FAILED      ZT_BIT(NL80211_STA_INFO_TX_FAILED)
#define STATION_INFO_LOCAL_PM       ZT_BIT(NL80211_STA_INFO_LOCAL_PM)
#define STATION_INFO_PEER_PM        ZT_BIT(NL80211_STA_INFO_PEER_PM)
#define STATION_INFO_NONPEER_PM     ZT_BIT(NL80211_STA_INFO_NONPEER_PM)
#define STATION_INFO_ASSOC_REQ_IES  0
#endif


#define ZT_SSID_SCAN_AMOUNT     9
#define ZT_SCAN_IE_LEN_MAX      2304

#define ZT_MAX_NUM_PMKIDS       4

#define ZT_MAX_REMAIN_ON_CHANNEL_DURATION   5000

#define _ASOCREQ_IE_OFFSET_     4
#define _ASOCRSP_IE_OFFSET_     6
#define _REASOCREQ_IE_OFFSET_   10
#define _REASOCRSP_IE_OFFSET_   6

static const zt_u32 cipher_suites[] =
{
    WLAN_CIPHER_SUITE_WEP40,
    WLAN_CIPHER_SUITE_WEP104,
    WLAN_CIPHER_SUITE_TKIP,
    WLAN_CIPHER_SUITE_CCMP,
};


#define CH_2G4_VAL(_channel, _freq, _flags)     \
    {                                               \
        .band               = NL80211_BAND_2GHZ,    \
                              .center_freq        = (_freq),              \
                                      .hw_value           = (_channel),           \
                                              .flags              = (_flags),             \
                                                      .max_antenna_gain   = 0,                    \
                                                              .max_power          = 30,                   \
    }

static struct ieee80211_channel zt_channels_2g4[] =
{
    CH_2G4_VAL(1, 2412, 0),
    CH_2G4_VAL(2, 2417, 0),
    CH_2G4_VAL(3, 2422, 0),
    CH_2G4_VAL(4, 2427, 0),
    CH_2G4_VAL(5, 2432, 0),
    CH_2G4_VAL(6, 2437, 0),
    CH_2G4_VAL(7, 2442, 0),
    CH_2G4_VAL(8, 2447, 0),
    CH_2G4_VAL(9, 2452, 0),
    CH_2G4_VAL(10, 2457, 0),
    CH_2G4_VAL(11, 2462, 0),
    CH_2G4_VAL(12, 2467, 0),
    CH_2G4_VAL(13, 2472, 0),
    CH_2G4_VAL(14, 2484, 0),
};
#define ZT_CHANNELS_2G4         (&zt_channels_2g4[0])
#define ZT_CHANNELS_2G4_NUM     ZT_ARRAY_SIZE(zt_channels_2g4)


#define RATE_VAL(_rate, _rateid, _flags)    \
    {                                           \
        .bitrate    = (_rate),                  \
                      .hw_value   = (_rateid),                \
                                    .flags      = (_flags),                 \
    }

static struct ieee80211_rate zt_rates[] =
{
    RATE_VAL(10, 0x1, 0),
    RATE_VAL(20, 0x2, 0),
    RATE_VAL(55, 0x4, 0),
    RATE_VAL(110, 0x8, 0),
    RATE_VAL(60, 0x10, 0),
    RATE_VAL(90, 0x20, 0),
    RATE_VAL(120, 0x40, 0),
    RATE_VAL(180, 0x80, 0),
    RATE_VAL(240, 0x100, 0),
    RATE_VAL(360, 0x200, 0),
    RATE_VAL(480, 0x400, 0),
    RATE_VAL(540, 0x800, 0),
};
#define ZT_RATES_B              (&zt_rates[0])
#define ZT_RATES_B_NUM          4
#define ZT_RATES_BG             (&zt_rates[0])
#define ZT_RATES_BG_NUM         12
#define ZT_RATES_A              (&zt_rates[4])
#define ZT_RATES_A_NUM          8


static struct ieee80211_supported_band zt_band_2ghz =
{
    .channels   = ZT_CHANNELS_2G4,
    .n_channels = ZT_CHANNELS_2G4_NUM,
    .bitrates   = ZT_RATES_BG,
    .n_bitrates = ZT_RATES_BG_NUM,
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
static const struct ieee80211_txrx_stypes
    wl_cfg80211_default_mgmt_stypes[NUM_NL80211_IFTYPES] =
{
    [NL80211_IFTYPE_ADHOC] = {
        .tx = 0xffff,
        .rx = ZT_BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_STATION] = {
        .tx = 0xffff,
        .rx = ZT_BIT(IEEE80211_STYPE_ACTION >> 4) |
        ZT_BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
    },
    [NL80211_IFTYPE_AP] = {
        .tx = 0xffff,
        .rx = ZT_BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
        ZT_BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        ZT_BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        ZT_BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        ZT_BIT(IEEE80211_STYPE_AUTH >> 4) |
        ZT_BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        ZT_BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_AP_VLAN] = {

        .tx = 0xffff,
        .rx = ZT_BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
        ZT_BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        ZT_BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        ZT_BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        ZT_BIT(IEEE80211_STYPE_AUTH >> 4) |
        ZT_BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        ZT_BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_P2P_CLIENT] = {
        .tx = 0xffff,
        .rx = ZT_BIT(IEEE80211_STYPE_ACTION >> 4) |
        ZT_BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
    },
    [NL80211_IFTYPE_P2P_GO] = {
        .tx = 0xffff,
        .rx = ZT_BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
        ZT_BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        ZT_BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        ZT_BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        ZT_BIT(IEEE80211_STYPE_AUTH >> 4) |
        ZT_BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        ZT_BIT(IEEE80211_STYPE_ACTION >> 4)
    },


#if defined(RTW_DEDICATED_P2P_DEVICE)
    [NL80211_IFTYPE_P2P_DEVICE] = {
        .tx = 0xffff,
        .rx = ZT_BIT(IEEE80211_STYPE_ACTION >> 4) |
        ZT_BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
    },
#endif

};
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
struct ieee80211_iface_limit zt_limits[] =
{
    {
        .max = 2,
        .types = ZT_BIT(NL80211_IFTYPE_STATION)
#if defined(ZT_CONFIG_P2P) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE))
        | ZT_BIT(NL80211_IFTYPE_P2P_CLIENT)
#endif
    },
#ifdef CONFIG_AP_MODE
    {
        .max = 1,
        .types = ZT_BIT(NL80211_IFTYPE_AP)
#if defined(ZT_CONFIG_P2P) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE))
        | ZT_BIT(NL80211_IFTYPE_P2P_GO)
#endif
    },
#endif
};

struct ieee80211_iface_combination zt_combinations[] =
{
    {
        .limits = zt_limits,
        .n_limits = ARRAY_SIZE(zt_limits),
        .max_interfaces = 2,
        .num_different_channels = 1,
    },
};
#endif

static zt_s32 wiphy_cfg(struct wiphy *pwiphy)
{
    zt_s32 ret = 0;

    CFG80211_DBG();

    pwiphy->signal_type     = CFG80211_SIGNAL_TYPE_MBM;

    pwiphy->max_scan_ssids  = ZT_SSID_SCAN_AMOUNT;
    pwiphy->max_scan_ie_len = ZT_SCAN_IE_LEN_MAX;
    pwiphy->max_num_pmkids  = ZT_MAX_NUM_PMKIDS;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)) || defined(COMPAT_KERNEL_RELEASE)
    pwiphy->max_remain_on_channel_duration = ZT_MAX_REMAIN_ON_CHANNEL_DURATION;
#endif

    pwiphy->interface_modes = ZT_BIT(NL80211_IFTYPE_STATION)
                              | ZT_BIT(NL80211_IFTYPE_ADHOC)
#ifdef CFG_ENABLE_AP_MODE
                              | ZT_BIT(NL80211_IFTYPE_AP)
#endif
#ifdef CONFIG_WIFI_MONITOR
                              | ZT_BIT(NL80211_IFTYPE_MONITOR)
#endif
#if defined(ZT_CONFIG_P2P) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE))
                              | ZT_BIT(NL80211_IFTYPE_P2P_CLIENT)
                              | ZT_BIT(NL80211_IFTYPE_P2P_GO)
#endif

                              ;
    LOG_I("interface_modes:0x%x", pwiphy->interface_modes);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)  || defined(COMPAT_KERNEL_RELEASE)) /*&& LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0))*/
#ifdef CFG_ENABLE_AP_MODE
    pwiphy->mgmt_stypes = wl_cfg80211_default_mgmt_stypes;
#endif
#endif

#if defined(WL_SINGLE_WIPHY) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
    wiphy->iface_combinations = zt_combinations;
    wiphy->n_iface_combinations = ARRAY_SIZE(zt_combinations);
#endif

    pwiphy->cipher_suites = cipher_suites;
    pwiphy->n_cipher_suites = ZT_ARRAY_SIZE(cipher_suites);

    pwiphy->bands[NL80211_BAND_2GHZ] = &zt_band_2ghz;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38) && LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0))
    pwiphy->flags |= WIPHY_FLAG_SUPPORTS_SEPARATE_DEFAULT_KEYS;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
    pwiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;
    pwiphy->flags |= WIPHY_FLAG_HAVE_AP_SME;
#endif

#if defined(CONFIG_PM) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
    pwiphy->flags |= WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
#endif

    return ret;
}

#ifdef CFG_ENABLE_AP_MODE
static zt_s32 cfg80211_ap_set_encryption(nic_info_st *pnic_info,
        struct cfg80211_crypt *param, zt_u32 param_len)
{
    sec_info_st *psec_info = pnic_info->sec_info;
    zt_s32 res = 0;
    zt_u32 wep_key_idx, wep_key_len;
    wdn_net_info_st *pwdn_info = NULL;

    CFG80211_DBG();

    if (param_len <
            ZT_OFFSETOF(struct cfg80211_crypt, key) + param->key_len)
    {
        CFG80211_WARN("param_len invalid !!!!!!!");
        res = -EINVAL;
        goto exit;
    }

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        res = -EINVAL;
        goto exit;
    }

    CFG80211_DBG("sta_addr: "ZT_MAC_FMT, ZT_MAC_ARG(param->sta_addr));
    if (zt_80211_is_bcast_addr(param->sta_addr))
    {
        CFG80211_DBG("set with boardcast address");
        pwdn_info = zt_wdn_find_info(pnic_info, param->sta_addr);
        if (pwdn_info == NULL)
        {
            if (param->alg != WLAN_CIPHER_SUITE_WEP40 &&
                    param->alg != WLAN_CIPHER_SUITE_WEP104)
            {
                CFG80211_WARN("can't find boradcast wdn!!!");
                goto exit;
            }
        }
        if (param->idx >= ZT_80211_WEP_KEYS)
        {
            res = -EINVAL;
            goto exit;
        }
    }
    else
    {
        pwdn_info = zt_wdn_find_info(pnic_info, param->sta_addr);
        if (pwdn_info == NULL)
        {
            CFG80211_DBG("sta has already been removed or never been added");
            goto exit;
        }
    }

    /* set group key(for wpa/wpa2) or default key(for wep) before establish */
    if (zt_80211_is_bcast_addr(param->sta_addr))
    {
        /* for wep key */
        if ((param->alg == WLAN_CIPHER_SUITE_WEP40) ||
                (param->alg == WLAN_CIPHER_SUITE_WEP104))
        {
            CFG80211_DBG("crypt.alg = WEP");

            if (psec_info->wpa_unicast_cipher || psec_info->wpa_multicast_cipher ||
                    psec_info->rsn_group_cipher || psec_info->rsn_pairwise_cipher)
            {
                CFG80211_WARN("wep no support 8021x !!!");
                res = -EINVAL;
                goto exit;
            }

            wep_key_idx = param->idx;
            wep_key_len = param->key_len;
            CFG80211_DBG("wep_key_idx=%d, len=%d\n", wep_key_idx,
                         wep_key_len);

            /* check key idex and key len */
            if (wep_key_idx >= ZT_80211_WEP_KEYS || wep_key_len == 0)
            {
                res = -EINVAL;
                goto exit;
            }
            if (wep_key_len > 0)
            {
                wep_key_len = wep_key_len <= 5 ? 5 : 13; /* 5B for wep40 and 13B for wep104 */
            }

            /* TODO: tx=1, the key only used to encrypt data in data send process,
            that is to say no used for boradcast */

            if (psec_info->bWepDefaultKeyIdxSet == 0)
            {
                CFG80211_DBG("wep, bWepDefaultKeyIdxSet=0");
                /* update encrypt algorithm */
                psec_info->ndisencryptstatus = zt_ndis802_11Encryption1Enabled;
                if (wep_key_len == 13)
                {
                    psec_info->dot11PrivacyAlgrthm = _WEP104_;
                    psec_info->dot118021XGrpPrivacy = _WEP104_;
                }
                else
                {
                    psec_info->dot11PrivacyAlgrthm = _WEP40_;
                    psec_info->dot118021XGrpPrivacy = _WEP40_;
                }
                psec_info->dot11PrivacyKeyIndex = wep_key_idx;
            }

            /* todo: force wep key id set to 0, other id value no used by STA */
            if (wep_key_idx == 0)
            {
                /* update default key(for wep) */
                psec_info->dot11PrivacyKeyIndex = wep_key_idx;
                zt_memcpy(&psec_info->dot11DefKey[wep_key_idx].skey[0],
                          param->key, wep_key_len);
                psec_info->dot11DefKeylen[wep_key_idx] = wep_key_len;
            }

            goto exit;
        }

        /* for group key */
        if (param->alg == WLAN_CIPHER_SUITE_TKIP)
        {
            CFG80211_DBG("set group_key, TKIP");

            psec_info->dot118021XGrpPrivacy = _TKIP_;

            /* KCK PTK0~127 */
            psec_info->dot118021XGrpKeyid = param->idx;
            zt_memcpy(psec_info->dot118021XGrpKey[param->idx].skey,
                      param->key, ZT_MIN(param->key_len, 16));
            /* set mic key */
            /* KEK PTK128~255 */
            zt_memcpy(psec_info->dot118021XGrptxmickey[param->idx].skey,
                      &param->key[16], 8); /* PTK128~191 */
            zt_memcpy(psec_info->dot118021XGrprxmickey[param->idx].skey,
                      &param->key[24], 8); /* PTK192~255 */

            psec_info->busetkipkey = zt_true;
        }
        else if (param->alg == WLAN_CIPHER_SUITE_CCMP)
        {
            CFG80211_DBG("set group_key, CCMP");
            psec_info->dot118021XGrpPrivacy = _AES_;
            /* KCK PTK0~127 */
            psec_info->dot118021XGrpKeyid = param->idx;
            CFG80211_DBG("set group_key id(%d), CCMP", psec_info->dot118021XGrpKeyid);
            zt_memcpy(psec_info->dot118021XGrpKey[param->idx].skey,
                      param->key, ZT_MIN(param->key_len, 16));
            zt_sec_ap_set_group_key(pnic_info, &pwdn_info->group_cam_id, param->sta_addr);
        }
        else
        {
            CFG80211_DBG("set group_key, none");
            goto exit;
        }

        psec_info->dot11PrivacyAlgrthm = psec_info->dot118021XGrpPrivacy;
        psec_info->binstallGrpkey = zt_true;

        /* set boardcast wdn */
        pwdn_info = zt_wdn_find_info(pnic_info, param->sta_addr);
        if (pwdn_info)
        {
            pwdn_info->dot118021XPrivacy = psec_info->dot118021XGrpPrivacy;
            pwdn_info->ieee8021x_blocked = zt_false;
            pwdn_info->state = E_WDN_AP_STATE_8021X_UNBLOCK;
        }

        goto exit;
    }

    /* set key(for wpa/wpa2) after establish */
    else if (psec_info->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X)
    {
        if (param->set_tx == 1)
        {
            CFG80211_DBG("set unicastkey");

            zt_memcpy(pwdn_info->dot118021x_UncstKey.skey, param->key,
                      ZT_MIN(param->key_len, 16));

            if (param->alg == WLAN_CIPHER_SUITE_TKIP)
            {
                CFG80211_DBG("set pairwise key, TKIP");
                pwdn_info->dot118021XPrivacy = _TKIP_;
                /* set mic key */
                zt_memcpy(pwdn_info->dot11tkiptxmickey.skey,
                          &param->key[16], 8);
                zt_memcpy(pwdn_info->dot11tkiprxmickey.skey,
                          &param->key[24], 8);
                psec_info->busetkipkey = zt_true;
            }
            else if (param->alg == WLAN_CIPHER_SUITE_CCMP)
            {
                CFG80211_DBG("set pairwise key, CCMP");
                pwdn_info->dot118021XPrivacy = _AES_;
                /* enable hardware encrypt */
                zt_sec_ap_set_unicast_key(pnic_info, &pwdn_info->unicast_cam_id,
                                          pwdn_info->dot118021XPrivacy,
                                          pwdn_info->mac, pwdn_info->dot118021x_UncstKey.skey);
                zt_mcu_set_dk_cfg(pnic_info, psec_info->dot11AuthAlgrthm, zt_true);
                zt_mcu_set_on_rcr_am(pnic_info, zt_true);
            }
            else if (param->alg == IW_ENCODE_ALG_NONE)
            {
                CFG80211_DBG("crypt.alg: none");
                pwdn_info->dot118021XPrivacy = _NO_PRIVACY_;
            }
        }

        pwdn_info->ieee8021x_blocked = zt_false;
        pwdn_info->state = E_WDN_AP_STATE_8021X_UNBLOCK;
    }

exit:
    return res;
}
#endif
static zt_s32 zt_cfg80211_sta_set_encryption(struct net_device *dev,
        struct cfg80211_crypt *param, zt_u32 param_len)
{
    ndev_priv_st *pndev_priv = netdev_priv(dev);
    nic_info_st *pnic_info = pndev_priv->nic;
    sec_info_st *psec_info = pnic_info->sec_info;
    wdn_net_info_st *pwdn_info;
    zt_s32 res = 0;

    CFG80211_DBG();

    if (param_len !=
            ZT_OFFSETOF(struct cfg80211_crypt, key) + param->key_len)
    {
        CFG80211_WARN("param_len invalid !!!!!!!");
        res = -EINVAL;
        goto exit;
    }

    if (zt_80211_is_bcast_addr(param->sta_addr))
    {
        if (param->idx >= ZT_80211_WEP_KEYS)
        {
            res = -EINVAL;
            goto exit;
        }
    }
    else
    {
        res = -EINVAL;
        goto exit;
    }

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        res = -EINVAL;
        goto exit;
    }

    if (psec_info->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) /* 802_1x */
    {
        local_info_st *plocal_info = pnic_info->local_info;
        if (plocal_info->work_mode == ZT_INFRA_MODE) /* sta mode */
        {
            pwdn_info = zt_wdn_find_info(pnic_info,
                                         zt_wlan_get_cur_bssid(pnic_info));
            if (pwdn_info == NULL)
            {
                CFG80211_WARN("pwdn_info NULL !!!!!!");
                goto exit;
            }

            if (param->alg != IW_ENCODE_ALG_NONE)
            {
                pwdn_info->ieee8021x_blocked = zt_false;
            }

            if (psec_info->ndisencryptstatus == zt_ndis802_11Encryption2Enabled ||
                    psec_info->ndisencryptstatus == zt_ndis802_11Encryption3Enabled)
            {
                pwdn_info->dot118021XPrivacy = psec_info->dot11PrivacyAlgrthm;
            }
            CFG80211_DBG("pwdn_info->dot118021XPrivacy = %d", pwdn_info->dot118021XPrivacy);

            zt_mcu_set_sec_cfg(pnic_info,
                               psec_info->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X ? 0xcf : 0xcc);

            /* PTK: param->u.crypt.key */
            if (param->set_tx == 1) /* pairwise key */
            {
                CFG80211_DBG("set unicastkey");
                /* KCK PTK0~127 */
                zt_memcpy(pwdn_info->dot118021x_UncstKey.skey, param->key,
                          min_t(zt_u16, param->key_len, 16));

                if (param->alg == WLAN_CIPHER_SUITE_TKIP) /* set mic key */
                {
                    /* KEK PTK128~255 */
                    zt_memcpy(pwdn_info->dot11tkiptxmickey.skey,
                              &(param->key[16]), 8); /* PTK128~191 */
                    zt_memcpy(pwdn_info->dot11tkiprxmickey.skey,
                              &(param->key[24]), 8); /* PTK192~255 */
                    psec_info->busetkipkey = zt_true;
                }
                if (param->alg == WLAN_CIPHER_SUITE_CCMP)
                {
                    CFG80211_DBG("sta_hw_set_unicast_key");
                    zt_sec_sta_set_unicast_key(pnic_info, &pwdn_info->unicast_cam_id,
                                               pwdn_info->dot118021XPrivacy,
                                               pwdn_info->mac, pwdn_info->dot118021x_UncstKey.skey);
                }
            }
            else /* group key */
            {
                CFG80211_DBG("set groupkey");
                zt_memcpy(psec_info->dot118021XGrpKey[param->idx].skey,
                          param->key,
                          min_t(zt_u16, param->key_len, 16));

                zt_memcpy(psec_info->dot118021XGrptxmickey[param->idx].skey,
                          &param->key, 8);
                zt_memcpy(psec_info->dot118021XGrprxmickey[param->idx].skey,
                          &param->key[8], 8);
                psec_info->binstallGrpkey = true;
                psec_info->dot118021XGrpKeyid = param->idx;
                if (psec_info->dot118021XGrpPrivacy == _AES_)
                {
                    CFG80211_DBG("sta_hw_set_group_key");
                    zt_sec_sta_set_group_key(pnic_info, &pwdn_info->group_cam_id, pwdn_info->bssid);
                }

                if (zt_p2p_is_valid(pnic_info))
                {
                    p2p_info_st *p2p_info = pnic_info->p2p;
                    CFG80211_INFO("state:%s", zt_p2p_state_to_str(p2p_info->p2p_state));
                    if (p2p_info->p2p_state == P2P_STATE_GONEGO_OK)
                    {
                        zt_p2p_set_state(p2p_info, P2P_STATE_EAPOL_DONE);
                        zt_p2p_nego_timer_set(pnic_info, P2P_EAPOL_NEGO_TIME);
                    }
                }

            }
        }
    }

exit:
    return res;
}

void zt_cfg80211_wiphy_unreg(nic_info_st *pnic_info)
{
    CFG80211_INFO("wiphy unregiester");
    wiphy_unregister(pnic_info->pwiphy);
}

void zt_cfg80211_wiphy_free(nic_info_st *pnic_info)
{
    struct wiphy *pwiphy;

    if (NULL == pnic_info)
    {
        return;
    }

    pwiphy = pnic_info->pwiphy;
    if (NULL == pwiphy)
    {
        return;
    }

    wiphy_free(pwiphy);
}

void zt_cfg80211_widev_free(nic_info_st *pnic_info)
{
    struct wireless_dev *pwidev;

    if (NULL == pnic_info)
    {
        return;
    }

    pwidev = pnic_info->pwidev;
    if (pwidev)
    {
        zt_kfree(pwidev);
    }
}

static inline void set_wiphy_pirv(struct wiphy *pwiphy, void *priv)
{
    *(void **)wiphy_priv(pwiphy) = priv;
}

static zt_s32 _add_key_cb(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
                          struct wireless_dev *wdev,
#else
                          struct net_device *ndev,
#endif
/*TODO: android #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 119))*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
                          int link_id,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
                          zt_u8 key_index, bool pairwise,
                          const zt_u8 *mac_addr,
#else
                          zt_u8 key_index, const zt_u8 *mac_addr,
#endif
                          struct key_params *params)
{
    ndev_priv_st *pndev_priv;
    nic_info_st *pnic_info;
    zt_u32 param_len;
    struct cfg80211_crypt *param = NULL;
    zt_s32 res = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
    struct net_device *ndev = wdev->netdev;
#endif
    pndev_priv = netdev_priv(ndev);
    pnic_info = pndev_priv->nic;

    CFG80211_INFO("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
    CFG80211_DBG("pairwise=%d\n", pairwise);
#endif

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        return -EINVAL;
    }

    param_len = ZT_OFFSETOF(struct cfg80211_crypt, key) + params->key_len;
    param = (struct cfg80211_crypt *)zt_kzalloc(param_len);
    if (param == NULL)
    {
        res = -EPERM;
        goto exit;
    }
    zt_memset(param, 0, param_len);

    zt_memset(param->sta_addr, 0xff, ETH_ALEN);

    param->alg = params->cipher;

    if (mac_addr == NULL || zt_80211_is_bcast_addr(mac_addr))
    {
        param->set_tx = 0;
    }
    else
    {
        param->set_tx = 1;
    }

    param->idx = key_index;
    CFG80211_DBG("cfg80211_crypt.idx=%d", param->idx);

    if (params->key_len && params->key)
    {
        param->key_len = params->key_len;
        zt_memcpy(param->key, (zt_u8 *) params->key, params->key_len);
    }
    if (zt_local_cfg_get_work_mode(pnic_info) == ZT_INFRA_MODE)
    {
        res = zt_cfg80211_sta_set_encryption(ndev, param, param_len);
    }
#ifdef CFG_ENABLE_AP_MODE
    else if (zt_local_cfg_get_work_mode(pnic_info) == ZT_MASTER_MODE)
    {
        if (mac_addr)
        {
            zt_memcpy(param->sta_addr, (void *)mac_addr, ETH_ALEN);
        }
        res = cfg80211_ap_set_encryption(pnic_info, param, param_len);
    }
#endif
    else
    {
        CFG80211_WARN("mode error!");
    }

exit :
    if (param)
    {
        zt_kfree((zt_u8 *)param);
    }

    return res;

}

static zt_s32 _get_key_cb(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
                          struct wireless_dev *wdev,
#else
                          struct net_device *ndev,
#endif
/*TODO: android #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 119))*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
                          int link_id,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
                          zt_u8 key_index, bool pairwise,
                          const zt_u8 *mac_addr,
#else
                          zt_u8 key_index, const zt_u8 *mac_addr,
#endif
                          void *cookie,
                          void (*callback)(void *cookie,
                                  struct key_params *))
{
    if (mac_addr)
    {
        CFG80211_INFO("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(mac_addr));
    }
    else
    {
        CFG80211_INFO();
    }
    return 0;
}


static zt_s32 _del_key_cb(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
                          struct wireless_dev *wdev,
#else
                          struct net_device *ndev,
#endif
/*TODO: android #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 119))*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
                          int link_id,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
                          zt_u8 key_index, bool pairwise,
                          const zt_u8 *mac_addr)
#else
                          zt_u8 key_index, const zt_u8 *mac_addr)
#endif
{
    ndev_priv_st *pndev_priv;
    nic_info_st *pnic_info;
    sec_info_st *psec_info;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
    struct net_device *ndev = wdev->netdev;
#endif
    pndev_priv = netdev_priv(ndev);
    pnic_info = pndev_priv->nic;
    psec_info = pnic_info->sec_info;

    CFG80211_INFO("key_index = %d", key_index);
    CFG80211_INFO("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    if (key_index == psec_info->dot11PrivacyKeyIndex)
    {
        psec_info->bWepDefaultKeyIdxSet = 0;
    }
    return 0;
}


static zt_s32 _set_default_key_cb(struct wiphy *wiphy,
                                  struct net_device *ndev,
/*TODO: android #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 137))*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
                                  int link_id,
#endif
                                  zt_u8 key_index
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)) || defined(COMPAT_KERNEL_RELEASE)
                                  , bool unicast, bool multicast
#endif
                                 )
{
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;
    sec_info_st *psec_info = pnic_info->sec_info;

    CFG80211_INFO("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    if (key_index < ZT_80211_WEP_KEYS &&
            (psec_info->dot11PrivacyAlgrthm == _WEP40_ ||
             psec_info->dot11PrivacyAlgrthm == _WEP104_))
    {
        psec_info->ndisencryptstatus = zt_ndis802_11Encryption1Enabled;

        psec_info->dot11PrivacyKeyIndex = key_index;

        psec_info->dot11PrivacyAlgrthm = _WEP40_;
        psec_info->dot118021XGrpPrivacy = _WEP40_;
        if (psec_info->dot11DefKeylen[key_index] == 13)
        {
            psec_info->dot11PrivacyAlgrthm = _WEP104_;
            psec_info->dot118021XGrpPrivacy = _WEP104_;
        }

        psec_info->bWepDefaultKeyIdxSet = 1;
    }

    return 0;
}


static zt_s32 _cfg80211_get_station(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
                                    struct wireless_dev *wdev,
#else
                                    struct net_device *ndev,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0))
                                    zt_u8 *mac,
#else
                                    const zt_u8 *mac,
#endif
                                    struct station_info *sinfo)
{
    ndev_priv_st *pndev_priv        = NULL;
    nic_info_st *pnic_info          = NULL;
    wdn_net_info_st *pwdn_net_info  = NULL;
    zt_u8 qual, level;
    zt_u16 max_rate = 0;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(mac));
    if (NULL == mac)
    {
        CFG80211_DBG("mac is null");
        return -ENOENT;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
    if (wdev->netdev == NULL)
    {
        CFG80211_DBG("netdev is null");
        return -ENOENT;
    }
    pndev_priv = netdev_priv(wdev->netdev);
#else
    pndev_priv = netdev_priv(ndev);
#endif
    if (NULL == pndev_priv)
    {
        CFG80211_DBG("pndev_priv is null");
        return -ENOENT;
    }

    pnic_info = pndev_priv->nic;
    if (NULL == pnic_info)
    {
        CFG80211_DBG("pnic_info is null");
        return -ENOENT;
    }

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    pwdn_net_info = zt_wdn_find_info(pnic_info, (zt_u8 *)mac);
    if (NULL == pwdn_net_info)
    {
        return -ENOENT;
    }

    zt_wlan_get_signal_and_qual(pnic_info, &qual, &level);
    zt_wlan_get_max_rate(pnic_info, (zt_u8 *)mac, &max_rate);

    sinfo->filled = 0;

    sinfo->filled |= STATION_INFO_SIGNAL;
    sinfo->signal = translate_percentage_to_dbm(level);

    sinfo->filled |= STATION_INFO_TX_BITRATE;
    sinfo->txrate.legacy = max_rate;

    sinfo->filled |= STATION_INFO_RX_PACKETS;
    sinfo->rx_packets = pwdn_net_info->wdn_stats.rx_pkts;

    sinfo->filled |= STATION_INFO_TX_PACKETS;
    sinfo->tx_packets = pwdn_net_info->wdn_stats.tx_pkts;

    sinfo->filled |= STATION_INFO_TX_FAILED;
    sinfo->tx_failed = pwdn_net_info->wdn_stats.tx_drops;

    return 0;
}



static zt_s32 _cfg80211_change_iface(struct wiphy *wiphy,
                                     struct net_device *ndev,
                                     enum nl80211_iftype type,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
                                     zt_u32 *flags,
#endif
                                     struct vif_params *params)
{
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;
#ifdef CFG_ENABLE_AP_MODE
    sec_info_st *psec_info = pnic_info->sec_info;
#endif
    struct wireless_dev *pwidev = pnic_info->pwidev;
    mlme_info_t *pmlme_info = pnic_info->mlme_info;
    enum nl80211_iftype old_type;
    sys_work_mode_e network_type = ZT_INFRA_MODE;
    zt_u8 change;
    p2p_info_st *p2p_info   = pnic_info->p2p;

#ifdef CFG_ENABLE_ADHOC_MODE
    zt_bool bConnected;
    zt_bool bAdhocMaster;
#endif
    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

#ifdef CFG_ENABLE_AP_MODE
    psec_info->wpa_unicast_cipher = 0;
    psec_info->wpa_multicast_cipher = 0;
    psec_info->rsn_group_cipher = 0;
    psec_info->rsn_pairwise_cipher = 0;
#endif

    if (ZT_CANNOT_RUN(pnic_info))
    {
        return -EPERM;
    }

    if (ndev_open(ndev) != 0)
    {
        return -EINVAL;
    }

#ifdef CONFIG_LPS
    if (ZT_RETURN_FAIL == zt_lps_wakeup(pnic_info, LPS_CTRL_SCAN, 0))
    {
        return -EPERM;
    }
#endif

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        return -EPERM;
    }

    old_type = pwidev->iftype;
    if (old_type != type)
    {
        change = zt_true;
        pmlme_info->action_public_dialog_token = 0xff;
        pmlme_info->action_public_rxseq = 0xffff;
    }
    CFG80211_INFO("old_type:%d, type:%d", old_type, type);

    ndev->type = ARPHRD_ETHER;

    if (type == NL80211_IFTYPE_MONITOR)
    {
    }
    else if (old_type == NL80211_IFTYPE_MONITOR)
    {
    }

    switch (type)
    {
#ifdef CFG_ENABLE_ADHOC_MODE
        case NL80211_IFTYPE_ADHOC:
            network_type = ZT_ADHOC_MODE;
            break;
#endif

        case NL80211_IFTYPE_STATION:
            network_type = ZT_INFRA_MODE;
            if (zt_p2p_is_valid(pnic_info))
            {
                CFG80211_INFO("DRIVER_CFG80211: %s", zt_p2p_role_to_str(p2p_info->role));
                zt_p2p_reset(pnic_info);
                if (change && P2P_ROLE_GO == p2p_info->role)
                {
                    zt_p2p_set_role(p2p_info, P2P_ROLE_DEVICE);
                    zt_p2p_set_state(p2p_info, p2p_info->pre_p2p_state);
                    CFG80211_INFO("role=%d, p2p_state=%d, pre_p2p_state=%d\n",
                                  p2p_info->role, p2p_info->p2p_state, p2p_info->pre_p2p_state);
                }

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE))
                if (p2p_info->role == P2P_ROLE_CLIENT)
                {
                    zt_p2p_set_role(p2p_info, P2P_ROLE_DEVICE);
                }
#endif
            }
            break;

#ifdef CFG_ENABLE_AP_MODE
        case NL80211_IFTYPE_AP:
            network_type = ZT_MASTER_MODE;
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE))
            if (zt_p2p_is_valid(pnic_info))
            {
                CFG80211_INFO("DRIVER_CFG80211, %s", zt_p2p_role_to_str(p2p_info->role));
                if (change && p2p_info->p2p_state != P2P_STATE_NONE)
                {
                    zt_p2p_set_role(p2p_info, P2P_ROLE_GO);
                }

            }
#endif
            zt_mlme_abort(pnic_info);
            break;
#endif

#ifdef CFG_ENABLE_MONITOR_MODE
        case NL80211_IFTYPE_MONITOR:
            network_type = ZT_MONITOR_MODE;
            ndev->type = ARPHRD_IEEE80211_RADIOTAP;
            break;
#endif

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE))
        case NL80211_IFTYPE_P2P_CLIENT:
            CFG80211_INFO("NL80211_IFTYPE_P2P_CLIENT");
            network_type = ZT_INFRA_MODE;
            if (zt_p2p_is_valid(pnic_info))
            {
                CFG80211_INFO("DRIVER_CFG80211, %s", zt_p2p_role_to_str(p2p_info->role));
                if (change && P2P_ROLE_GO == p2p_info->role)
                {
                    zt_p2p_set_role(p2p_info, P2P_ROLE_DEVICE);
                    zt_p2p_set_state(p2p_info, p2p_info->pre_p2p_state);
                    CFG80211_INFO("%s, role=%d, p2p_state=%d, pre_p2p_state=%d\n", __func__,
                                  p2p_info->role, p2p_info->p2p_state, p2p_info->pre_p2p_state);
                }
                zt_p2p_set_role(p2p_info, P2P_ROLE_CLIENT);

            }
            break;

#ifdef CFG_ENABLE_AP_MODE
        case NL80211_IFTYPE_P2P_GO:
        {
            CFG80211_INFO("NL80211_IFTYPE_P2P_GO, %s", zt_p2p_role_to_str(p2p_info->role));
            network_type = ZT_MASTER_MODE;

            if (change && p2p_info->p2p_state != P2P_STATE_NONE)
            {
                zt_p2p_set_role(p2p_info, P2P_ROLE_GO);
            }

        }
        break;
#endif
#endif

        default:
            CFG80211_ERROR("op type error");
            return -EPERM;
    }
    pwidev->iftype = type;

#ifdef CFG_ENABLE_ADHOC_MODE
    if (NL80211_IFTYPE_ADHOC == old_type && NL80211_IFTYPE_ADHOC != type)
    {
        zt_mlme_get_connect(pnic_info, &bConnected);
        bAdhocMaster = zt_get_adhoc_master(pnic_info);
        if (bConnected && bAdhocMaster)
        {
            zt_adhoc_leave_ibss_msg_send(pnic_info);
            zt_yield();
        }
        zt_adhoc_flush_all_resource(pnic_info, network_type);
    }
#endif

    CFG80211_DBG("mode == %d", type);
    zt_memset(pnic_info->sec_info, 0, sizeof(sec_info_st));
    zt_local_cfg_set_work_mode(pnic_info, network_type);
    zt_mcu_set_op_mode(pnic_info, network_type);

    return 0;
}

void zt_cfg80211_scan_done_event_up(nic_info_st *pnic_info, zt_bool babort)
{
    struct zt_widev_priv *pwdev_info = pnic_info->widev_priv;
#if (KERNEL_VERSION(4, 7, 0) <= LINUX_VERSION_CODE)
    struct cfg80211_scan_info info;

    zt_memset(&info, 0, sizeof(info));
    info.aborted = babort;
#endif
    zt_os_api_lock_lock(&pwdev_info->scan_req_lock);
    if (pwdev_info->pscan_request != NULL)
    {
        LOG_W("[%s]: with scan req", __func__);
        if (pwdev_info->pscan_request->wiphy == pnic_info->pwiphy)
        {
#if (KERNEL_VERSION(4, 7, 0) <= LINUX_VERSION_CODE)
            cfg80211_scan_done(pwdev_info->pscan_request, &info);
#else
            cfg80211_scan_done(pwdev_info->pscan_request, babort);
#endif
        }
        else
        {
            LOG_W("[%s]: wrong compare", __func__);
        }
        pwdev_info->pscan_request = NULL;
    }
    else
    {
        LOG_W("[%s]: scan req is NULL", __func__);
    }
    zt_os_api_lock_unlock(&pwdev_info->scan_req_lock);
}

static zt_u64 cfg80211_get_timestamp_us(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0))
    return ktime_to_us(ktime_get_boottime());
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
    struct timespec ts;
    get_monotonic_boottime(&ts);
    return ((zt_u64) ts.tv_sec * 1000000) + ts.tv_nsec / 1000;
#else
    struct timeval tv;
    do_gettimeofday(&tv);
    return ((zt_u64) tv.tv_sec * 1000000) + tv.tv_usec;
#endif
}

#define MAX_BSSINFO_LEN 1000
struct cfg80211_bss *inform_bss(nic_info_st *pnic_info,
                                zt_wlan_mgmt_scan_que_node_t *pscan_que_node)
{
    zt_u8 *pbuf;
    zt_u16 frame_len;
    zt_80211_frame_e frame_type;
    zt_80211_mgmt_t *pframe;
    struct ieee80211_channel *channel;
    zt_s32 signal_dbm;
    struct cfg80211_bss *pbss = NULL;
    zt_u8 bc_addr[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    struct wireless_dev *pwdev = pnic_info->pwidev;
    struct wiphy *pwiphy = pwdev->wiphy;

    pbuf = zt_kzalloc(MAX_BSSINFO_LEN);
    if (pbuf == NULL)
    {
        CFG80211_WARN("buffer alloc failed!");
        return pbss;
    }
    zt_memset(pbuf, 0, MAX_BSSINFO_LEN);

    frame_len = ZT_OFFSETOF(zt_80211_mgmt_t, beacon) + pscan_que_node->ie_len;
    if (frame_len > MAX_BSSINFO_LEN)
    {
        CFG80211_WARN("ie_length is too long");
        goto exit;
    }

    /**
     * make frame
     */
    pframe = (void *)pbuf;
    /* frame control */
    pframe->frame_control = 0;
    /* frame type */
    frame_type = pscan_que_node->frame_type;
    zt_80211_hdr_type_set(pframe, frame_type);
    /* address */
    zt_memcpy(pframe->da,
              frame_type == ZT_80211_FRM_BEACON ?
              bc_addr : nic_to_local_addr(pnic_info),
              ETH_ALEN);
    zt_memcpy(pframe->sa, pscan_que_node->bssid, ETH_ALEN);
    zt_memcpy(pframe->bssid, pscan_que_node->bssid, ETH_ALEN);
    /* sequence control */
    pframe->seq_ctrl = 0;
    /* element */
    void *tmp_beacon = (zt_s8 *)pframe + ZT_OFFSETOF(zt_80211_mgmt_t, beacon);
    zt_memcpy(tmp_beacon, pscan_que_node->ies, pscan_que_node->ie_len);
    /* timestamp */
    pframe->beacon.timestamp = zt_cpu_to_le64(cfg80211_get_timestamp_us());

    channel =
        ieee80211_get_channel(pwiphy, zt_ch_2_freq((zt_s32)pscan_que_node->channel));
    signal_dbm =
        translate_percentage_to_dbm(pscan_que_node->signal_strength) *
        100; /* mBm (100*dBm) */
    pbss = cfg80211_inform_bss_frame(pwiphy, channel,
                                     (void *)pframe, frame_len,
                                     signal_dbm, GFP_ATOMIC);
    if (unlikely(!pbss))
    {
        CFG80211_WARN("pbss is NULL %s channel %d", pscan_que_node->ssid.data,
                      pscan_que_node->channel);
        goto exit;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 38) && !defined COMPAT_KERNEL_RELEASE
    if (frame_type == ZT_80211_FRM_BEACON)
    {
        if (pbss->len_information_elements != pbss->len_beacon_ies)
        {
            pbss->information_elements = pbss->beacon_ies;
            pbss->len_information_elements = pbss->len_beacon_ies;
        }
    }
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
    cfg80211_put_bss(pwiphy, pbss);
#else
    cfg80211_put_bss(pbss);
#endif

exit:
    if (pbuf)
    {
        zt_kfree(pbuf);
    }

    return pbss;
}

zt_s32 zt_cfg80211_inform_check_bss(nic_info_st *pnic_info)
{
    zt_bool privacy;
    zt_wlan_mgmt_info_t *wlan_mgmt_info = pnic_info->wlan_mgmt_info;
    zt_wlan_network_t *pcur_network = &wlan_mgmt_info->cur_network;
    struct cfg80211_bss *bss = NULL;
    struct ieee80211_channel *channel = NULL;
    struct wireless_dev *pwdev = pnic_info->pwidev;
    struct wiphy *pwiphy = pwdev->wiphy;

    privacy = !!(pcur_network->cap_info & ZT_80211_MGMT_CAPAB_PRIVACY);
    channel = ieee80211_get_channel(pwiphy,
                                    zt_ch_2_freq((zt_s32)pcur_network->channel));
    bss = cfg80211_get_bss(pwiphy, channel,
                           pcur_network->bssid, pcur_network->ssid.data,
                           pcur_network->ssid.length,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
                           (ZT_80211_CAPAB_IS_IBSS(pcur_network->cap_info) ?
                            IEEE80211_BSS_TYPE_IBSS : IEEE80211_BSS_TYPE_ESS),
                           IEEE80211_PRIVACY(privacy));
#else
                           ZT_80211_MGMT_CAPAB_ESS,
                           ZT_80211_MGMT_CAPAB_IBSS);
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
    cfg80211_put_bss(pwiphy, bss);
#else
    cfg80211_put_bss(bss);
#endif

    return (bss != NULL);
}

#ifdef CFG_ENABLE_ADHOC_MODE
void zt_cfg80211_unlink_ibss(nic_info_st *pnic_info)
{
    zt_bool privacy;
    zt_wlan_mgmt_info_t *wlan_mgmt_info = pnic_info->wlan_mgmt_info;
    zt_wlan_network_t *pcur_network = &wlan_mgmt_info->cur_network;
    struct cfg80211_bss *bss = NULL;
    struct ieee80211_channel *channel = NULL;
    struct wireless_dev *pwdev = pnic_info->pwidev;
    struct wiphy *pwiphy = pwdev->wiphy;

    privacy = !!(pcur_network->cap_info & ZT_80211_MGMT_CAPAB_PRIVACY);
    channel = ieee80211_get_channel(pwiphy,
                                    zt_ch_2_freq((zt_s32)pcur_network->channel));
    bss = cfg80211_get_bss(pwiphy, channel,
                           pcur_network->bssid, pcur_network->ssid.data,
                           pcur_network->ssid.length,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
                           (ZT_80211_CAPAB_IS_IBSS(pcur_network->cap_info) ?
                            IEEE80211_BSS_TYPE_IBSS : IEEE80211_BSS_TYPE_ESS),
                           IEEE80211_PRIVACY(privacy));
#else
                           ZT_80211_MGMT_CAPAB_ESS,
                           ZT_80211_MGMT_CAPAB_IBSS);
#endif

    if (bss)
    {
        cfg80211_unlink_bss(pwiphy, bss);
        CFG80211_INFO("%s(): cfg80211_unlink %s!! () ", __func__,
                      pcur_network->ssid.data);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
        cfg80211_put_bss(pwiphy, bss);
#else
        cfg80211_put_bss(bss);
#endif
    }
    return;
}

void zt_cfg80211_ibss_indicate_connect(nic_info_st *pnic_info)
{
    struct wireless_dev *pwdev = (struct wireless_dev *)pnic_info->pwidev;
    zt_wlan_mgmt_info_t *wlan_mgmt_info = pnic_info->wlan_mgmt_info;
    zt_wlan_network_t *pcur_network = &wlan_mgmt_info->cur_network;
    zt_wlan_mgmt_scan_que_node_t *pscan_que_node = NULL;
    zt_wlan_mgmt_scan_que_for_rst_e scan_que_for_rst;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
    struct wiphy *wiphy = pwdev->wiphy;
    zt_u32 freq = 2412;
    struct ieee80211_channel *notify_channel;
#endif

    CFG80211_DBG();

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
    freq = zt_ch_2_freq((zt_s32)pcur_network->channel);
    LOG_D("freq = %d", freq);
#endif
    pwdev->iftype = NL80211_IFTYPE_ADHOC;

    if (zt_local_cfg_get_work_mode(pnic_info) != ZT_MONITOR_MODE)
    {
        zt_wlan_mgmt_scan_que_for_begin(pnic_info, pscan_que_node)
        {
            if (!zt_memcmp(pscan_que_node->bssid, pcur_network->bssid, ETH_ALEN) &&
                    !zt_memcmp(pscan_que_node->ssid.data, pcur_network->ssid.data,
                               pcur_network->ssid.length))
            {
                CFG80211_DBG("INFORM BSS before event up, ssid %s, ssid %s",
                             pscan_que_node->ssid.data,
                             pcur_network->ssid.data);
                inform_bss(pnic_info, pscan_que_node);
            }
        }
        zt_wlan_mgmt_scan_que_for_end(scan_que_for_rst);
    }

    if (!zt_cfg80211_inform_check_bss(pnic_info))
    {
        CFG80211_DBG("bss not found!!");
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
    notify_channel = ieee80211_get_channel(wiphy, freq);
    cfg80211_ibss_joined(pnic_info->ndev, pcur_network->bssid,
                         notify_channel, GFP_ATOMIC);
#else
    cfg80211_ibss_joined(pnic_info->ndev, pcur_network->bssid,
                         GFP_ATOMIC);
#endif
}
#endif

void zt_cfg80211_indicate_connect(nic_info_st *pnic_info)
{
    struct wireless_dev *pwdev = (struct wireless_dev *)pnic_info->pwidev;
    zt_wlan_mgmt_info_t *wlan_mgmt_info = pnic_info->wlan_mgmt_info;
    zt_wlan_network_t *pcur_network = &wlan_mgmt_info->cur_network;
    zt_wlan_mgmt_scan_que_node_t *pscan_que_node = NULL;
    zt_wlan_mgmt_scan_que_for_rst_e scan_que_for_rst;

    CFG80211_DBG();

    if (pwdev->iftype != NL80211_IFTYPE_STATION
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
            && pwdev->iftype != NL80211_IFTYPE_P2P_CLIENT
#endif
       )
    {
        return;
    }

#ifdef CFG_ENABLE_AP_MODE
    if (zt_local_cfg_get_work_mode(pnic_info) == ZT_MASTER_MODE)
    {
        return;
    }
#endif

#ifdef CFG_ENABLE_MONITOR_MODE
    if (zt_local_cfg_get_work_mode(pnic_info) != ZT_MONITOR_MODE)
#endif
    {
        zt_wlan_mgmt_scan_que_for_begin(pnic_info, pscan_que_node)
        {
            if (!zt_memcmp(pscan_que_node->bssid, pcur_network->bssid, ETH_ALEN) &&
                    !zt_memcmp(pscan_que_node->ssid.data, pcur_network->ssid.data,
                               pcur_network->ssid.length))
            {
                CFG80211_DBG("INFORM BSS before event up, ssid %s, ssid %s, bss_ch:%d, scan_ch:%d, bssid:"ZT_MAC_FMT,
                             pscan_que_node->ssid.data
                             , pcur_network->ssid.data
                             , pcur_network->channel
                             , pscan_que_node->channel
                             , ZT_MAC_ARG(pscan_que_node->bssid)
                            );
                inform_bss(pnic_info, pscan_que_node);
            }
        }
        zt_wlan_mgmt_scan_que_for_end(scan_que_for_rst);
    }

    if (!zt_cfg80211_inform_check_bss(pnic_info))
    {
        CFG80211_INFO("bss not found!!");
    }
    if (zt_p2p_is_valid(pnic_info))
    {
        p2p_info_st *p2p_info           = pnic_info->p2p;
        zt_p2p_set_pre_state(p2p_info, p2p_info->p2p_state);
        zt_p2p_set_role(p2p_info, P2P_ROLE_CLIENT);
        zt_p2p_set_state(p2p_info, P2P_STATE_GONEGO_OK);
        if (p2p_info->go_negoing & ZT_BIT(P2P_INVIT_RESP))
        {
            //p2p_info->go_negoing  = 0;
            zt_p2p_nego_timer_set(pnic_info, P2P_CONN_NEGO_TIME);
        }
        else if (p2p_info->go_negoing & ZT_BIT(P2P_GO_NEGO_CONF))
        {
            zt_p2p_nego_timer_set(pnic_info, P2P_CONN_NEGO_TIME);
        }
        CFG80211_DBG("role=%s, p2p_state=%s, pre_p2p_state=%s\n",
                     zt_p2p_role_to_str(p2p_info->role),
                     zt_p2p_state_to_str(p2p_info->p2p_state),
                     zt_p2p_state_to_str(p2p_info->pre_p2p_state));

    }
    CFG80211_DBG("req ie_len:%d, resp ie_len:%d",
                 pcur_network->assoc_req.ie_len, pcur_network->assoc_resp.ie_len);
    cfg80211_connect_result(pnic_info->ndev,
                            pcur_network->mac_addr,
                            pcur_network->assoc_req.ie,
                            pcur_network->assoc_req.ie_len,
                            pcur_network->assoc_resp.ie,
                            pcur_network->assoc_resp.ie_len,
                            WLAN_STATUS_SUCCESS, GFP_ATOMIC);
}

void zt_cfg80211_indicate_disconnect(nic_info_st *pnic_info)
{
    struct wireless_dev *pwdev = (struct wireless_dev *)pnic_info->pwidev;
    struct net_device *ndev = pnic_info->ndev;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
    mlme_info_t *pmlme_info = pnic_info->mlme_info;
    zt_mlme_conn_res_t *pconn_res = &pmlme_info->conn_res;
#endif

    if (pwdev->iftype != NL80211_IFTYPE_STATION
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
            && pwdev->iftype != NL80211_IFTYPE_P2P_CLIENT
#endif
       )
    {
        return;
    }

#ifdef CFG_ENABLE_AP_MODE
    if (zt_local_cfg_get_work_mode(pnic_info) == ZT_MASTER_MODE)
    {
        return;
    }
#endif

    zt_os_api_disable_all_data_queue(ndev);
    if (zt_p2p_is_valid(pnic_info))
    {
        p2p_info_st *p2p_info = pnic_info->p2p;

        zt_p2p_set_state(p2p_info, p2p_info->pre_p2p_state);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
//        if (pwdev->iftype != NL80211_IFTYPE_P2P_CLIENT)
#endif
        {
            zt_p2p_set_role(p2p_info, P2P_ROLE_DEVICE);
        }
        zt_p2p_nego_timer_set(pnic_info, P2P_CONN_NEGO_TIME);
        CFG80211_DBG("role=%s, p2p_state=%s, pre_p2p_state=%s\n",
                     zt_p2p_role_to_str(p2p_info->role), zt_p2p_state_to_str(p2p_info->p2p_state),
                     zt_p2p_state_to_str(p2p_info->pre_p2p_state));

    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0) || defined(COMPAT_KERNEL_RELEASE)
    CFG80211_DBG("pwdev->sme_state(b)=%d\n", pwdev->sme_state);

    if (pwdev->sme_state == CFG80211_SME_CONNECTING)
    {
        cfg80211_connect_result(ndev,
                                NULL,
                                NULL, 0,
                                NULL, 0,
                                WLAN_STATUS_UNSPECIFIED_FAILURE,
                                GFP_ATOMIC);
    }
    else if (pwdev->sme_state == CFG80211_SME_CONNECTED)
    {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0))
        cfg80211_disconnected(ndev,
                              pconn_res->reason_code,
                              NULL, 0,
                              pconn_res->local_disconn,
                              GFP_ATOMIC);
#else
        cfg80211_disconnected(ndev, 0, NULL, 0, GFP_ATOMIC);
#endif
    }

    CFG80211_DBG("pwdev->sme_state(a)=%d", pwdev->sme_state);
#else
    {
        zt_bool bConnect;
        zt_mlme_get_connect(pnic_info, &bConnect);
        CFG80211_DBG("call cfg80211_disconnected, reason:%d, local_generite=%d",
                     pconn_res->reason_code, pconn_res->local_disconn);

        if (bConnect == zt_true)
        {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0))
            if (!pconn_res->local_disconn)
            {
                cfg80211_disconnected(ndev, pconn_res->reason_code, NULL, 0,
                                      zt_false, GFP_ATOMIC);
            }
            else
            {
                cfg80211_disconnected(ndev, ZT_80211_REASON_UNSPECIFIED, NULL, 0, zt_true,
                                      GFP_ATOMIC);
            }
#else
            cfg80211_disconnected(ndev, 0, NULL, 0, GFP_ATOMIC);
#endif
        }
        else
        {
            cfg80211_connect_result(ndev,
                                    NULL,
                                    NULL, 0,
                                    NULL, 0,
                                    WLAN_STATUS_UNSPECIFIED_FAILURE,
                                    GFP_ATOMIC);
        }
    }
#endif
}

/* The rescan only occurs after the p2p invitation
 * process and will not affect other processes.*/
#define MAX_RETRY_SCAN (5)

zt_s32 zt_cfg80211_scan_complete(nic_info_st *pnic_info)
{
    zt_wlan_mgmt_scan_que_node_t *pscan_que_node = NULL;
    zt_wlan_mgmt_scan_que_for_rst_e scan_que_for_rst;

    struct zt_widev_priv *pwdev_info = pnic_info->widev_priv;
    struct cfg80211_scan_request *req = pwdev_info->pscan_request;

    zt_u8 ch_num = 0;
    p2p_info_st *p2p_info = NULL;

    zt_s32 ret = 0;
    zt_wlan_ssid_t ssids[1];
    zt_s32 p2p_scan_ssid_found = 0;
    zt_s32 p2p_scan_ssid_search = 0;

    CFG80211_DBG("[%d] scan complete", pnic_info->ndev_id);

    if (zt_p2p_is_valid(pnic_info))
    {
        p2p_info  = pnic_info->p2p;

        ch_num = p2p_info->ext_channel_num + req->n_channels;

        if (req->ssids != NULL && 0 == zt_memcmp(req->ssids->ssid, "DIRECT-", 7) &&
            req->ssids[0].ssid_len > 7 && (p2p_info->go_negoing & ZT_BIT(P2P_INVIT_RESP)))
        {
            /* need to check p2p ssid found or not, only after invitation resp */
            p2p_scan_ssid_search = 1;
        }
    }
    else{
        ch_num = req->n_channels;
    }

    zt_wlan_mgmt_scan_que_for_begin(pnic_info, pscan_que_node)
    {
        CFG80211_DBG(" pscan_que_node have results");

        /* checkout channel */
        if (ch_num)
        {
            zt_u8 i;

            for (i = 0; i < req->n_channels; i++)
            {
                if (req->channels[i]->hw_value == pscan_que_node->channel)
                {
                    goto INFORM_BSS;
                }
            }

            if (zt_p2p_is_valid(pnic_info))
            {
                zt_u8 j;

                for (j = 0; j < p2p_info->ext_channel_num; j++)
                {
                    if (p2p_info->social_channel[j] == pscan_que_node->channel)
                    {
                        goto INFORM_BSS;
                    }
                }
            }

            continue;
        }

INFORM_BSS:
        /* check p2p ssid */
        if(p2p_scan_ssid_search && !p2p_scan_ssid_found)
        {
            if(0 == zt_memcmp(req->ssids[0].ssid, pscan_que_node->ssid.data, req->ssids[0].ssid_len))
            {
                CFG80211_INFO("[%d] scan_found ssid : %s", pnic_info->ndev_id, pscan_que_node->ssid.data);
                p2p_scan_ssid_found = 1;
                p2p_info->scan_times = 0;
            }
        }

        /* inform bss */
        inform_bss(pnic_info, pscan_que_node);
        CFG80211_DBG("%s"" "ZT_MAC_FMT" %s",
                     pscan_que_node->frame_type == ZT_80211_FRM_BEACON ? "BCN " : "PROB",
                     ZT_MAC_ARG(pscan_que_node->bssid),
                     pscan_que_node->ssid.data);
    }
    zt_wlan_mgmt_scan_que_for_end(scan_que_for_rst);

    if(p2p_scan_ssid_search && !p2p_scan_ssid_found){
        /* if p2p bssid not found, retry scan */
        if(p2p_info->scan_times < MAX_RETRY_SCAN)
        {
            zt_memset(ssids, 0, sizeof(ssids));
            zt_memcpy(ssids[0].data, req->ssids[0].ssid, req->ssids[0].ssid_len);
            ret = zt_mlme_scan_start(pnic_info, SCAN_TYPE_ACTIVE,
                                     ssids, 1,
                                     NULL, 0,
                                     ZT_MLME_FRAMEWORK_NETLINK);

            p2p_info->scan_times++;
            CFG80211_WARN("[%d] scan times : %d, ret : %d", pnic_info->ndev_id, p2p_info->scan_times, ret);
            if(!ret){
                return 0;
            }else{
                p2p_info->scan_times = 0;
            }
        }else{
            p2p_info->scan_times = 0;
        }
    }

    return 1;

}

zt_s32 zt_cfg80211_p2p_cb_reg(nic_info_st *pnic_info)
{
    p2p_info_st *p2p_info = pnic_info->p2p;
    if (zt_false == p2p_info->scb.init_flag)
    {
        p2p_info->scb.remain_on_channel_expired = zt_cfg80211_remain_on_channel_expired;
        p2p_info->scb.rx_mgmt           = zt_cfg80211_p2p_rx_mgmt;
        p2p_info->scb.ready_on_channel  = zt_cfg80211_p2p_ready_on_channel;
        p2p_info->scb.init_flag = zt_true;
    }

    return 0;
}

static zt_s32 cfg80211_p2p_nego_ctl_scan(nic_info_st *pnic_info,
        zt_u8 req_ch_nums,
        zt_u8 buddy_flag)
{
    p2p_info_st *p2p_info = NULL;
    if (NULL == pnic_info)
    {
        return 0;
    }

    p2p_info = pnic_info->p2p;
    if (zt_p2p_is_valid(pnic_info))
    {
        CFG80211_INFO("[%d] buddy:%d, nego:0x%x,scan_deny:%d",
                      pnic_info->ndev_id, buddy_flag, p2p_info->go_negoing, p2p_info->scan_deny);
        if (buddy_flag)
        {
            if (p2p_info->scan_deny)
            {
                return 1;
            }
            if (p2p_info->go_negoing)
            {
                if (0 == p2p_info->nego_timer_flag)
                {
                    p2p_info->nego_timer_flag = 1;

                }
                return 1;//scan_done = true
            }

        }

    }

    return 0;
}
static zt_s32 _call_scan_cb(struct wiphy *wiphy
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
                            , struct net_device *ndev
#endif
                            , struct cfg80211_scan_request *req)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
    struct net_device *ndev;
#endif
    zt_s32 i, ret = 0;
    ndev_priv_st *pndev_priv;
    nic_info_st *pnic_info;
    mlme_info_t *pmlme_info;
    zt_scan_info_t *pscan_info;
    struct zt_widev_priv *pwdev_info;
    mlme_state_e state;
    zt_bool bConnect, bBusy;
    zt_bool scan_done = zt_false;
    zt_wlan_ssid_t ssids[ZT_SCAN_REQ_SSID_NUM];
    zt_u8 ssid_num = 0;
    zt_u8 current_ch[ZT_SCAN_REQ_CHANNEL_NUM];
    zt_u8 scan_time_for_one_ch = 6;
    zt_u8 scan_time = 3;
    zt_u8 social_channel = 0, ext_channel = 0;
    zt_u8 req_ch_nums    = req->n_channels;

    CFG80211_DBG("scan start!");

    if (req == NULL)
    {
        ret = -EINVAL;
        return ret;
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
    if (ndev == NULL)
    {
        ret = -EINVAL;
        return ret;
    }
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
    ndev = req->wdev->netdev;
#endif

    pndev_priv = netdev_priv(ndev);
    pnic_info = pndev_priv->nic;

    CFG80211_INFO("[%d] mac addr: "ZT_MAC_FMT, pnic_info->ndev_id,
                  ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    pmlme_info = (mlme_info_t *)pnic_info->mlme_info;
    pscan_info = pnic_info->scan_info;
    pwdev_info = pnic_info->widev_priv;
    zt_os_api_lock_lock(&pwdev_info->scan_req_lock);
    pwdev_info->pscan_request = req;
    zt_os_api_lock_unlock(&pwdev_info->scan_req_lock);

    if (pnic_info->buddy_nic)
    {
        mlme_state_e state;
        zt_mlme_get_state((nic_info_st *)(pnic_info->buddy_nic), &state);
        if (state == MLME_STATE_CONN_SCAN ||
                state == MLME_STATE_AUTH ||
                state == MLME_STATE_ASSOC ||
                state == MLME_STATE_SCAN)
        {
            CFG80211_DBG("buddy interface is under linking or scaning!");
            scan_done = zt_true;
            goto exit;
        }
    }

    LOG_I("this is zdg:: req->ssid:: %s\n", req->ssids);

    if (req->ssids != NULL && 0 == zt_memcmp(req->ssids->ssid, "DIRECT-", 7)
            && zt_p2p_get_ie((zt_u8 *) req->ie, req->ie_len, NULL, NULL))
    {
        if (req->ssids->ssid_len == 7)
        {
            CFG80211_INFO("abort p2p listen!");
            scan_done = zt_true;
            goto exit;
        }

        p2p_info_st *p2p_info = pnic_info->p2p;
        scan_time_for_one_ch  = 1;
        scan_time             = 1;

        CFG80211_DBG(" p2p ie_len:%zu", req->ie_len);
        for (i = 0; i < req_ch_nums; i++)
        {
            CFG80211_INFO("[%d] channel[%d]: hw_value[%d]", pnic_info->ndev_id, i,
                          req->channels[i]->hw_value);
        }

        if (req->n_channels == 3 && req->channels[0]->hw_value == 1
                && req->channels[1]->hw_value == 6 && req->channels[2]->hw_value == 11)
        {
            social_channel = 1;
        }

        p2p_info->ext_channel_num = 0;

        if (req->n_channels == 1)
        {
            ext_channel = 1;
        }

        if (zt_p2p_is_valid(pnic_info))
        {
            if (cfg80211_p2p_nego_ctl_scan(pnic_info, req_ch_nums, zt_false))
            {
                scan_done = zt_true;
                goto exit;
            }
            zt_p2p_set_pre_state(p2p_info, p2p_info->p2p_state);

        }
        else
        {
            /*register callback function*/
            zt_cfg80211_p2p_cb_reg(pnic_info);
            zt_p2p_enable(pnic_info, P2P_ROLE_DEVICE);
        }

        zt_p2p_scan_entry(pnic_info, social_channel, (zt_u8 *)req->ie, req->ie_len);

    }
    else if (cfg80211_p2p_nego_ctl_scan(pnic_info->buddy_nic, req_ch_nums, zt_true))
    {
        scan_done = zt_true;
        goto exit;
    }

    /* if traffic busy been detected, the current scan request should ignore,
    with the purpose of no interference traffic, unless timeout occurs from
    the start of traffic busy is detected. */
    zt_mlme_get_connect(pndev_priv->nic, &bConnect);
    if (bConnect == zt_true)
    {
        static zt_bool on_check = zt_false;
        static zt_timer_t timer;

        zt_mlme_get_traffic_busy(pndev_priv->nic, &bBusy);
        if (bBusy == zt_true)
        {
            if (!on_check)
            {
                on_check = zt_true;
                zt_timer_set(&timer, 12 * 1000);
            }
            if (zt_timer_expired(&timer))
            {
                zt_timer_restart(&timer);
            }
            //            else
            {
                scan_done = zt_true;
                goto exit;
            }
        }
        else if (on_check)
        {
            on_check = zt_false;
        }
    }
    if (NULL != pndev_priv->nic->buddy_nic)
    {
        zt_mlme_get_connect(pndev_priv->nic->buddy_nic, &bConnect);
        if (bConnect == zt_true)
        {
            zt_mlme_get_traffic_busy(pndev_priv->nic->buddy_nic, &bBusy);
            if (bBusy == zt_true)
            {
                scan_done = zt_true;
                goto exit;
            }
        }
    }

    zt_mlme_get_state(pnic_info, &state);
    if (state <= MLME_STATE_ASSOC)
    {
        scan_done = zt_true;
        goto exit;
    }

    CFG80211_DBG("req->n_ssids:%d", req->n_ssids);
    zt_memset(ssids, 0, sizeof(ssids));
    if (req->ssids)
    {
        for (i = 0; i < req->n_ssids && i < ZT_ARRAY_SIZE(ssids); i++)
        {
            if (req->ssids[i].ssid_len)
            {
                CFG80211_DBG("ssid = %s, ssid_len = %d", req->ssids[i].ssid, req->ssids[i].ssid_len);
                zt_memcpy(ssids[i].data, req->ssids[i].ssid, req->ssids[i].ssid_len);
                ssids[i].length = req->ssids[i].ssid_len;
            }
            else
            {
                break;
            }
        }
        ssid_num = i;
    }

    CFG80211_DBG("n_channels:%d", req->n_channels);
    zt_memset(current_ch, 0, sizeof(current_ch));
    if (req->channels[0])
    {
        for (i = 0; i < req->n_channels && i < ZT_ARRAY_SIZE(current_ch); i++)
        {
            CFG80211_DBG("hw_value:%u flags:0x%08x",
                         req->channels[i]->hw_value, req->channels[i]->flags);
            current_ch[i] = req->channels[i]->hw_value;
        }

        if (zt_p2p_is_valid(pnic_info))
        {
            zt_u8 j = 0;

            p2p_info_st *p2p_info = pnic_info->p2p;
            if (ext_channel)
            {
                /* after invitation response, gc will scan in all channel */
                for(j = 0; j < 13; j++)
                {
                    p2p_info->social_channel[j] = j + 1;

                }
                p2p_info->ext_channel_num = 13;
            }
        }
    }

    if (req->n_channels == 1)
    {
        if (ext_channel)
        {
            ret = zt_mlme_scan_start(pnic_info, SCAN_TYPE_ACTIVE,
                                     ssids, ssid_num,
                                     NULL, 0,
                                     ZT_MLME_FRAMEWORK_NETLINK);
        }
        else
        {
            for (i = 1; i < scan_time_for_one_ch; i++)
            {
                zt_memcpy(&current_ch[i], &current_ch[0], sizeof(current_ch[0]));
            }
            ret = zt_mlme_scan_start(pnic_info, SCAN_TYPE_ACTIVE,
                                     ssids, ssid_num,
                                     current_ch, scan_time_for_one_ch,
                                     ZT_MLME_FRAMEWORK_NETLINK);
        }
    }
    else if (req->n_channels <= 4)
    {
        zt_s8 j;
        for (j = req->n_channels - 1; j >= 0; j--)
        {
            for (i = 0; i < scan_time; i++)
            {
                zt_memcpy(&current_ch[j * scan_time + i], &current_ch[j],
                          sizeof(current_ch[0]));
            }
        }
        ret = zt_mlme_scan_start(pnic_info, SCAN_TYPE_ACTIVE,
                                 ssids, ssid_num,
                                 current_ch, scan_time * req->n_channels,
                                 ZT_MLME_FRAMEWORK_NETLINK);
    }
    else
    {
        ret = zt_mlme_scan_start(pnic_info, SCAN_TYPE_ACTIVE,
                                 ssids, ssid_num,
                                 current_ch, req->n_channels,
                                 ZT_MLME_FRAMEWORK_NETLINK);
    }

    if (ret)
        scan_done = zt_true;

exit:
    if (scan_done == zt_true)
    {
        if(zt_cfg80211_scan_complete(pnic_info)){
            zt_cfg80211_scan_done_event_up(pnic_info, zt_true);
        }
    }

    return ret;
}

static zt_s32 _set_wiphy_params(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 17, 0))
		zt_s32 radio_idx,
#endif
		zt_u32 changed)
{
    CFG80211_DBG();

    return 0;
}

static zt_s32 cfg80211_set_auth_type(sec_info_st *psec_info,
                                     enum nl80211_auth_type sme_auth_type)
{
    CFG80211_DBG("nl80211 auth type=%d", sme_auth_type);

    switch (sme_auth_type)
    {
        case NL80211_AUTHTYPE_AUTOMATIC:
            psec_info->dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
            break;

        case NL80211_AUTHTYPE_OPEN_SYSTEM:
            psec_info->dot11AuthAlgrthm = dot11AuthAlgrthm_Open;
            if (psec_info->ndisauthtype > zt_ndis802_11AuthModeWPA)
            {
                psec_info->dot11AuthAlgrthm = dot11AuthAlgrthm_8021X;
            }
            break;
        case NL80211_AUTHTYPE_SHARED_KEY:
            psec_info->dot11AuthAlgrthm = dot11AuthAlgrthm_Shared;
            psec_info->ndisencryptstatus = zt_ndis802_11Encryption1Enabled;
            break;
        default:
            psec_info->dot11AuthAlgrthm = dot11AuthAlgrthm_Open;
    }

    return 0;
}

static zt_s32 cfg80211_set_cipher(sec_info_st *psec_info,
                                  zt_u32 nl_cipher, bool ucast)
{
    zt_u32 sec_status, cipher;

    switch (nl_cipher)
    {
        case 0:
        case IW_AUTH_CIPHER_NONE:
            cipher = _NO_PRIVACY_;
            sec_status = zt_ndis802_11EncryptionDisabled;
            break;
        case WLAN_CIPHER_SUITE_WEP40:
            cipher = _WEP40_;
            sec_status = zt_ndis802_11Encryption1Enabled;
            break;
        case WLAN_CIPHER_SUITE_WEP104:
            cipher = _WEP104_;
            sec_status = zt_ndis802_11Encryption1Enabled;
            break;
        case WLAN_CIPHER_SUITE_TKIP:
            cipher = _TKIP_;
            sec_status = zt_ndis802_11Encryption2Enabled;
            break;
        case WLAN_CIPHER_SUITE_CCMP:
            cipher = _AES_;
            sec_status = zt_ndis802_11Encryption3Enabled;
            break;
        default:
            CFG80211_DBG("Unsupported cipher: 0x%x, ucast: %d", nl_cipher, ucast);
            return -ENOTSUPP;
    }

    if (ucast)
    {
        psec_info->dot11PrivacyAlgrthm = cipher;
        psec_info->ndisencryptstatus = sec_status;
    }
    else
    {
        psec_info->dot118021XGrpPrivacy = cipher;
    }

    return 0;
}

static zt_s32 cfg80211_set_wep_key(nic_info_st *pnic_info,
                                   struct cfg80211_connect_params *sme)
{
    sec_info_st *psec_info = pnic_info->sec_info;
    zt_u32 wep_key_idx, wep_key_len;
    zt_u32 res = 0;
    wep_key_idx = sme->key_idx;
    wep_key_len = sme->key_len;

    CFG80211_DBG("wep_key_idx = %d, wep_key_len = %d, ", wep_key_idx, wep_key_len);
    CFG80211_ARRAY((zt_u8 *)sme->key, wep_key_len);

    if (sme->key_idx > ZT_80211_WEP_KEYS)
    {
        res = -EINVAL;
        goto exit;
    }

    if (wep_key_len > 0)
    {
        wep_key_len = wep_key_len <= 5 ? 5 : 13;

        if (wep_key_len == 13)
        {
            psec_info->dot11PrivacyAlgrthm = _WEP104_;
        }
        else
        {
            psec_info->dot11PrivacyAlgrthm = _WEP40_;
        }
    }
    else
    {
        res = -EINVAL;
        goto exit;
    }

    zt_memcpy(psec_info->dot11DefKey[wep_key_idx].skey, sme->key, wep_key_len);
    psec_info->dot11DefKeylen[wep_key_idx] = wep_key_len;
    psec_info->key_mask |= ZT_BIT(wep_key_idx);

exit:
    return res;
}

static zt_s32 cfg80211_set_wpa_ie(nic_info_st *pnic_info, zt_u8 *pie,
                                  size_t ielen)
{
    sec_info_st *sec_info = pnic_info->sec_info;
    zt_u8 *buf = NULL;
    zt_s32 group_cipher = 0, pairwise_cipher = 0;
    zt_u16 cnt = 0;
    zt_u8 eid, wps_oui[4] = { 0x0, 0x50, 0xf2, 0x04 };
    zt_s32 res = 0;

    CFG80211_DBG();

    if (pie == NULL)
    {
        goto exit;
    }

    if (ielen > ZT_MAX_WPA_IE_LEN)
    {
        res = -EINVAL;
        goto exit;
    }

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        res = -EINVAL;
        goto exit;
    }

    if (ielen)
    {
        buf = zt_kzalloc(ielen);
        if (buf == NULL)
        {
            res = -ENOMEM;
            goto exit;
        }
        zt_memcpy(buf, pie, ielen);

        if (ielen < ZT_RSN_HD_LEN)
        {
            CFG80211_WARN("Ie len too short(%d)", (zt_u16)ielen);
            res = -EINVAL;
            goto exit;
        }

        {
            void *pdata;
            zt_u16 data_len;

            if (!zt_80211_mgmt_wpa_survey(buf, ielen, &pdata, &data_len,
                                          &group_cipher, &pairwise_cipher))
            {
                sec_info->dot11AuthAlgrthm = dot11AuthAlgrthm_8021X;
                sec_info->ndisauthtype = zt_ndis802_11AuthModeWPAPSK;
                sec_info->wpa_enable = zt_true;
                zt_memcpy(sec_info->supplicant_ie, pdata, data_len);
            }
            else if (!zt_80211_mgmt_rsn_survey(buf, ielen, &pdata, &data_len,
                                               &group_cipher, &pairwise_cipher))
            {
                sec_info->dot11AuthAlgrthm = dot11AuthAlgrthm_8021X;
                sec_info->ndisauthtype = zt_ndis802_11AuthModeWPA2PSK;
                sec_info->rsn_enable = zt_true;
                zt_memcpy(sec_info->supplicant_ie, pdata, data_len);
            }
        }

        switch (group_cipher)
        {
            case ZT_CIPHER_SUITE_TKIP:
                sec_info->dot118021XGrpPrivacy = _TKIP_;
                sec_info->ndisencryptstatus = zt_ndis802_11Encryption2Enabled;
                CFG80211_DBG("dot118021XGrpPrivacy=_TKIP_");
                break;
            case ZT_CIPHER_SUITE_CCMP:
                sec_info->dot118021XGrpPrivacy = _AES_;
                sec_info->ndisencryptstatus = zt_ndis802_11Encryption3Enabled;
                CFG80211_DBG("dot118021XGrpPrivacy=_AES_");
                break;
        }

        switch (pairwise_cipher)
        {
            case ZT_CIPHER_SUITE_NONE:
                break;
            case ZT_CIPHER_SUITE_TKIP:
                sec_info->dot11PrivacyAlgrthm = _TKIP_;
                sec_info->ndisencryptstatus = zt_ndis802_11Encryption2Enabled;
                CFG80211_DBG("dot11PrivacyAlgrthm=_TKIP_");
                break;
            case ZT_CIPHER_SUITE_CCMP:
                sec_info->dot11PrivacyAlgrthm = _AES_;
                sec_info->ndisencryptstatus = zt_ndis802_11Encryption3Enabled;
                CFG80211_DBG("dot11PrivacyAlgrthm=_AES_");
                break;
        }

        while (cnt < ielen)
        {
            eid = buf[cnt];
            if (eid == ZT_80211_MGMT_EID_VENDOR_SPECIFIC &&
                    !zt_memcmp(&buf[cnt + 2], wps_oui, 4))
            {
                CFG80211_DBG("SET WPS_IE");
                sec_info->wps_ie_len = ZT_MIN(buf[cnt + 1] + 2, 512);
                zt_memcpy(sec_info->wps_ie, &buf[cnt], sec_info->wps_ie_len);
                cnt += buf[cnt + 1] + 2;
                break;
            }
            else
            {
                cnt += buf[cnt + 1] + 2;
            }
        }

        zt_mcu_set_on_rcr_am(pnic_info, zt_false);
        //        zt_mcu_set_hw_invalid_all(pnic_info);
    }

exit :
    if (buf)
    {
        zt_kfree(buf);
    }
    return res;
}

static zt_s32 _connect_cb(struct wiphy *wiphy, struct net_device *ndev,
                          struct cfg80211_connect_params *sme)
{
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;
    sec_info_st *psec_info = pnic_info->sec_info;
    zt_wlan_mgmt_info_t *pwlan_info = pnic_info->wlan_mgmt_info;
    zt_wlan_network_t *pcur_network = &pwlan_info->cur_network;
    zt_u32 wpa_version, key_mgmt;
    zt_s32 res = 0;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        return -EINVAL;
    }

    if (!sme->ssid || sme->ssid_len == 0)
    {
        res = -EINVAL;
        goto exit;
    }

    if (sme->ssid_len > IW_ESSID_MAX_SIZE)
    {
        res = -E2BIG;
        goto exit;
    }

#ifdef CONFIG_LPS
    if (ZT_RETURN_FAIL == zt_lps_wakeup(pnic_info, LPS_CTRL_SCAN, 0))
    {
        return -1;
    }
#endif
    CFG80211_DBG("privacy=%d, key=%p, key_len=%d, key_idx=%d, auth_type=%d, wpa:%d",
                 sme->privacy, sme->key, sme->key_len, sme->key_idx, sme->auth_type,
                 sme->crypto.wpa_versions);

#ifdef CFG_ENABLE_AP_MODE
    /* connection request no work best on master mode  */
    if (zt_local_cfg_get_work_mode(pnic_info) == ZT_MASTER_MODE)
    {
        res = -EPERM;
        goto exit;
    }
#endif

    if (zt_p2p_is_valid(pnic_info))
    {
        zt_scan_stop(pnic_info->buddy_nic);
        zt_assoc_stop(pnic_info->buddy_nic);
    }

    /* if buddy interfase is under linking, ignore current request */
    if (pnic_info->buddy_nic)
    {
        mlme_state_e state;
        zt_mlme_get_state((nic_info_st *)(pnic_info->buddy_nic), &state);
        if (state == MLME_STATE_CONN_SCAN ||
                state == MLME_STATE_AUTH ||
                state == MLME_STATE_ASSOC)
        {
            CFG80211_WARN("buddy interface is under linking !");
            res = -EPERM;
            goto exit;
        }
    }

    /* if connection is on building, ignore current request */
    {
        mlme_state_e state;
        zt_mlme_get_state(pnic_info, &state);
        if (state <= MLME_STATE_ASSOC && state > MLME_STATE_SCAN)
        {
            res = 0;
            goto exit;
        }
    }

    /* checkout if system scanning, abort scan process with timeout */
    if (zt_scan_wait_done(pnic_info, zt_true, 200))
    {
        res = -EBUSY;
        goto exit;
    }

    zt_memset(pcur_network->ssid.data, 0, ZT_80211_MAX_SSID_LEN + 1);
    zt_memcpy(pcur_network->ssid.data, sme->ssid, sme->ssid_len);
    pcur_network->ssid.length = sme->ssid_len;
    CFG80211_DBG("ssid = %s, len = %d",
                 pcur_network->ssid.data, pcur_network->ssid.length);

    if (!sme->bssid || zt_80211_is_bcast_addr(sme->bssid) ||
            zt_80211_is_zero_addr(sme->bssid) || zt_80211_is_mcast_addr(sme->bssid))
    {
        CFG80211_DBG("[WLAN_IW] : [sa_data is boradcast or zero ether]");
        res = -EPERM;
        goto exit;
    }
    zt_wlan_set_cur_bssid(pnic_info, (zt_u8 *)sme->bssid);

    /* cleap up sec info */
    zt_memset(pnic_info->sec_info, 0x0, sizeof(sec_info_st));

    psec_info->ndisencryptstatus    = zt_ndis802_11EncryptionDisabled;
    psec_info->dot11PrivacyAlgrthm  = _NO_PRIVACY_;
    psec_info->dot118021XGrpPrivacy = _NO_PRIVACY_;
    psec_info->dot11AuthAlgrthm     = dot11AuthAlgrthm_Open;
    psec_info->ndisauthtype         = zt_ndis802_11AuthModeOpen;
    psec_info->busetkipkey          = zt_false;

    /* parse auth mode */
    wpa_version = sme->crypto.wpa_versions;
    if (!wpa_version)
    {
        psec_info->ndisauthtype = zt_ndis802_11AuthModeOpen;
    }
    else if (wpa_version & (NL80211_WPA_VERSION_1 | NL80211_WPA_VERSION_2))
    {
        psec_info->ndisauthtype = zt_ndis802_11AuthModeWPAPSK;
    }

    cfg80211_set_auth_type(psec_info, sme->auth_type);

    /* parse ie */
    res = cfg80211_set_wpa_ie(pnic_info, (zt_u8 *)sme->ie, sme->ie_len);
    if (res < 0)
    {
        goto exit;
    }

    /* parse crypto for wep */
    if (sme->key_len > 0 && sme->key)
    {
        res = cfg80211_set_wep_key(pnic_info, sme);
        if (res < 0)
        {
            res = -EOPNOTSUPP;
            goto exit;
        }
    }

    /* parse crypto for wpa */
    if (sme->crypto.n_ciphers_pairwise)
    {
        res = cfg80211_set_cipher(psec_info, sme->crypto.ciphers_pairwise[0], zt_true);
        if (res < 0)
        {
            goto exit;
        }
    }

    res = cfg80211_set_cipher(psec_info, sme->crypto.cipher_group, zt_false);
    if (res < 0)
    {
        goto exit;
    }

    if (sme->crypto.n_akm_suites)
    {
        key_mgmt = sme->crypto.akm_suites[0];
        if (key_mgmt == WLAN_AKM_SUITE_8021X || key_mgmt == WLAN_AKM_SUITE_PSK)
        {
            psec_info->dot11AuthAlgrthm = dot11AuthAlgrthm_8021X;
        }
        else
        {
            CFG80211_DBG("Invalid key mgmt: 0x%x", key_mgmt);
        }
    }

    if (zt_p2p_is_valid(pnic_info))
    {
        zt_p2p_connect_entry(pnic_info, (zt_u8 *)sme->ie, sme->ie_len);
		zt_memset(pcur_network->ssid.data, 0, ZT_80211_MAX_SSID_LEN + 1);
		zt_memcpy(pcur_network->ssid.data, "DIRECT-", 7);
		pcur_network->ssid.length = 7;
    }

    {
        zt_s32 ret = zt_mlme_conn_start(pnic_info,
                                        zt_wlan_get_cur_bssid(pnic_info),
                                        zt_wlan_get_cur_ssid(pnic_info),
                                        ZT_MLME_FRAMEWORK_NETLINK,
                                        zt_true);
        if (ret < 0)
        {
            CFG80211_WARN("connect start fail, fail code: %d", ret);
        }
    }

exit:
    return res;
}


static zt_s32 _disconnect_cb(struct wiphy *wiphy, struct net_device *ndev,
                             zt_u16 reason_code)
{
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;

    CFG80211_INFO("mac addr: "ZT_MAC_FMT" reason_code=%d",
                  ZT_MAC_ARG(nic_to_local_addr(pnic_info)), reason_code);

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        return -EINVAL;
    }

    zt_mlme_deauth(pnic_info, zt_true, reason_code);
#ifdef CONFIG_LPS
    if (ZT_RETURN_FAIL == zt_lps_wakeup(pnic_info, LPS_CTRL_DISCONNECT, 0))
    {
        return -1;
    }
#endif

    return 0;
}

static zt_s32 _join_ibss_cb(struct wiphy *wiphy, struct net_device *ndev,
                            struct cfg80211_ibss_params *params)
{
    zt_s32 res = 0;
#ifdef CFG_ENABLE_ADHOC_MODE
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;
    sec_info_st *psec_info = pnic_info->sec_info;
    zt_wlan_mgmt_info_t *wlan_mgmt_info = pnic_info->wlan_mgmt_info;
    zt_wlan_network_t *pcur_network = &wlan_mgmt_info->cur_network;
    zt_bool bConnected = zt_false;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
    struct cfg80211_chan_def *pch_def;
#endif
    struct ieee80211_channel *pch;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
    pch_def = (struct cfg80211_chan_def *)(&params->chandef);
    pch = (struct ieee80211_channel *)pch_def->chan;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31))
    pch = (struct ieee80211_channel *)(params->channel);
#endif
    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        return -EINVAL;
    }

#ifdef CONFIG_LPS
    if (ZT_RETURN_FAIL == zt_lps_wakeup(pnic_info, LPS_CTRL_SCAN, 0))
    {
        return -1;
    }
#endif

    if (zt_get_adhoc_master(pnic_info) == zt_true)
    {
        res = 0;
        goto exit;
    }

    if (zt_local_cfg_get_work_mode(pnic_info) != ZT_ADHOC_MODE)
    {
        res = -EPERM;
        goto exit;
    }

    if (!params->ssid || !params->ssid_len)
    {
        res = -EINVAL;
        goto exit;
    }

    if (params->ssid_len > IW_ESSID_MAX_SIZE)
    {
        res = -E2BIG;
        goto exit;
    }

    pcur_network->bcn_interval = params->beacon_interval;
    pcur_network->cap_info = 0;
    pcur_network->cap_info |= ZT_80211_MGMT_CAPAB_IBSS;
    pcur_network->short_slot = NON_SHORT_SLOT_TIME;

    zt_memcpy(pcur_network->mac_addr, nic_to_local_addr(pnic_info), ETH_ALEN);

    psec_info->ndisencryptstatus = zt_ndis802_11EncryptionDisabled;
    psec_info->dot11PrivacyAlgrthm = _NO_PRIVACY_;
    psec_info->dot118021XGrpPrivacy = _NO_PRIVACY_;
    psec_info->dot11AuthAlgrthm = dot11AuthAlgrthm_Open;
    psec_info->ndisauthtype = zt_ndis802_11AuthModeOpen;

    res  = cfg80211_set_auth_type(psec_info, NL80211_AUTHTYPE_OPEN_SYSTEM);
    pcur_network->channel = freq_2_ch(pch->center_freq);
    zt_wlan_set_cur_bw(pnic_info, CHANNEL_WIDTH_20);

    zt_mlme_get_connect(pnic_info, &bConnected);
    if (bConnected)
    {
        res = 0;
        goto exit;
    }
    else
    {
        zt_wlan_ssid_t ssid ;
        zt_memset(&ssid, 0, sizeof(zt_wlan_ssid_t));
        ssid.length = params->ssid_len;
        zt_memcpy(ssid.data, (zt_u8 *)params->ssid, ssid.length);
        CFG80211_INFO("start connect to: %s", ssid.data);

        zt_wlan_set_cur_ssid(pnic_info, &ssid);
        zt_scan_wait_done(pnic_info, zt_true, 1000);

        zt_mlme_scan_ibss_start(pnic_info,
                                &pcur_network->ssid,
                                &pcur_network->channel,
                                ZT_MLME_FRAMEWORK_NETLINK);
    }

exit:
#endif
    return res;
}


static zt_s32 _leave_ibss_cb(struct wiphy *wiphy, struct net_device *ndev)
{
#ifdef CFG_ENABLE_ADHOC_MODE
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;
    struct wireless_dev *pwdev = pnic_info->pwidev;
    enum nl80211_iftype old_iftype;
    zt_bool bConnected;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));
    old_iftype = pwdev->iftype;
    zt_mlme_get_connect(pnic_info, &bConnected);

    if (bConnected && old_iftype == NL80211_IFTYPE_ADHOC)
    {
        zt_adhoc_leave_ibss_msg_send(pnic_info);
        zt_yield();
        pwdev->iftype = NL80211_IFTYPE_STATION;
        /* free message queue in wdn_info */
        CFG80211_DBG("free resource");
        zt_adhoc_flush_all_resource(pnic_info, ZT_INFRA_MODE);

    }
#endif
    return 0;
}

static zt_s32 _call_set_txpower(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
                                struct wireless_dev *wdev,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 17, 0))
				zt_s32 radio_idx,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)) || defined(COMPAT_KERNEL_RELEASE)
                                enum nl80211_tx_power_setting type, zt_s32 mbm)
#else
                                enum tx_power_setting type, zt_s32 dbm)
#endif
{
    CFG80211_DBG();

    return 0;
}


static zt_s32 _call_get_txpower(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
                                struct wireless_dev *wdev,
#endif
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(6, 17, 0))
                                int radio_idx,
#endif
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0))
                                unsigned int link_id,
#endif
                                zt_s32 *dbm)
{
    CFG80211_DBG();

    *dbm = (12);

    return 0;
}


static zt_s32 _cfg80211_set_power_mgmt(struct wiphy *wiphy,
                                       struct net_device *ndev, bool enabled, zt_s32 timeout)
{
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;
    struct  zt_widev_priv *pwdev_info = pnic_info->widev_priv;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));
    CFG80211_DBG("power management %s", enabled ? "enabled" : "disabled");

    pwdev_info->power_mgmt = enabled;

    return 0;
}


static zt_s32 _set_pmksa_cb(struct wiphy *wiphy,
                            struct net_device *ndev,
                            struct cfg80211_pmksa *pmksa)
{
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;
    sec_info_st *psec_info = pnic_info->sec_info;
    zt_u8 index, blInserted = zt_false;
    zt_u8 strZeroMacAddress[ETH_ALEN] = { 0x00 };
    zt_bool bConnect;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    if (zt_memcmp((zt_u8 *) pmksa->bssid, strZeroMacAddress, ETH_ALEN) == zt_true)
    {
        return -EINVAL;
    }

    zt_mlme_get_connect(pnic_info, &bConnect);

    if (bConnect == zt_false)
    {
        CFG80211_DBG(" not set pmksa cause not in linked state");
        return -EINVAL;
    }


    blInserted = zt_false;

    for (index = 0; index < NUM_PMKID_CACHE; index++)
    {
        if (zt_memcmp(psec_info->PMKIDList[index].Bssid, (zt_u8 *) pmksa->bssid,
                      ETH_ALEN) == zt_true)
        {
            CFG80211_DBG(" BSSID exists in the PMKList.");

            zt_memcpy(psec_info->PMKIDList[index].PMKID,
                      (zt_u8 *) pmksa->pmkid, WLAN_PMKID_LEN);
            psec_info->PMKIDList[index].bUsed = zt_true;
            psec_info->PMKIDIndex = index + 1;
            blInserted = zt_true;
            break;
        }
    }

    if (!blInserted)
    {
        CFG80211_DBG(" Use the new entry index = %d for this PMKID.",
                     psec_info->PMKIDIndex);

        zt_memcpy(psec_info->PMKIDList[psec_info->PMKIDIndex].Bssid,
                  (zt_u8 *) pmksa->bssid, ETH_ALEN);
        zt_memcpy(psec_info->PMKIDList[psec_info->PMKIDIndex].PMKID,
                  (zt_u8 *) pmksa->pmkid, WLAN_PMKID_LEN);

        psec_info->PMKIDList[psec_info->PMKIDIndex].bUsed = zt_true;
        psec_info->PMKIDIndex++;
        if (psec_info->PMKIDIndex == 16)
        {
            psec_info->PMKIDIndex = 0;
        }
    }

    return 0;
}


static zt_s32 _del_pmksa_cb(struct wiphy *wiphy,
                            struct net_device *ndev,
                            struct cfg80211_pmksa *pmksa)
{
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;
    sec_info_st *psec_info = pnic_info->sec_info;
    zt_u8 index, bMatched = zt_false;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    for (index = 0; index < NUM_PMKID_CACHE; index++)
    {
        if (zt_memcmp(psec_info->PMKIDList[index].Bssid, (zt_u8 *) pmksa->bssid,
                      ETH_ALEN) == zt_true)
        {
            zt_memset(psec_info->PMKIDList[index].Bssid, 0x00, ETH_ALEN);
            zt_memset(psec_info->PMKIDList[index].PMKID, 0x00, WLAN_PMKID_LEN);
            psec_info->PMKIDList[index].bUsed = zt_false;
            bMatched = zt_true;
            CFG80211_DBG(" clear id:%hhu", index);
            break;
        }
    }

    if (zt_false == bMatched)
    {
        return -EINVAL;
    }

    return 0;
}


static zt_s32 _flush_pmksa_cb(struct wiphy *wiphy,
                              struct net_device *ndev)
{
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;
    sec_info_st *psec_info = pnic_info->sec_info;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    zt_memset(&psec_info->PMKIDList[0], 0x00,
              sizeof(SEC_PMKID_LIST) * NUM_PMKID_CACHE);
    psec_info->PMKIDIndex = 0;

    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
static zt_s32 _set_monitor_channel(struct wiphy *wiphy
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 13, 0))
                                   , struct net_device *dev
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
                                   , struct cfg80211_chan_def *chandef
#else
                                   , struct ieee80211_channel *chan, enum nl80211_channel_type channel_type
#endif
                                  )
{

    return 0;
}
#endif


static zt_s32 _cfg80211_Mgmt_Tx(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
                                struct wireless_dev *wdev,
#else
                                struct net_device *ndev,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0)) || defined(COMPAT_KERNEL_RELEASE)
                                struct ieee80211_channel *chan,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)) || defined(COMPAT_KERNEL_RELEASE)
                                bool offchan,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
                                enum nl80211_channel_type channel_type,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
                                bool channel_type_valid,
#endif
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)) || defined(COMPAT_KERNEL_RELEASE)
                                zt_u32 wait,
#endif
                                const zt_u8 *buf, size_t len,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
                                bool no_cck,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
                                bool dont_wait_for_ack,
#endif
#else
                                struct cfg80211_mgmt_tx_params *params,
#endif
                                zt_u64 *cookie)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)) || defined(COMPAT_KERNEL_RELEASE)
    struct ieee80211_channel *chan = params->chan;
    const zt_u8 *buf = params->buf;
    size_t len = params->len;
#endif
    zt_s32 wait_ack = 0;
    bool ack = zt_true;
    zt_u8 tx_ch;
    zt_u8 frame_styp;
    ndev_priv_st *pndev_priv;
    nic_info_st *pnic_info;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
    if (wdev->netdev)
    {
        pndev_priv = netdev_priv(wdev->netdev);
        pnic_info = pndev_priv->nic;
    }
    else
    {
        return -1;
    }
#else
    struct wireless_dev *wdev;
    if (ndev == NULL)
    {
        return -1;
    }
    pndev_priv = netdev_priv(ndev);
    pnic_info = pndev_priv->nic;
    wdev = ndev_to_wdev(ndev);
#endif

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        return -EINVAL;
    }

    if (chan == NULL)
    {
        return -1;
    }
    tx_ch = (zt_u8)ieee80211_frequency_to_channel(chan->center_freq);

    /* cookie generation */
    *cookie = (zt_ptr) buf;

    /* indicate ack before issue frame to avoid racing with rsp frame */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
    cfg80211_mgmt_tx_status(ndev, *cookie, buf, len, ack, GFP_KERNEL);
#else
    cfg80211_mgmt_tx_status(wdev, *cookie, buf, len, ack, GFP_KERNEL);
#endif
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34) && LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 36))
    cfg80211_action_tx_status(ndev, *cookie, buf, len, ack, GFP_KERNEL);
#endif

    frame_styp = zt_le16_to_cpu(((struct wl_ieee80211_hdr_3addr *)buf)->frame_ctl) &
                 IEEE80211_FCTL_STYPE;
    if (IEEE80211_STYPE_PROBE_RESP == frame_styp)
    {
        wait_ack = 0;
        CFG80211_INFO("IEEE80211_STYPE_PROBE_RESP");
#ifdef CONFIG_LPS
        if (ZT_RETURN_FAIL == zt_lps_wakeup(pnic_info, LPS_CTRL_SCAN, 0))
        {
            return -1;
        }
#endif
    }

    if (zt_p2p_is_valid(pnic_info))
    {
        CFG80211_DBG("ch=%d", tx_ch);
        zt_p2p_tx_action_process(pnic_info, (zt_u8 *)buf, len, tx_ch, wait_ack);
    }
    else
    {
        CFG80211_DBG("need to do for not p2p");
    }
    return 0;
}

static void mgmt_frame_register(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
                                struct wireless_dev *wdev,
#else
                                struct net_device *ndev,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0))
                                struct mgmt_frame_regs *upd)
#else
                                zt_u16 frame_type, bool reg)
#endif
{
    return;
}

void zt_cfg80211_ndev_destructor(struct net_device *ndev)
{
    CFG80211_DBG();

#ifdef CONFIG_IOCTL_CFG80211
    if (ndev->ieee80211_ptr)
    {
        zt_kfree(ndev->ieee80211_ptr);
    }
#endif
    free_netdev(ndev);
}

void zt_ap_cfg80211_assoc_event_up(nic_info_st *pnic_info,  zt_u8 *passoc_req,
                                   zt_u32 assoc_req_len)
{
    struct net_device *ndev = pnic_info->ndev;
    struct wireless_dev *pwdev = pnic_info->pwidev;
#if defined(ZT_USE_CFG80211_STA_EVENT) || defined(COMPAT_KERNEL_RELEASE)
#else
    zt_wlan_mgmt_info_t *pwlan_info = pnic_info->wlan_mgmt_info;
    zt_wlan_network_t *pcur_network = &pwlan_info->cur_network;
    zt_s32 freq;
    zt_u32 channel;
#endif
    CFG80211_DBG();

    if (pwdev->wiphy == NULL)
    {
        CFG80211_WARN("wiphy is null!");
        return;
    }

    if (passoc_req && assoc_req_len > 0)
    {
#if defined(ZT_USE_CFG80211_STA_EVENT) || defined(COMPAT_KERNEL_RELEASE)
        struct station_info sinfo;
        zt_u8 ie_offset;
        if (GetFrameSubType(passoc_req) == WIFI_ASSOCREQ)
        {
            ie_offset = _ASOCREQ_IE_OFFSET_;
        }
        else
        {
            ie_offset = _REASOCREQ_IE_OFFSET_;
        }
        zt_memset(&sinfo, 0, sizeof(sinfo));
        sinfo.filled = STATION_INFO_ASSOC_REQ_IES;
        sinfo.assoc_req_ies = passoc_req + WLAN_HDR_A3_LEN + ie_offset;
        sinfo.assoc_req_ies_len = assoc_req_len - WLAN_HDR_A3_LEN - ie_offset;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
        cfg80211_new_sta(pnic_info->pwidev, GetAddr2Ptr(passoc_req), &sinfo, GFP_ATOMIC);
#else
        cfg80211_new_sta(ndev, GetAddr2Ptr(passoc_req), &sinfo, GFP_ATOMIC);
#endif
#else
        channel = pcur_network->channel;
        freq = zt_ch_2_freq(channel);

#ifdef COMPAT_KERNEL_RELEASE
        zt_cfg80211_rx_mgmt(pnic_info, freq, 0, passoc_req, assoc_req_len, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && !defined(CONFIG_CFG80211_FORCE_COMPATIBLE_2_6_37_UNDER)
        zt_cfg80211_rx_mgmt(pnic_info, freq, 0, passoc_req, assoc_req_len, GFP_ATOMIC);
#else
        pwdev->iftype = NL80211_IFTYPE_STATION;
        CFG80211_DBG("iftype=%d before call cfg80211_send_rx_assoc()", pwdev->iftype);
        zt_cfg80211_send_rx_assoc(pnic_info, NULL, passoc_req, assoc_req_len);
        CFG80211_DBG("iftype=%d after call cfg80211_send_rx_assoc()", pwdev->iftype);
        pwdev->iftype = NL80211_IFTYPE_AP;
#endif
#endif
    }
}

void zt_ap_cfg80211_disassoc_event_up(nic_info_st *pnic_info,
                                      wdn_net_info_st *pwdn_info)
{
    struct net_device *ndev = pnic_info->ndev;
#if defined(ZT_USE_CFG80211_STA_EVENT) || defined(COMPAT_KERNEL_RELEASE)
#else
    zt_wlan_mgmt_info_t *pwlan_info = pnic_info->wlan_mgmt_info;
    zt_wlan_network_t *pcur_network = &pwlan_info->cur_network;
    zt_u8 mgmt_buf[128] = { 0 };
    zt_u16 *frame_ctrl;
    struct wl_ieee80211_hdr *pwlanhdr;
    zt_s32 freq;
    zt_u32 channel;
    zt_u8 *pmgmt_frame;
    zt_u16 frame_len;
    zt_u16 reason;
#endif
    CFG80211_DBG();
#ifdef CFG_ENABLE_AP_MODE
    pwdn_info->state = E_WDN_AP_STATE_READY;
#endif
#if defined(ZT_USE_CFG80211_STA_EVENT) || defined(COMPAT_KERNEL_RELEASE)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
    cfg80211_del_sta(pnic_info->pwidev, pwdn_info->mac, GFP_ATOMIC);
#else
    cfg80211_del_sta(ndev, pwdn_info->mac, GFP_ATOMIC);
#endif
#else
    channel = pcur_network->channel;
    freq = zt_ch_2_freq(channel);
    reason = pwdn_info->reason_code;
    pmgmt_frame = mgmt_buf;
    pwlanhdr = (struct wl_ieee80211_hdr *)pmgmt_frame;

    frame_ctrl = &(pwlanhdr->frame_ctl);
    *(frame_ctrl) = 0;

    zt_memcpy(pwlanhdr->addr1, pwdn_info->mac, ZT_80211_MAC_ADDR_LEN);
    zt_memcpy(pwlanhdr->addr2, nic_to_local_addr(pnic_info), ZT_80211_MAC_ADDR_LEN);
    zt_memcpy(pwlanhdr->addr3, zt_wlan_get_cur_bssid(pnic_info),
              ZT_80211_MAC_ADDR_LEN);

    SetSeqNum(pwlanhdr, pwdn_info->mgmt_seq);
    pwdn_info->mgmt_seq++;
    SetFrameSubType(pmgmt_frame, WIFI_DEAUTH);

    pmgmt_frame += sizeof(struct wl_ieee80211_hdr_3addr);
    frame_len = sizeof(struct wl_ieee80211_hdr_3addr);

    reason = zt_cpu_to_le16(reason);
    pmgmt_frame = zt_80211_set_fixed_ie(pmgmt_frame, _RSON_CODE_, (zt_u8 *)&reason,
                                        &frame_len);

#ifdef COMPAT_KERNEL_RELEASE
    zt_cfg80211_rx_mgmt(pnic_info, freq, 0, mgmt_buf, frame_len, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && !defined(CONFIG_CFG80211_FORCE_COMPATIBLE_2_6_37_UNDER)
    zt_cfg80211_rx_mgmt(pnic_info, freq, 0, mgmt_buf, frame_len, GFP_ATOMIC);
#else
    cfg80211_send_disassoc(ndev, mgmt_buf, frame_len);
#endif
#endif
}

#ifdef CFG_ENABLE_AP_MODE
static zt_s32  monitor_open(struct net_device *ndev)
{
    CFG80211_DBG();

    return 0;
}

static zt_s32  monitor_close(struct net_device *ndev)
{
    CFG80211_DBG();

    return 0;
}

static zt_s32  monitor_xmit_entry(struct sk_buff *skb, struct net_device *ndev)
{
    CFG80211_DBG();

    return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0))
static void  monitor_set_multicast_list(struct net_device *ndev)
{
    CFG80211_DBG();

    return ;
}
#endif

static zt_s32  monitor_set_mac_address(struct net_device *ndev, void *addr)
{
    CFG80211_DBG();

    return 0;
}

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2, 6, 29))
static const struct net_device_ops zt_cfg80211_monitor_if_ops =
{
    .ndo_open = monitor_open,
    .ndo_stop = monitor_close,
    .ndo_start_xmit = monitor_xmit_entry,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0))
    .ndo_set_multicast_list = monitor_set_multicast_list,
#endif
    .ndo_set_mac_address = monitor_set_mac_address,
};
#endif

static zt_s32 add_monitor(nic_info_st *pnic_info, zt_s8 *name,
                          struct net_device **ndev)
{
    zt_s32 ret = 0;
    struct net_device *mon_ndev = NULL;
    struct wireless_dev *mon_wdev = NULL;
    struct zt_netdev_priv *pnpi;
    struct zt_widev_priv *pwdev_priv = pnic_info->widev_priv;

    if (!name)
    {
        CFG80211_WARN(" without specific name");
        ret = -EINVAL;
        goto out;
    }

    if (pwdev_priv->pmon_ndev)
    {
        CFG80211_DBG("monitor interface exist");
        ret = -EBUSY;
        goto out;
    }

    mon_ndev = alloc_etherdev(sizeof(struct zt_netdev_priv));
    if (!mon_ndev)
    {
        CFG80211_WARN(" allocate ndev fail");
        ret = -ENOMEM;
        goto out;
    }

    mon_ndev->type = ARPHRD_IEEE80211_RADIOTAP;
    zt_strncpy(mon_ndev->name, name, IFNAMSIZ);
    mon_ndev->name[IFNAMSIZ - 1] = 0;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 8))
    mon_ndev->priv_destructor = zt_cfg80211_ndev_destructor;
#else
    mon_ndev->destructor = zt_cfg80211_ndev_destructor;
#endif

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2, 6, 29))
    mon_ndev->netdev_ops = &zt_cfg80211_monitor_if_ops;
#else
    mon_ndev->open = monitor_open;
    mon_ndev->stop = monitor_close;
    mon_ndev->hard_start_xmit = monitor_xmit_entry;
    mon_ndev->set_mac_address = monitor_set_mac_address;
#endif

    pnpi = netdev_priv(mon_ndev);
    pnpi->priv = pnic_info;
    pnpi->priv_size = sizeof(pnic_info);

    mon_wdev = (struct wireless_dev *)zt_kzalloc(sizeof(struct wireless_dev));
    if (!mon_wdev)
    {
        CFG80211_WARN(" allocate mon_wdev fail");
        ret = -ENOMEM;
        goto out;
    }

    mon_wdev->wiphy = ((struct wireless_dev *)pnic_info->pwidev)->wiphy;
    mon_wdev->netdev = mon_ndev;
    mon_wdev->iftype = NL80211_IFTYPE_MONITOR;
    mon_ndev->ieee80211_ptr = mon_wdev;

    ret = register_netdevice(mon_ndev);
    if (ret)
    {
        goto out;
    }

    *ndev = pwdev_priv->pmon_ndev = mon_ndev;
    zt_memcpy(pwdev_priv->ifname_mon, name, IFNAMSIZ + 1);

out:
    if (ret && mon_wdev)
    {
        zt_kfree(mon_wdev);
        mon_wdev = NULL;
    }

    if (ret && mon_ndev)
    {
        free_netdev(mon_ndev);
        *ndev = mon_ndev = NULL;
    }

    return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
static struct wireless_dev *
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)) || defined(COMPAT_KERNEL_RELEASE)
static struct net_device *
#else
static zt_s32
#endif
add_virtual_intf(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
                 const char *name,
#else
                 char *name,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
                 zt_u8 name_assign_type,
#endif
                 enum nl80211_iftype type,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
                 zt_u32 *flags,
#endif
                 struct vif_params *params)
{
    zt_s32 ret = 0;
    struct net_device *ndev = NULL;
    nic_info_st *pnic_info = *((nic_info_st **)wiphy_priv(wiphy));

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));
    CFG80211_DBG("wiphy:%s, name:%s, type:%d\n", wiphy_name(wiphy), name, type);

    switch (type)
    {
        case NL80211_IFTYPE_ADHOC:
        case NL80211_IFTYPE_AP_VLAN:
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_MESH_POINT:
            ret = -ENODEV;
            break;
        case NL80211_IFTYPE_MONITOR:
            ret = add_monitor(pnic_info, (zt_s8 *)name, &ndev);
            break;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
        case NL80211_IFTYPE_P2P_CLIENT:
#endif
        case NL80211_IFTYPE_STATION:
            ret = -ENODEV;
            break;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
        case NL80211_IFTYPE_P2P_GO:
#endif
        case NL80211_IFTYPE_AP:
            ret = -ENODEV;
            break;
        default:
            ret = -ENODEV;
            CFG80211_WARN("Unsupported interface type\n");
            break;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
    return ndev ? ndev->ieee80211_ptr : ERR_PTR(ret);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)) || defined(COMPAT_KERNEL_RELEASE)
    return ndev ? ndev : ERR_PTR(ret);
#else
    return ret;
#endif
}

static zt_s32 del_virtual_intf(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
                               struct wireless_dev *wdev
#else
                               struct net_device *ndev
#endif
                              )
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
    struct net_device *ndev = wdev->netdev;
#endif
    zt_s32 ret = 0;
    nic_info_st *pnic_info;
    struct zt_widev_priv *pwdev;
    ndev_priv_st *pndev_priv;

    CFG80211_DBG();

    if (!ndev)
    {
        ret = -EINVAL;
        goto exit;
    }

    pndev_priv = netdev_priv(ndev);
    pnic_info = pndev_priv->nic;
    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));
    pwdev = pnic_info->widev_priv;



    unregister_netdevice(ndev);

    if (ndev == pwdev->pmon_ndev)
    {
        pwdev->pmon_ndev = NULL;
        pwdev->ifname_mon[0] = '\0';
        CFG80211_DBG(" remove monitor interface");
    }

exit:
    return ret;
}

static zt_s32 add_beacon(nic_info_st *pnic_info, const zt_u8 *head,
                         size_t head_len,
                         const zt_u8 *tail, size_t tail_len)
{

    zt_s32 ret = 0;
    zt_u8 *pbuf = NULL;
    uint len = 0;
    uint wps_ielen = 0;
    //zt_u8 *p2p_ie;
    zt_u32 p2p_ielen = 0;
    zt_u8 got_p2p_ie = zt_false;
    p2p_info_st *p2p_info = pnic_info->p2p;


    CFG80211_DBG("beacon_head_len=%zu, beacon_tail_len=%zu", head_len, tail_len);

    if (zt_mlme_check_mode(pnic_info, ZT_MASTER_MODE) != zt_true)
    {
        return -EINVAL;
    }
    if (head_len < 24)
    {
        return -EINVAL;
    }
    pbuf = zt_kzalloc(head_len + tail_len);
    if (!pbuf)
    {
        return -ENOMEM;
    }
    zt_memcpy(pbuf, (void *)head + 24, head_len - 24);
    zt_memcpy(pbuf + head_len - 24, (void *)tail, tail_len);

    len = head_len + tail_len - 24;

    if (zt_wlan_get_wps_ie(pbuf + 12, len - 12, NULL, &wps_ielen))
    {
        CFG80211_INFO("add bcn, wps_ielen=%d\n", wps_ielen);
    }

    if (zt_p2p_is_valid(pnic_info))
    {
        if (zt_p2p_get_ie(pbuf + _FIXED_IE_LENGTH_, len - _FIXED_IE_LENGTH_, NULL,
                          &p2p_ielen))
        {
            got_p2p_ie = zt_true;
            CFG80211_INFO("got p2p_ie, len = %d\n", p2p_ielen);

            if (p2p_info->p2p_state == P2P_STATE_NONE)
            {
                CFG80211_INFO("Enable P2P for the first time\n");
                zt_p2p_enable(pnic_info, P2P_ROLE_GO);
            }
            else
            {
                CFG80211_INFO("enter GO mode , p2p_ielen=%d\n", p2p_ielen);
                zt_p2p_set_role(p2p_info, P2P_ROLE_GO);
                zt_p2p_set_state(p2p_info, P2P_STATE_GONEGO_OK);
                p2p_info->intent = 15;
            }

        }

    }

    if (zt_ap_set_beacon(pnic_info, pbuf, len, ZT_MLME_FRAMEWORK_NETLINK) == 0)
    {
        ret = 0;
    }
    else
    {
        ret = -EINVAL;
    }

    zt_kfree(pbuf);

    return ret;
}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)) && !defined(COMPAT_KERNEL_RELEASE)
static zt_s32 add_beacon_cb(struct wiphy *wiphy, struct net_device *ndev,
                            struct beacon_parameters *info)
{
    zt_s32 ret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
    struct net_device *ndev = wdev->netdev;
#endif
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        return -EINVAL;
    }

    ret =
        add_beacon(pnic_info, info->head, info->head_len, info->tail,
                   info->tail_len);

    zt_ap_work_start(pnic_info);

    return ret;
}

static zt_s32 set_beacon_cb(struct wiphy *wiphy, struct net_device *ndev,
                            struct beacon_parameters *info)
{
    CFG80211_DBG();

    add_beacon_cb(wiphy, ndev, info);

    return 0;
}

static zt_s32 del_beacon_cb(struct wiphy *wiphy, struct net_device *ndev)
{
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    if (zt_ap_work_stop(pnic_info))
    {
        return -EINVAL;
    }
    return 0;
}
#else
static zt_s32 cfg80211_start_ap(struct wiphy *wiphy, struct net_device *ndev,
                                struct cfg80211_ap_settings *settings)
{
    zt_s32 ret = 0;
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;
    sec_info_st *psec_info = pnic_info->sec_info;
    zt_wlan_mgmt_info_t *pwlan_info = pnic_info->wlan_mgmt_info;
    zt_wlan_network_t *pcur_network = &pwlan_info->cur_network;

    CFG80211_DBG(" hidden_ssid:%d, auth_type:%d\n", settings->hidden_ssid,
                 settings->auth_type);

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        return -EINVAL;
    }

    psec_info->dot11AuthAlgrthm =
        settings->auth_type == NL80211_AUTHTYPE_OPEN_SYSTEM ? dot11AuthAlgrthm_Open :
        settings->auth_type == NL80211_AUTHTYPE_SHARED_KEY ? dot11AuthAlgrthm_Shared :
        dot11AuthAlgrthm_Auto;

    ret =
        add_beacon(pnic_info, settings->beacon.head,
                   settings->beacon.head_len, settings->beacon.tail,
                   settings->beacon.tail_len);

    pcur_network->hidden_ssid_mode = (zt_80211_hidden_ssid_e)settings->hidden_ssid;
    if (settings->ssid && settings->ssid_len)
    {
        pcur_network->hidden_ssid.length = settings->ssid_len;
        zt_memcpy(pcur_network->hidden_ssid.data, settings->ssid, settings->ssid_len);
    }
    zt_ap_work_start(pnic_info);
    return ret;
}

static zt_s32 cfg80211_change_beacon(struct wiphy *wiphy,
                                     struct net_device *ndev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 7, 0))
                                     struct cfg80211_ap_update *info
#else
				     struct cfg80211_beacon_data *info
#endif
)
{
    zt_s32 ret = 0;
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    ret =
        add_beacon(pnic_info, 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 7, 0))
		   info->beacon.head, info->beacon.head_len, info->beacon.tail,
                   info->beacon.tail_len
#else
		   info->head, info->head_len, info->tail,
                   info->tail_len
#endif
	);
    return ret;
}

static zt_s32 cfg80211_stop_ap(struct wiphy *wiphy, struct net_device *ndev
/*TODO: android #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 137))*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 19, 2))
    ,unsigned int link_id
#endif
)
{
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    if (zt_ap_work_stop(pnic_info))
    {
        return -EINVAL;
    }
    return 0;
}
#endif
static zt_s32 add_station(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
                          struct wireless_dev *wdev,
#else
                          struct net_device *ndev,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0))
                          zt_u8 *mac,
#else
                          const zt_u8 *mac,
#endif
                          struct station_parameters *params)
{
    zt_s32 ret = 0;
    CFG80211_DBG();

    return ret;
}

static zt_s32 del_station(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
                          struct wireless_dev *wdev,
#else
                          struct net_device *ndev,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0))
                          zt_u8 *mac
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0))
                          const zt_u8 *mac
#else
                          struct station_del_parameters *params
#endif
                         )
{
    const zt_u8 *target_mac;
    zt_u16 reason_code;
    wdn_net_info_st *pwdn_info = NULL;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
    struct net_device *ndev = wdev->netdev;
#endif
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;

    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0))
    target_mac = mac;
    reason_code = ZT_80211_REASON_PREV_AUTH_NOT_VALID;
#else
    target_mac = params->mac;
    reason_code = params->reason_code;
#endif
    if (zt_mlme_check_mode(pnic_info, ZT_MASTER_MODE) != zt_true)
    {
        CFG80211_WARN("sys mode is not WIFI_AP_STATE");
        return -EINVAL;
    }

    if (!target_mac)
    {
        CFG80211_DBG("flush all sta, and cam_entry");

        if (zt_ap_deauth_all_sta(pnic_info, reason_code))
        {
            return -EINVAL;
        }
        return 0;
    }

    CFG80211_DBG("free sta macaddr =" ZT_MAC_FMT, ZT_MAC_ARG(target_mac));

    if (zt_80211_is_bcast_addr(target_mac))
    {
        return -EINVAL;
    }

    pwdn_info = zt_wdn_find_info(pnic_info, (zt_u8 *)target_mac);
    if (pwdn_info != NULL)
    {
        CFG80211_DBG("wdn state:%d", pwdn_info->state);
        if (pwdn_info->state > E_WDN_AP_STATE_ASSOC)
        {
            pwdn_info->reason_code = reason_code;
            CFG80211_DBG("free psta, aid=%d\n", pwdn_info->aid);
            if (zt_mlme_check_mode(pnic_info, ZT_MASTER_MODE) == zt_true)
            {
                if (pwdn_info->mode == ZT_MASTER_MODE)
                {
                    zt_ap_msg_load(pnic_info, &pwdn_info->ap_msg,
                                   ZT_AP_MSG_TAG_DEAUTH_FRAME, NULL, 0);
                }
            }
            CFG80211_INFO("wdn_remove :"ZT_MAC_FMT, ZT_MAC_ARG(target_mac));
            if (zt_p2p_is_valid(pnic_info))
            {
                if (pwdn_info->is_p2p_device && 1 >= zt_wdn_get_cnt(pnic_info))
                {
                    CFG80211_INFO("p2p restart");
                    zt_p2p_cannel_remain_on_channel(pnic_info, 1);
                    zt_p2p_enable(pnic_info, P2P_ROLE_DEVICE);
                }
            }

        }
    }
    else
    {
        CFG80211_DBG("the wdn has never been added");
    }

    return 0;
}

static zt_s32 change_station(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
                             struct wireless_dev *wdev,
#else
                             struct net_device *ndev,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0))
                             zt_u8 *mac,
#else
                             const zt_u8 *mac,
#endif
                             struct station_parameters *params)
{
    CFG80211_DBG("mac addr:"ZT_MAC_FMT, ZT_MAC_ARG(mac));
    CFG80211_DBG("aid:%d", params->aid);

    return 0;
}


static zt_s32 dump_station(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
                           struct wireless_dev *wdev,
#else
                           struct net_device *ndev,
#endif
                           zt_s32 idx, zt_u8 *mac,
                           struct station_info *sinfo)
{

    zt_s32 ret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0))
    struct net_device *ndev = wdev->netdev;
#endif
    ndev_priv_st *pndev_priv = netdev_priv(ndev);
    nic_info_st *pnic_info = pndev_priv->nic;

    wdn_net_info_st *pwdn_info = NULL;
    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));

    pwdn_info = zt_wdn_find_info_by_id(pnic_info, (zt_u8)idx);

    if (pwdn_info == NULL)
    {
        CFG80211_DBG("Station is not found\n");
        ret = -ENOENT;
        goto exit;
    }
    zt_memcpy(mac, pwdn_info->mac, ETH_ALEN);
    sinfo->filled = 0;
    sinfo->filled |= ZT_BIT(NL80211_STA_INFO_SIGNAL);

exit:
    return ret;
}

static zt_s32 change_bss(struct wiphy *wiphy, struct net_device *ndev,
                         struct bss_parameters *params)
{
    CFG80211_DBG();

    return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
static zt_s32 set_channel(struct wiphy *wiphy
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
                          , struct net_device *ndev
#endif
                          , struct ieee80211_channel *chan
                          , enum nl80211_channel_type channel_type)
{
    zt_s32 chan_target = (zt_u8) ieee80211_frequency_to_channel(chan->center_freq);
    zt_s32 chan_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
    zt_s32 chan_width = CHANNEL_WIDTH_20;
    nic_info_st *pnic_info = *((nic_info_st **)wiphy_priv(wiphy));

    CFG80211_DBG();

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        return -EINVAL;
    }

    switch (channel_type)
    {
        case NL80211_CHAN_NO_HT:
        case NL80211_CHAN_HT20:
            chan_width = CHANNEL_WIDTH_20;
            chan_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
            break;
        case NL80211_CHAN_HT40MINUS:
            chan_width = CHANNEL_WIDTH_40;
            chan_offset = HAL_PRIME_CHNL_OFFSET_UPPER;
            break;
        case NL80211_CHAN_HT40PLUS:
            chan_width = CHANNEL_WIDTH_40;
            chan_offset = HAL_PRIME_CHNL_OFFSET_LOWER;
            break;
        default:
            chan_width = CHANNEL_WIDTH_20;
            chan_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
            break;
    }

    zt_hw_info_set_channel_bw(pnic_info, chan_target, chan_width, chan_offset);

    return 0;
}

#endif
#endif



#if defined(CONFIG_PNO_SUPPORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
static zt_s32 _sched_scan_start(struct wiphy *wiphy,
                                struct net_device *dev,
                                struct cfg80211_sched_scan_request
                                *request)
{
    CFG80211_DBG();

    return 0;
}


static zt_s32 _sched_scan_stop(struct wiphy *wiphy,
                               struct net_device *dev)
{
    CFG80211_DBG();

    return 0;
}
#endif

#ifdef ZT_CONFIG_P2P
static zt_s32 cfg80211_remain_on_channel_cb(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
        struct wireless_dev *wdev,
#else
        struct net_device *ndev,
#endif
        struct ieee80211_channel *channel,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
        enum nl80211_channel_type
        channel_type,
#endif
        zt_u32 duration, zt_u64 *cookie
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7, 2, 0))
	, const zt_u8* rx_addr
#endif //7.2.0
	)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
    struct net_device *ndev = wdev->netdev;
#endif

    zt_u8 remain_ch = (zt_u8) ieee80211_frequency_to_channel(channel->center_freq);

    struct cfg80211_wifidirect_info *pcfg80211_wdinfo;
    ndev_priv_st *pndev_priv;
    p2p_info_st *p2p_info;
    nic_info_st *pnic_info;
    zt_u8 is_p2p_find = zt_false;

    CFG80211_DBG("start");
#ifndef CONFIG_RADIO_WORK
#define WL_ROCH_DURATION_ENLARGE
#define WL_ROCH_BACK_OP
#endif

    if (ndev == NULL)
    {
        return -EINVAL;
    }

    pndev_priv = netdev_priv(ndev);
    pnic_info = pndev_priv->nic;
    p2p_info = pnic_info->p2p;
    pcfg80211_wdinfo = &pndev_priv->cfg80211_wifidirect;

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        return -EINVAL;
    }

    if (!zt_p2p_is_valid(pnic_info))
    {
        /*register callback function*/
        zt_cfg80211_p2p_cb_reg(pnic_info);
        zt_p2p_enable(pnic_info, P2P_ROLE_DEVICE);
    }

    {
        zt_bool bConnect = zt_false;

        zt_mlme_get_connect(pndev_priv->nic, &bConnect);
        if (bConnect == zt_true)
            return 0;
    }

    p2p_info->listen_channel = remain_ch;

    if (p2p_info && p2p_info->is_ro_ch)
    {
        CFG80211_INFO("it is already remain on channel");
        zt_p2p_cannel_remain_on_channel(pnic_info, 1);
    }
    is_p2p_find = (duration < (p2p_info->ext_listen_interval)) ? zt_true : zt_false;
    *cookie = atomic_inc_return(&pcfg80211_wdinfo->ro_ch_cookie_gen);
    CFG80211_INFO("[%d] mac addr: "ZT_MAC_FMT", cookie:%llx, remain_ch:%d, duration=%d, nego:0x%x",
                  pnic_info->ndev_id, ZT_MAC_ARG(nic_to_local_addr(pnic_info)), *cookie, remain_ch, duration, p2p_info->go_negoing);
    zt_memcpy(&pcfg80211_wdinfo->remain_on_ch_channel, channel, sizeof(struct ieee80211_channel));
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
    pcfg80211_wdinfo->remain_on_ch_type = channel_type;
#endif
    pcfg80211_wdinfo->remain_on_ch_cookie = *cookie;

#if 0
    while (0 != duration && duration < 400)
    {
        duration = duration * 3;
    }
#endif
    pcfg80211_wdinfo->duration = duration;
    if (zt_false == zt_p2p_is_valid(pnic_info))
    {
        CFG80211_DBG("[%d] mac addr: "ZT_MAC_FMT ", not support p2p",
                     pnic_info->ndev_id, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));
        return 0;
    }

    p2p_info->ro_ch_duration = duration;
    p2p_info->remain_ch = remain_ch;

    zt_scan_wait_done(pnic_info, zt_false, 200);
    zt_scan_wait_done(pnic_info->buddy_nic, zt_false, 200);
    zt_mlme_scan_abort(pnic_info);
    zt_mlme_scan_abort(pnic_info->buddy_nic);

    if (p2p_info->scb.ready_on_channel)
    {
        p2p_info->scb.ready_on_channel(pnic_info, NULL, 0);
    }
    zt_p2p_remain_on_channel(pnic_info);
    CFG80211_INFO("end");
    return 0;

}

static zt_s32 cfg80211_cancel_remain_on_channel_cb(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
        struct wireless_dev *wdev,
#else
        struct net_device *ndev,
#endif
        zt_u64 cookie)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
    struct net_device *ndev = wdev->netdev;
#endif

    p2p_info_st *p2p_info   = NULL;
    ndev_priv_st *pndev_priv = NULL;
    nic_info_st *pnic_info   = NULL;
    pndev_priv = netdev_priv(ndev);

    pnic_info = pndev_priv->nic;
    p2p_info = pnic_info->p2p;
    CFG80211_DBG("mac addr: "ZT_MAC_FMT, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));
    if (ndev == NULL)
    {
        return -EINVAL;
    }

    if (pnic_info->is_driver_critical)
    {
        CFG80211_WARN("driver enter crital");
        return -EINVAL;
    }

    if (zt_false == zt_p2p_is_valid(pnic_info))
    {
        CFG80211_DBG("[%d] mac addr: "ZT_MAC_FMT ", not support p2p",
                     pnic_info->ndev_id, ZT_MAC_ARG(nic_to_local_addr(pnic_info)));
        return 0;
    }

    if (zt_false == p2p_info->is_ro_ch)
    {
        return 0;
    }

    //zt_os_api_timer_unreg(&p2p_info->remain_ch_timer);

    CFG80211_INFO("cookie:0x%llx~ 0x%llx, nego:0x%x\n",
                  cookie, pndev_priv->cfg80211_wifidirect.remain_on_ch_cookie,
                  p2p_info->go_negoing);
    zt_p2p_cannel_remain_on_channel(pnic_info, 1);
    CFG80211_DBG("end");
    return 0;

}
#endif // ZT_CONFIG_P2P

static struct cfg80211_ops zt_cfg80211_ops =
{
    .change_virtual_intf = _cfg80211_change_iface,

    .add_key = _add_key_cb,
    .get_key = _get_key_cb, //
    .del_key = _del_key_cb,
    .set_default_key = _set_default_key_cb,

    .get_station = _cfg80211_get_station,
    .scan = _call_scan_cb,
    .set_wiphy_params = _set_wiphy_params,
    .connect = _connect_cb,
    .disconnect = _disconnect_cb,

    .join_ibss = _join_ibss_cb, //
    .leave_ibss = _leave_ibss_cb, //

    .set_tx_power = _call_set_txpower,
    .get_tx_power = _call_get_txpower,
    .set_power_mgmt = _cfg80211_set_power_mgmt, //

    .set_pmksa = _set_pmksa_cb, //
    .del_pmksa = _del_pmksa_cb, //
    .flush_pmksa = _flush_pmksa_cb, //

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
    .set_monitor_channel = _set_monitor_channel, //
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
    .mgmt_tx = _cfg80211_Mgmt_Tx, //
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
    .update_mgmt_frame_registrations = mgmt_frame_register,
#else
    .mgmt_frame_register = mgmt_frame_register, //
#endif
#elif  (LINUX_VERSION_CODE>=KERNEL_VERSION(2, 6, 34) && LINUX_VERSION_CODE<=KERNEL_VERSION(2, 6, 35))
    .action = _cfg80211_Mgmt_Tx,
#endif

#if defined(CONFIG_PNO_SUPPORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
    .sched_scan_start = _sched_scan_start, //
    .sched_scan_stop = _sched_scan_stop, //
#endif

#ifdef CFG_ENABLE_AP_MODE
    .add_virtual_intf = add_virtual_intf,
    .del_virtual_intf = del_virtual_intf,

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)) && !defined(COMPAT_KERNEL_RELEASE)
    .add_beacon = add_beacon_cb,
    .set_beacon = set_beacon_cb,
    .del_beacon = del_beacon_cb,
#else
    .start_ap = cfg80211_start_ap,
    .change_beacon = cfg80211_change_beacon,
    .stop_ap = cfg80211_stop_ap,
#endif

    .add_station = add_station,
    .del_station = del_station,
    .change_station = change_station,
    .dump_station = dump_station,
    .change_bss = change_bss,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
    .set_channel = set_channel,
#endif
#endif

#ifdef ZT_CONFIG_P2P
    .remain_on_channel = cfg80211_remain_on_channel_cb,
    .cancel_remain_on_channel = cfg80211_cancel_remain_on_channel_cb,
#endif

};

zt_s32 zt_cfg80211_alloc(nic_info_st *pnic_info)
{
    struct net_device *pndev = pnic_info->ndev;
    struct wiphy *pwiphy;
    struct wireless_dev *pwidev;
    zt_widev_priv_t *pwidev_priv;

    CFG80211_DBG();

    pnic_info->pwiphy = NULL;
    pnic_info->pwidev = NULL;

    /* alloc wireless phy object */
    pwiphy = wiphy_new(&zt_cfg80211_ops, sizeof(hif_node_st *));
    if (NULL == pwiphy)
    {
        CFG80211_WARN("allocate wiphy device fail !!");
        return -1;
    }
    if (wiphy_cfg(pwiphy))
    {
        CFG80211_WARN("wiphy config fail !!");
        return -2;
    }
    set_wiphy_dev(pwiphy, pnic_info->dev);
    set_wiphy_pirv(pwiphy, pnic_info);
    pnic_info->pwiphy = pwiphy;

    /* alloc wireless device */
    pwidev = (void *)zt_kzalloc(sizeof(struct wireless_dev));
    if (NULL == pwidev)
    {
        CFG80211_INFO("allocate wireless device fail !!");
        return -3;
    }
    pwidev->wiphy           = pwiphy;
    pwidev->iftype          = NL80211_IFTYPE_STATION;
    pndev->ieee80211_ptr    = pwidev;
    pwidev->netdev          = pndev;
    pnic_info->pwidev       = pwidev;

    /* initialize wireless private data */
    pwidev_priv                 = pnic_info->widev_priv;
    pwidev_priv->pwidev         = pwidev;
    pwidev_priv->pmon_ndev      = NULL;
    pwidev_priv->ifname_mon[0]  = '\0';
    pwidev_priv->pnic_info      = pnic_info;
    pwidev_priv->pscan_request  = NULL;
    zt_os_api_lock_init(&pwidev_priv->scan_req_lock, ZT_LOCK_TYPE_BH);
    pwidev_priv->power_mgmt     = zt_false;

    pwidev_priv->bandroid_scan = zt_true;

    atomic_set(&pwidev_priv->ro_ch_to, 1);
    atomic_set(&pwidev_priv->switch_ch_to, 1);

    return 0;
}

zt_s32 zt_cfg80211_reg(struct wiphy *pwiphy)
{
    CFG80211_DBG();

    return wiphy_register(pwiphy);
}

void zt_cfg80211_widev_unreg(nic_info_st *pnic_info)
{
    struct wireless_dev *pwidev;
    struct net_device *pndev;

    if (NULL == pnic_info)
    {
        CFG80211_WARN("pnic_info null");
        return;
    }
    CFG80211_DBG("ndev_id:%d", pnic_info->ndev_id);

    zt_scan_wait_done(pnic_info, zt_true, 400);

    pwidev = pnic_info->pwidev;
    pndev = pnic_info->ndev;

/*TODO:android #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))  */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 19, 2))
    if (pwidev->valid_links && pwidev->links[0].client.current_bss)
    {
        zt_u8 is_local_disc = 1;
        CFG80211_INFO("clear current_bss by cfg80211_disconnected");
        cfg80211_disconnected(pndev, 0, NULL, 0, is_local_disc, GFP_ATOMIC);
    }
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0))
    if (pwidev->current_bss)
    {
        zt_u8 is_local_disc = 1;
        CFG80211_INFO("clear current_bss by cfg80211_disconnected");
        cfg80211_disconnected(pndev, 0, NULL, 0, is_local_disc, GFP_ATOMIC);
    }
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)) && \
    (LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0))) || \
    defined(COMPAT_KERNEL_RELEASE)
    if (pwidev->current_bss)
    {
        CFG80211_INFO("clear current_bss by cfg80211_disconnected");
        cfg80211_disconnected(pndev, 0, NULL, 0, GFP_ATOMIC);
    }
#endif
}

zt_s32 zt_cfg80211_remain_on_channel_expired(void *nic, void *param,
        zt_u32 param_len)
{

    nic_info_st *pnic_info  = NULL;
    ndev_priv_st *ndev_priv = NULL;
    cfg80211_wifidirect_info_st *cfg_wfdirect_info = NULL;
    zt_widev_priv_t *pwidev = NULL;
    CFG80211_DBG("start");

    if (NULL == nic)
    {
        LOG_E("[%s, %d] input param is null", __func__, __LINE__);
        return ZT_RETURN_FAIL;
    }

    pnic_info = nic;
    ndev_priv = netdev_priv(pnic_info->ndev);
    if (NULL == ndev_priv)
    {
        LOG_E("[%s, %d] input param is null", __func__, __LINE__);
        return ZT_RETURN_FAIL;
    }

    cfg_wfdirect_info = &ndev_priv->cfg80211_wifidirect;
    pwidev = pnic_info->widev_priv;
    CFG80211_INFO("cookie:0x%llx", cfg_wfdirect_info->remain_on_ch_cookie);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
    cfg80211_remain_on_channel_expired(pnic_info->ndev,
                                       cfg_wfdirect_info->remain_on_ch_cookie,
                                       &cfg_wfdirect_info->remain_on_ch_channel, cfg_wfdirect_info->remain_on_ch_type,
                                       GFP_KERNEL);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
    cfg80211_remain_on_channel_expired(pwidev->pwidev,
                                       cfg_wfdirect_info->remain_on_ch_cookie,
                                       &cfg_wfdirect_info->remain_on_ch_channel, cfg_wfdirect_info->remain_on_ch_type,
                                       GFP_KERNEL);
#else
    cfg80211_remain_on_channel_expired(pwidev->pwidev,
                                       cfg_wfdirect_info->remain_on_ch_cookie,
                                       &cfg_wfdirect_info->remain_on_ch_channel, GFP_ATOMIC);
#endif

    return ZT_RETURN_OK;

}

zt_s32 zt_cfg80211_p2p_rx_mgmt(void *nic_info, void *param, zt_u32 param_len)
{
    zt_s32 freq = 0;
    nic_info_st *pnic_info = NULL;
    zt_u8 *pmgmt_frame = param;
    zt_u32 frame_len = param_len;
    p2p_info_st *p2p_info = NULL;

    CFG80211_DBG("start");

    if (NULL == nic_info)
    {
        LOG_E("[%s, %d] input param is null", __func__, __LINE__);
        return ZT_RETURN_FAIL;
    }
    pnic_info   = nic_info;
    p2p_info    = pnic_info->p2p;
    freq = zt_ch_2_freq(p2p_info->report_ch);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) || defined(COMPAT_KERNEL_RELEASE)
    zt_cfg80211_rx_mgmt(pnic_info, freq, 0, pmgmt_frame, frame_len, GFP_ATOMIC);
#else
    cfg80211_rx_action(pnic_info->ndev, freq, pmgmt_frame, frame_len, GFP_ATOMIC);
#endif
    CFG80211_INFO("report_ch:%d, freq:%d", p2p_info->report_ch, freq);
    return 0;
}

zt_s32 zt_cfg80211_p2p_ready_on_channel(void *nic_info, void *param,
                                        zt_u32 param_len)
{
    nic_info_st *pnic_info  = NULL;
    ndev_priv_st *ndev_priv = NULL;
    zt_widev_priv_t *pwidev = NULL;
    cfg80211_wifidirect_info_st *cfg_wifi_info = NULL;

    CFG80211_DBG("start");
    if (NULL == nic_info)
    {
        LOG_E("[%s, %d] input param is null", __func__, __LINE__);
        return ZT_RETURN_FAIL;
    }

    pnic_info = nic_info;
    ndev_priv = netdev_priv(pnic_info->ndev);
    if (NULL == ndev_priv)
    {
        LOG_E("[%s, %d] input param is null", __func__, __LINE__);
        return ZT_RETURN_FAIL;
    }
    pwidev = &ndev_priv->widev_priv;
    cfg_wifi_info = &ndev_priv->cfg80211_wifidirect;
    CFG80211_DBG("[%d] remain_on_ch_cookie:%lld", pnic_info->ndev_id,
                 cfg_wifi_info->remain_on_ch_cookie);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
    zt_cfg80211_ready_on_channel(pnic_info->ndev,
                                 cfg_wifi_info->remain_on_ch_cookie, &cfg_wifi_info->remain_on_ch_channel,
                                 cfg_wifi_info->remain_on_ch_type, cfg_wifi_info->duration, GFP_KERNEL);
#else
    zt_cfg80211_ready_on_channel(pwidev->pwidev, cfg_wifi_info->remain_on_ch_cookie,
                                 &cfg_wifi_info->remain_on_ch_channel, cfg_wifi_info->remain_on_ch_type,
                                 cfg_wifi_info->duration, GFP_KERNEL);
#endif
    return 0;
}

#endif

