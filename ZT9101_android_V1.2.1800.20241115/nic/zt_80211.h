/*
 * zt_80211.h
 *
 * This file contains all the prototypes for the zt_80211.c file
 *
 * Author: luozhi
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
#ifndef __ZT_80211_H__
#define __ZT_80211_H__

#include "zt_typedef.h"

/* ETH Header */
#define ZT_ETH_ALEN               (6)
#define ZT_ETH_HLEN               (14)
#define ZT_EEPROM_MAX_SIZE        (512)
#define ZT_SSID_LEN               (32)
#define ZT_MAX_BITRATES           (8)
#define ZT_RATES_NUM              (13)
#define ZT_MCS_NUM                (16)
#define ZT_MAX_CHANNEL_NUM        (14)
#define ZT_MAX_WPA_IE_LEN         (256)
#define ZT_MAX_WPS_IE_LEN         (512)
#define ZT_MAX_P2P_IE_LEN         (256)
#define ZT_MAX_WFD_IE_LEN         (128)
#define ZT_TID_NUM                16
#define ZT_ETH_P_IP               0x0800
#define ZT_ETH_P_ARP              0x0806
#define ZT_ETH_P_EAPOL            0x888e
#define ZT_ETH_P_WAPI             0x88B4
#define ZT_ETH_P_ATALK            0x809B
#define ZT_ETH_P_AARP             0x80F3
#define ZT_ETH_P_8021Q            0x8100
#define ZT_ETH_P_IPX              0x8137
#define ZT_ETH_P_IPV6             0x86DD
#define ZT_ETH_P_PPP_DISC         0x8863
#define ZT_ETH_P_PPP_SES          0x8864

#define ZT_IS_MCAST(mac)            (mac[0] & 0x01)
#define ZT_IP_MCAST_MAC(mac)        ((mac[0]==0x01)&&(mac[1]==0x00)&&(mac[2]==0x5e))
#define ZT_ICMPV6_MCAST_MAC(mac)    ((mac[0]==0x33)&&(mac[1]==0x33)&&(mac[2]!=0xff))
#define ZT_BROADCAST_MAC_ADDR(mac)  (((mac[0] == 0xff) && (mac[1] == 0xff) && \
                                      (mac[2] == 0xff) && (mac[3] == 0xff) && (mac[4] == 0xff) && (mac[5] == 0xff)))

#define ZT_IPV4_SRC(_iphdr)             (((zt_u8 *)(_iphdr)) + 12)
#define ZT_IPV4_DST(_iphdr)             (((zt_u8 *)(_iphdr)) + 16)
#define ZT_GET_IPV4_IHL(_iphdr)         zt_be_bits_to_u8(((zt_u8 *)(_iphdr)) + 0, 0, 4)
#define ZT_GET_IPV4_PROTOCOL(_iphdr)    zt_be_bits_to_u8(((zt_u8 *)(_iphdr)) + 9, 0, 8)
#define ZT_GET_IPV4_TOS(_iphdr)         zt_be_bits_to_u8(((zt_u8 *)(_iphdr)) + 1, 0, 8)

#define ZT_GET_UDP_SRC(_udphdr)         zt_be_bits_to_u16(((zt_u8 *)(_udphdr)) + 0, 0, 16)
#define ZT_GET_UDP_DST(_udphdr)         zt_be_bits_to_u16(((zt_u8 *)(_udphdr)) + 2, 0, 16)

#define ZT_TCP_SRC(_tcphdr)             (((zt_u8 *)(_tcphdr)) + 0)
#define ZT_TCP_DST(_tcphdr)             (((zt_u8 *)(_tcphdr)) + 2)
#define ZT_GET_TCP_FIN(_tcphdr)         zt_be_bits_to_u8(((zt_u8 *)(_tcphdr)) + 13, 0, 1)
#define ZT_GET_TCP_SYN(_tcphdr)         zt_be_bits_to_u8(((zt_u8 *)(_tcphdr)) + 13, 1, 1)
#define ZT_GET_TCP_ACK(_tcphdr)         zt_be_bits_to_u8(((zt_u8 *)(_tcphdr)) + 13, 4, 1)

#define ZT_MAC_FMT     "%02x:%02x:%02x:%02x:%02x:%02x"
#define ZT_MAC_ARG(x)  ((zt_u8*)(x))[0],((zt_u8*)(x))[1],((zt_u8*)(x))[2],((zt_u8*)(x))[3],((zt_u8*)(x))[4],((zt_u8*)(x))[5]

#define MAX_SUBFRAME_COUNT  64

/*
 * ieee802.11 MAC frame type define
 */

#define ZT_80211_MAC_ADDR_LEN   6

typedef zt_u8 zt_80211_ftype_t;
/* MAC frame type */
#define ZT_80211_FTYPE_MGMT                   0x0000
#define ZT_80211_FTYPE_CTL                    0x0004
#define ZT_80211_FTYPE_DATA                   0x0008
#define ZT_80211_FTYPE_EXT                    0x000C

typedef zt_u8 zt_80211_stype_t;
/* management subtype */
#define ZT_80211_STYPE_ASSOC_REQ              0x0000
#define ZT_80211_STYPE_ASSOC_RESP             0x0010
#define ZT_80211_STYPE_REASSOC_REQ            0x0020
#define ZT_80211_STYPE_REASSOC_RESP           0x0030
#define ZT_80211_STYPE_PROBE_REQ              0x0040
#define ZT_80211_STYPE_PROBE_RESP             0x0050
#define ZT_80211_STYPE_BEACON                 0x0080
#define ZT_80211_STYPE_ATIM                   0x0090
#define ZT_80211_STYPE_DISASSOC               0x00A0
#define ZT_80211_STYPE_AUTH                   0x00B0
#define ZT_80211_STYPE_DEAUTH                 0x00C0
#define ZT_80211_STYPE_ACTION                 0x00D0
/* control subtype */
#define ZT_80211_STYPE_CTL_EXT                0x0060
#define ZT_80211_STYPE_BACK_REQ               0x0080
#define ZT_80211_STYPE_BACK                   0x0090
#define ZT_80211_STYPE_PSPOLL                 0x00A0
#define ZT_80211_STYPE_RTS                    0x00B0
#define ZT_80211_STYPE_CTS                    0x00C0
#define ZT_80211_STYPE_ACK                    0x00D0
#define ZT_80211_STYPE_CFEND                  0x00E0
#define ZT_80211_STYPE_CFENDACK               0x00F0
/* data subtype */
#define ZT_80211_STYPE_DATA                   0x0000
#define ZT_80211_STYPE_DATA_CFACK             0x0010
#define ZT_80211_STYPE_DATA_CFPOLL            0x0020
#define ZT_80211_STYPE_DATA_CFACKPOLL         0x0030
#define ZT_80211_STYPE_NULLFUNC               0x0040
#define ZT_80211_STYPE_CFACK                  0x0050
#define ZT_80211_STYPE_CFPOLL                 0x0060
#define ZT_80211_STYPE_CFACKPOLL              0x0070
#define ZT_80211_STYPE_QOS_DATA               0x0080
#define ZT_80211_STYPE_QOS_DATA_CFACK         0x0090
#define ZT_80211_STYPE_QOS_DATA_CFPOLL        0x00A0
#define ZT_80211_STYPE_QOS_DATA_CFACKPOLL     0x00B0
#define ZT_80211_STYPE_QOS_NULLFUNC           0x00C0
#define ZT_80211_STYPE_QOS_CFACK              0x00D0
#define ZT_80211_STYPE_QOS_CFPOLL             0x00E0
#define ZT_80211_STYPE_QOS_CFACKPOLL          0x00F0

/* frame type */
typedef enum
{
    /* management frame */
    ZT_80211_FRM_ASSOC_REQ          = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_ASSOC_REQ,
    ZT_80211_FRM_ASSOC_RESP         = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_ASSOC_RESP,
    ZT_80211_FRM_REASSOC_REQ        = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_REASSOC_REQ,
    ZT_80211_FRM_REASSOC_RESP       = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_REASSOC_RESP,
    ZT_80211_FRM_PROBE_REQ          = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_PROBE_REQ,
    ZT_80211_FRM_PROBE_RESP         = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_PROBE_RESP,
    ZT_80211_FRM_BEACON             = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_BEACON,
    ZT_80211_FRM_ATIM               = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_ATIM,
    ZT_80211_FRM_DISASSOC           = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_DISASSOC,
    ZT_80211_FRM_AUTH               = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_AUTH,
    ZT_80211_FRM_DEAUTH             = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_DEAUTH,
    ZT_80211_FRM_ACTION             = ZT_80211_FTYPE_MGMT | ZT_80211_STYPE_ACTION,
    /* control frame */
    ZT_80211_FRM_CTL_EXT            = ZT_80211_FTYPE_CTL | ZT_80211_STYPE_CTL_EXT,
    ZT_80211_FRM_BACK_REQ           = ZT_80211_FTYPE_CTL | ZT_80211_STYPE_BACK_REQ,
    ZT_80211_FRM_BACK               = ZT_80211_FTYPE_CTL | ZT_80211_STYPE_BACK,
    ZT_80211_FRM_PSPOLL             = ZT_80211_FTYPE_CTL | ZT_80211_STYPE_PSPOLL,
    ZT_80211_FRM_RTS                = ZT_80211_FTYPE_CTL | ZT_80211_STYPE_RTS,
    ZT_80211_FRM_CTS                = ZT_80211_FTYPE_CTL | ZT_80211_STYPE_CTS,
    ZT_80211_FRM_ACK                = ZT_80211_FTYPE_CTL | ZT_80211_STYPE_ACK,
    ZT_80211_FRM_CFEND              = ZT_80211_FTYPE_CTL | ZT_80211_STYPE_CFEND,
    ZT_80211_FRM_CFENDACK           = ZT_80211_FTYPE_CTL | ZT_80211_STYPE_CFENDACK,
    /* data frame */
    ZT_80211_FRM_DATA               = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_DATA,
    ZT_80211_FRM_DATA_CFACK         = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_DATA_CFACK,
    ZT_80211_FRM_DATA_CFPOLL        = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_DATA_CFPOLL,
    ZT_80211_FRM_DATA_CFACKPOLL     = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_DATA_CFACKPOLL,
    ZT_80211_FRM_NULLFUNC           = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_NULLFUNC,
    ZT_80211_FRM_CFACK              = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_CFACK,
    ZT_80211_FRM_CFPOLL             = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_CFPOLL,
    ZT_80211_FRM_CFACKPOLL          = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_CFACKPOLL,
    ZT_80211_FRM_QOS_DATA           = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_QOS_DATA,
    ZT_80211_FRM_QOS_DATA_CFACK     = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_QOS_DATA_CFACK,
    ZT_80211_FRM_QOS_DATA_CFPOLL    = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_QOS_DATA_CFPOLL,
    ZT_80211_FRM_QOS_DATA_CFACKPOLL = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_QOS_DATA_CFACKPOLL,
    ZT_80211_FRM_QOS_NULLFUNC       = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_QOS_NULLFUNC,
    ZT_80211_FRM_QOS_CFACK          = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_QOS_CFACK,
    ZT_80211_FRM_QOS_CFPOLL         = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_QOS_CFPOLL,
    ZT_80211_FRM_QOS_CFACKPOLL      = ZT_80211_FTYPE_DATA | ZT_80211_STYPE_QOS_CFACKPOLL,
} zt_80211_frame_e;

/* Duration/ID field */
#define MASK_DI_DURATION                        ZT_BITS(0, 14)
#define MASK_DI_AID                             ZT_BITS(0, 13)
#define MASK_DI_AID_MSB                         ZT_BITS(14, 15)
#define MASK_DI_CFP_FIXED_VALUE                 ZT_BIT(15)

/* Sequence Control field */
#define MASK_SC_SEQ_NUM                         ZT_BITS(4, 15)
#define MASK_SC_SEQ_NUM_OFFSET                  4
#define MASK_SC_FRAG_NUM                        ZT_BITS(0, 3)
#define INVALID_SEQ_CTRL_NUM                    0x000F

/* QoS Control field */
#define TID_NUM                                 16
#define TID_MASK                                ZT_BITS(0, 3)
#define EOSP                                    ZT_BIT(4)
#define ACK_POLICY                              ZT_BITS(5, 6)
#define A_MSDU_PRESENT                          ZT_BIT(7)

#define ZT_80211_HT_CTRL_LEN                             4

/* FCS field */
#define FCS_LEN                                 4

/* frame control field desc 2bytes */
#define PROTOCL_VER         ZT_BITS(0,1)
#define TYPE                ZT_BITS(2,3)
#define SUB_TYPE            ZT_BITS(4,7)
#define MORE_FRAG           ZT_BIT(10)
#define RETRY               ZT_BIT(11)
#define PWRMGT              ZT_BIT(12)
#define MORE_DATA           ZT_BIT(13)
#define PROTECTED           ZT_BIT(14)
#define ORDER               ZT_BIT(15)

/*
 * DS bit usage
 *
 * TA = transmitter address
 * RA = receiver address
 * DA = destination address
 * SA = source address
 *
 * ToDS    FromDS  A1(RA)  A2(TA)  A3      A4      Use
 * -----------------------------------------------------------------
 *  0       0       DA      SA      BSSID   -       IBSS/DLS
 *  0       1       DA      BSSID   SA      -       AP -> STA
 *  1       0       BSSID   SA      DA      -       AP <- STA
 *  1       1       RA      TA      DA      SA      unspecified (WDS)
 */

/*******************************************************************************
 *                        ieee802.11 frame format define
 ******************************************************************************/
/*
 * MAC header fields
 */
typedef zt_u16 zt_80211_frame_ctrl_t;
#define ZT_80211_FCTL_VERS              0x0003
#define ZT_80211_FCTL_FTYPE             0x000C
#define ZT_80211_FCTL_STYPE             0x00F0
#define ZT_80211_FCTL_TODS              0x0100
#define ZT_80211_FCTL_FROMDS            0x0200
#define ZT_80211_FCTL_MOREFRAGS         0x0400
#define ZT_80211_FCTL_RETRY             0x0800
#define ZT_80211_FCTL_PM                0x1000
#define ZT_80211_FCTL_MOREDATA          0x2000
#define ZT_80211_FCTL_PROTECTED         0x4000
#define ZT_80211_FCTL_ORDER             0x8000
#define ZT_80211_FCTL_CTL_EXT           0x0F00
/* control extension - for ZT_80211_FRM_CTL_EXT */
#define ZT_80211_CTL_EXT_POLL           0x0200
#define ZT_80211_CTL_EXT_SPR            0x0300
#define ZT_80211_CTL_EXT_GRANT          0x0400
#define ZT_80211_CTL_EXT_DMG_CTS        0x0500
#define ZT_80211_CTL_EXT_DMG_DTS        0x0600
#define ZT_80211_CTL_EXT_SSW            0x0800
#define ZT_80211_CTL_EXT_SSW_FBACK      0x0900
#define ZT_80211_CTL_EXT_SSW_ACK        0x0A00


typedef zt_u16 zt_80211_duration_t;
typedef zt_u8 zt_80211_addr_t[ZT_80211_MAC_ADDR_LEN];
typedef zt_80211_addr_t zt_80211_bssid_t;

typedef zt_u16 zt_80211_seq_ctrl_t;
#define ZT_80211_SCTL_FRAG_MASK         0x000F
#define ZT_80211_SCTL_FRAG_SHIFT        0
#define ZT_80211_SCTL_SEQ_MASK          0xFFF0
#define ZT_80211_SCTL_SEQ_SHIFT         4

/*
 * management frame
 */
/* no element fields */
typedef zt_u64 zt_80211_mgmt_timestamp_t;
typedef zt_u16 zt_80211_mgmt_beacon_interval_t;
typedef zt_u16 zt_80211_mgmt_capab_t;
#define ZT_80211_MGMT_CAPAB_ESS                 (1<<0)
#define ZT_80211_MGMT_CAPAB_IBSS                (1<<1)
#define ZT_80211_MGMT_CAPAB_CF_POLLABLE         (1<<2)
#define ZT_80211_MGMT_CAPAB_CF_POLL_REQUEST     (1<<3)
#define ZT_80211_MGMT_CAPAB_PRIVACY             (1<<4)
#define ZT_80211_MGMT_CAPAB_SHORT_PREAMBLE      (1<<5)
#define ZT_80211_MGMT_CAPAB_PBCC                (1<<6)
#define ZT_80211_MGMT_CAPAB_CHANNEL_AGILITY     (1<<7)
/* 802.11h */
#define ZT_80211_MGMT_CAPAB_SPECTRUM_MGMT       (1<<8)
#define ZT_80211_MGMT_CAPAB_QOS                 (1<<9)
#define ZT_80211_MGMT_CAPAB_SHORT_SLOT_TIME     (1<<10)
#define ZT_80211_MGMT_CAPAB_APSD                (1<<11)
#define ZT_80211_MGMT_CAPAB_RADIO_MEASURE       (1<<12)
#define ZT_80211_MGMT_CAPAB_DSSS_OFDM           (1<<13)
#define ZT_80211_MGMT_CAPAB_DEL_BACK            (1<<14)
#define ZT_80211_MGMT_CAPAB_IMM_BACK            (1<<15)
/*
 * A mesh STA sets the ESS and IBSS capability bits to zero.
 * however, this holds true for p2p probe responses (in the p2p_find
 * phase) as well.
 */
#define ZT_80211_CAPAB_IS_MESH_STA_BSS(cap)  \
    (!((cap) & (ZT_80211_MGMT_CAPAB_ESS | ZT_80211_MGMT_CAPAB_IBSS)))

#define ZT_80211_CAPAB_IS_IBSS(cap)  \
    (!((cap) & ZT_80211_MGMT_CAPAB_ESS) && ((cap) & ZT_80211_MGMT_CAPAB_IBSS))

/* Status codes */
typedef enum
{
    ZT_80211_STATUS_SUCCESS                                     = 0,
    ZT_80211_STATUS_UNSPECIFIED_FAILURE                         = 1,
    ZT_80211_STATUS_CAPS_UNSUPPORTED                            = 10,
    ZT_80211_STATUS_REASSOC_NO_ASSOC                            = 11,
    ZT_80211_STATUS_ASSOC_DENIED_UNSPEC                         = 12,
    ZT_80211_STATUS_NOT_SUPPORTED_AUTH_ALG                      = 13,
    ZT_80211_STATUS_UNKNOWN_AUTH_TRANSACTION                    = 14,
    ZT_80211_STATUS_CHALLENGE_FAIL                              = 15,
    ZT_80211_STATUS_AUTH_TIMEOUT                                = 16,
    ZT_80211_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA                 = 17,
    ZT_80211_STATUS_ASSOC_DENIED_RATES                          = 18,
    /* 802.11b */
    ZT_80211_STATUS_ASSOC_DENIED_NOSHORTPREAMBLE                = 19,
    ZT_80211_STATUS_ASSOC_DENIED_NOPBCC                         = 20,
    ZT_80211_STATUS_ASSOC_DENIED_NOAGILITY                      = 21,
    /* 802.11h */
    ZT_80211_STATUS_ASSOC_DENIED_NOSPECTRUM                     = 22,
    ZT_80211_STATUS_ASSOC_REJECTED_BAD_POWER                    = 23,
    ZT_80211_STATUS_ASSOC_REJECTED_BAD_SUPP_CHAN                = 24,
    /* 802.11g */
    ZT_80211_STATUS_ASSOC_DENIED_NOSHORTTIME                    = 25,
    ZT_80211_STATUS_ASSOC_DENIED_NODSSSOFDM                     = 26,
    /* 802.11w */
    ZT_80211_STATUS_ASSOC_REJECTED_TEMPORARILY                  = 30,
    ZT_80211_STATUS_ROBUST_MGMT_FRAME_POLICY_VIOLATION          = 31,
    /* 802.11i */
    ZT_80211_STATUS_INVALID_IE                                  = 40,
    ZT_80211_STATUS_INVALID_GROUP_CIPHER                        = 41,
    ZT_80211_STATUS_INVALID_PAIRWISE_CIPHER                     = 42,
    ZT_80211_STATUS_INVALID_AKMP                                = 43,
    ZT_80211_STATUS_UNSUPP_RSN_VERSION                          = 44,
    ZT_80211_STATUS_INVALID_RSN_IE_CAP                          = 45,
    ZT_80211_STATUS_CIPHER_SUITE_REJECTED                       = 46,
    /* 802.11e */
    ZT_80211_STATUS_UNSPECIFIED_QOS                             = 32,
    ZT_80211_STATUS_ASSOC_DENIED_NOBANDWIDTH                    = 33,
    ZT_80211_STATUS_ASSOC_DENIED_LOWACK                         = 34,
    ZT_80211_STATUS_ASSOC_DENIED_UNSUPP_QOS                     = 35,
    ZT_80211_STATUS_REQUEST_DECLINED                            = 37,
    ZT_80211_STATUS_INVALID_QOS_PARAM                           = 38,
    ZT_80211_STATUS_CHANGE_TSPEC                                = 39,
    ZT_80211_STATUS_WAIT_TS_DELAY                               = 47,
    ZT_80211_STATUS_NO_DIRECT_LINK                              = 48,
    ZT_80211_STATUS_STA_NOT_PRESENT                             = 49,
    ZT_80211_STATUS_STA_NOT_QSTA                                = 50,
    /* 802.11s */
    ZT_80211_STATUS_ANTI_CLOG_REQUIRED                          = 76,
    ZT_80211_STATUS_FCG_NOT_SUPP                                = 78,
    ZT_80211_STATUS_STA_NO_TBTT                                 = 78,
    /* 802.11ad */
    ZT_80211_STATUS_REJECTED_WITH_SUGGESTED_CHANGES             = 39,
    ZT_80211_STATUS_REJECTED_FOR_DELAY_PERIOD                   = 47,
    ZT_80211_STATUS_REJECT_WITH_SCHEDULE                        = 83,
    ZT_80211_STATUS_PENDING_ADMITTING_FST_SESSION               = 86,
    ZT_80211_STATUS_PERFORMING_FST_NOW                          = 87,
    ZT_80211_STATUS_PENDING_GAP_IN_BA_WINDOW                    = 88,
    ZT_80211_STATUS_REJECT_U_PID_SETTING                        = 89,
    ZT_80211_STATUS_REJECT_DSE_BAND                             = 96,
    ZT_80211_STATUS_DENIED_WITH_SUGGESTED_BAND_AND_CHANNEL      = 99,
    ZT_80211_STATUS_DENIED_DUE_TO_SPECTRUM_MANAGEMENT           = 103,
} zt_80211_statuscode_e;

/* Reason codes */
typedef enum
{
    ZT_80211_REASON_UNSPECIFIED                     = 1,
    ZT_80211_REASON_PREV_AUTH_NOT_VALID             = 2,
    ZT_80211_REASON_DEAUTH_LEAVING                  = 3,
    ZT_80211_REASON_DISASSOC_DUE_TO_INACTIVITY      = 4,
    ZT_80211_REASON_DISASSOC_AP_BUSY                = 5,
    ZT_80211_REASON_CLASS2_FRAME_FROM_NONAUTH_STA   = 6,
    ZT_80211_REASON_CLASS3_FRAME_FROM_NONASSOC_STA  = 7,
    ZT_80211_REASON_DISASSOC_STA_HAS_LEFT           = 8,
    ZT_80211_REASON_STA_REQ_ASSOC_WITHOUT_AUTH      = 9,
    /* 802.11h */
    ZT_80211_REASON_DISASSOC_BAD_POWER             = 10,
    ZT_80211_REASON_DISASSOC_BAD_SUPP_CHAN         = 11,
    /* 802.11i */
    ZT_80211_REASON_INVALID_IE                     = 13,
    ZT_80211_REASON_MIC_FAILURE                    = 14,
    ZT_80211_REASON_4WAY_HANDSHAKE_TIMEOUT         = 15,
    ZT_80211_REASON_GROUP_KEY_HANDSHAKE_TIMEOUT    = 16,
    ZT_80211_REASON_IE_DIFFERENT                   = 17,
    ZT_80211_REASON_INVALID_GROUP_CIPHER           = 18,
    ZT_80211_REASON_INVALID_PAIRWISE_CIPHER        = 19,
    ZT_80211_REASON_INVALID_AKMP                   = 20,
    ZT_80211_REASON_UNSUPP_RSN_VERSION             = 21,
    ZT_80211_REASON_INVALID_RSN_IE_CAP             = 22,
    ZT_80211_REASON_IEEE8021X_FAILED               = 23,
    ZT_80211_REASON_CIPHER_SUITE_REJECTED          = 24,
    /* TDLS (802.11z) */
    ZT_80211_REASON_TDLS_TEARDOWN_UNREACHABLE      = 25,
    ZT_80211_REASON_TDLS_TEARDOWN_UNSPECIFIED      = 26,
    /* 802.11e */
    ZT_80211_REASON_DISASSOC_UNSPECIFIED_QOS       = 32,
    ZT_80211_REASON_DISASSOC_QAP_NO_BANDWIDTH      = 33,
    ZT_80211_REASON_DISASSOC_LOW_ACK               = 34,
    ZT_80211_REASON_DISASSOC_QAP_EXCEED_TXOP       = 35,
    ZT_80211_REASON_QSTA_LEAVE_QBSS                = 36,
    ZT_80211_REASON_QSTA_NOT_USE                   = 37,
    ZT_80211_REASON_QSTA_REQUIRE_SETUP             = 38,
    ZT_80211_REASON_QSTA_TIMEOUT                   = 39,
    ZT_80211_REASON_QSTA_CIPHER_NOT_SUPP           = 45,
    /* 802.11s */
    ZT_80211_REASON_MESH_PEER_CANCELED             = 52,
    ZT_80211_REASON_MESH_MAX_PEERS                 = 53,
    ZT_80211_REASON_MESH_CONFIG                    = 54,
    ZT_80211_REASON_MESH_CLOSE                     = 55,
    ZT_80211_REASON_MESH_MAX_RETRIES               = 56,
    ZT_80211_REASON_MESH_CONFIRM_TIMEOUT           = 57,
    ZT_80211_REASON_MESH_INVALID_GTK               = 58,
    ZT_80211_REASON_MESH_INCONSISTENT_PARAM        = 59,
    ZT_80211_REASON_MESH_INVALID_SECURITY          = 60,
    ZT_80211_REASON_MESH_PATH_ERROR                = 61,
    ZT_80211_REASON_MESH_PATH_NOFORWARD            = 62,
    ZT_80211_REASON_MESH_PATH_DEST_UNREACHABLE     = 63,
    ZT_80211_REASON_MAC_EXISTS_IN_MBSS             = 64,
    ZT_80211_REASON_MESH_CHAN_REGULATORY           = 65,
    ZT_80211_REASON_MESH_CHAN                      = 66,
} zt_80211_reasoncode_e;


/* element fields */
typedef enum
{
    ZT_80211_MGMT_EID_SSID                           = 0,
    ZT_80211_MGMT_EID_SUPP_RATES                     = 1,
    ZT_80211_MGMT_EID_FH_PARAMS                      = 2,  /* reserved now */
    ZT_80211_MGMT_EID_DS_PARAMS                      = 3,
    ZT_80211_MGMT_EID_CF_PARAMS                      = 4,
    ZT_80211_MGMT_EID_TIM                            = 5,
    ZT_80211_MGMT_EID_IBSS_PARAMS                    = 6,
    ZT_80211_MGMT_EID_COUNTRY                        = 7,
    /* 8, 9 reserved */
    ZT_80211_MGMT_EID_REQUEST                        = 10,
    ZT_80211_MGMT_EID_QBSS_LOAD                      = 11,
    ZT_80211_MGMT_EID_EDCA_PARAM_SET                 = 12,
    ZT_80211_MGMT_EID_TSPEC                          = 13,
    ZT_80211_MGMT_EID_TCLAS                          = 14,
    ZT_80211_MGMT_EID_SCHEDULE                       = 15,
    ZT_80211_MGMT_EID_CHALLENGE                      = 16,
    /* 17-31 reserved for challenge text extension */
    ZT_80211_MGMT_EID_PWR_CONSTRAINT                 = 32,
    ZT_80211_MGMT_EID_PWR_CAPABILITY                 = 33,
    ZT_80211_MGMT_EID_TPC_REQUEST                    = 34,
    ZT_80211_MGMT_EID_TPC_REPORT                     = 35,
    ZT_80211_MGMT_EID_SUPPORTED_CHANNELS             = 36,
    ZT_80211_MGMT_EID_CHANNEL_SWITCH                 = 37,
    ZT_80211_MGMT_EID_MEASURE_REQUEST                = 38,
    ZT_80211_MGMT_EID_MEASURE_REPORT                 = 39,
    ZT_80211_MGMT_EID_QUIET                          = 40,
    ZT_80211_MGMT_EID_IBSS_DFS                       = 41,
    ZT_80211_MGMT_EID_ERP_INFO                       = 42,
    ZT_80211_MGMT_EID_TS_DELAY                       = 43,
    ZT_80211_MGMT_EID_TCLAS_PROCESSING               = 44,
    ZT_80211_MGMT_EID_HT_CAPABILITY                  = 45,
    ZT_80211_MGMT_EID_QOS_CAPA                       = 46,
    /* 47 reserved for Broadcom */
    ZT_80211_MGMT_EID_RSN                            = 48,
    ZT_80211_MGMT_EID_802_15_COEX                    = 49,
    ZT_80211_MGMT_EID_EXT_SUPP_RATES                 = 50,
    ZT_80211_MGMT_EID_AP_CHAN_REPORT                 = 51,
    ZT_80211_MGMT_EID_NEIGHBOR_REPORT                = 52,
    ZT_80211_MGMT_EID_RCPI                           = 53,
    ZT_80211_MGMT_EID_MOBILITY_DOMAIN                = 54,
    ZT_80211_MGMT_EID_FAST_BSS_TRANSITION            = 55,
    ZT_80211_MGMT_EID_TIMEOUT_INTERVAL               = 56,
    ZT_80211_MGMT_EID_RIC_DATA                       = 57,
    ZT_80211_MGMT_EID_DSE_REGISTERED_LOCATION        = 58,
    ZT_80211_MGMT_EID_SUPPORTED_REGULATORY_CLASSES   = 59,
    ZT_80211_MGMT_EID_EXT_CHANSWITCH_ANN             = 60,
    ZT_80211_MGMT_EID_HT_OPERATION                   = 61,
    ZT_80211_MGMT_EID_SECONDARY_CHANNEL_OFFSET       = 62,
    ZT_80211_MGMT_EID_BSS_AVG_ACCESS_DELAY           = 63,
    ZT_80211_MGMT_EID_ANTENNA_INFO                   = 64,
    ZT_80211_MGMT_EID_RSNI                           = 65,
    ZT_80211_MGMT_EID_MEASUREMENT_PILOT_TX_INFO      = 66,
    ZT_80211_MGMT_EID_BSS_AVAILABLE_CAPACITY         = 67,
    ZT_80211_MGMT_EID_BSS_AC_ACCESS_DELAY            = 68,
    ZT_80211_MGMT_EID_TIME_ADVERTISEMENT             = 69,
    ZT_80211_MGMT_EID_RRM_ENABLED_CAPABILITIES       = 70,
    ZT_80211_MGMT_EID_MULTIPLE_BSSID                 = 71,
    ZT_80211_MGMT_EID_BSS_COEX_2040                  = 72,
    ZT_80211_MGMT_EID_BSS_INTOLERANT_CHL_REPORT      = 73,
    ZT_80211_MGMT_EID_OVERLAP_BSS_SCAN_PARAM         = 74,
    ZT_80211_MGMT_EID_RIC_DESCRIPTOR                 = 75,
    ZT_80211_MGMT_EID_MMIE                           = 76,
    ZT_80211_MGMT_EID_ASSOC_COMEBACK_TIME            = 77,
    ZT_80211_MGMT_EID_EVENT_REQUEST                  = 78,
    ZT_80211_MGMT_EID_EVENT_REPORT                   = 79,
    ZT_80211_MGMT_EID_DIAGNOSTIC_REQUEST             = 80,
    ZT_80211_MGMT_EID_DIAGNOSTIC_REPORT              = 81,
    ZT_80211_MGMT_EID_LOCATION_PARAMS                = 82,
    ZT_80211_MGMT_EID_NON_TX_BSSID_CAP               = 83,
    ZT_80211_MGMT_EID_SSID_LIST                      = 84,
    ZT_80211_MGMT_EID_MULTI_BSSID_IDX                = 85,
    ZT_80211_MGMT_EID_FMS_DESCRIPTOR                 = 86,
    ZT_80211_MGMT_EID_FMS_REQUEST                    = 87,
    ZT_80211_MGMT_EID_FMS_RESPONSE                   = 88,
    ZT_80211_MGMT_EID_QOS_TRAFFIC_CAPA               = 89,
    ZT_80211_MGMT_EID_BSS_MAX_IDLE_PERIOD            = 90,
    ZT_80211_MGMT_EID_TSF_REQUEST                    = 91,
    ZT_80211_MGMT_EID_TSF_RESPOSNE                   = 92,
    ZT_80211_MGMT_EID_WNM_SLEEP_MODE                 = 93,
    ZT_80211_MGMT_EID_TIM_BCAST_REQ                  = 94,
    ZT_80211_MGMT_EID_TIM_BCAST_RESP                 = 95,
    ZT_80211_MGMT_EID_COLL_IF_REPORT                 = 96,
    ZT_80211_MGMT_EID_CHANNEL_USAGE                  = 97,
    ZT_80211_MGMT_EID_TIME_ZONE                      = 98,
    ZT_80211_MGMT_EID_DMS_REQUEST                    = 99,
    ZT_80211_MGMT_EID_DMS_RESPONSE                   = 100,
    ZT_80211_MGMT_EID_LINK_ID                        = 101,
    ZT_80211_MGMT_EID_WAKEUP_SCHEDUL                 = 102,
    /* 103 reserved */
    ZT_80211_MGMT_EID_CHAN_SWITCH_TIMING             = 104,
    ZT_80211_MGMT_EID_PTI_CONTROL                    = 105,
    ZT_80211_MGMT_EID_PU_BUFFER_STATUS               = 106,
    ZT_80211_MGMT_EID_INTERWORKING                   = 107,
    ZT_80211_MGMT_EID_ADVERTISEMENT_PROTOCOL         = 108,
    ZT_80211_MGMT_EID_EXPEDITED_BW_REQ               = 109,
    ZT_80211_MGMT_EID_QOS_MAP_SET                    = 110,
    ZT_80211_MGMT_EID_ROAMING_CONSORTIUM             = 111,
    ZT_80211_MGMT_EID_EMERGENCY_ALERT                = 112,
    ZT_80211_MGMT_EID_MESH_CONFIG                    = 113,
    ZT_80211_MGMT_EID_MESH_ID                        = 114,
    ZT_80211_MGMT_EID_LINK_METRIC_REPORT             = 115,
    ZT_80211_MGMT_EID_CONGESTION_NOTIFICATION        = 116,
    ZT_80211_MGMT_EID_PEER_MGMT                      = 117,
    ZT_80211_MGMT_EID_CHAN_SWITCH_PARAM              = 118,
    ZT_80211_MGMT_EID_MESH_AWAKE_WINDOW              = 119,
    ZT_80211_MGMT_EID_BEACON_TIMING                  = 120,
    ZT_80211_MGMT_EID_MCCAOP_SETUP_REQ               = 121,
    ZT_80211_MGMT_EID_MCCAOP_SETUP_RESP              = 122,
    ZT_80211_MGMT_EID_MCCAOP_ADVERT                  = 123,
    ZT_80211_MGMT_EID_MCCAOP_TEARDOWN                = 124,
    ZT_80211_MGMT_EID_GANN                           = 125,
    ZT_80211_MGMT_EID_RANN                           = 126,
    ZT_80211_MGMT_EID_EXT_CAPABILITY                 = 127,
    /* 128, 129 reserved for Agere */
    ZT_80211_MGMT_EID_PREQ                           = 130,
    ZT_80211_MGMT_EID_PREP                           = 131,
    ZT_80211_MGMT_EID_PERR                           = 132,
    /* 133-136 reserved for Cisco */
    ZT_80211_MGMT_EID_PXU                            = 137,
    ZT_80211_MGMT_EID_PXUC                           = 138,
    ZT_80211_MGMT_EID_AUTH_MESH_PEER_EXCH            = 139,
    ZT_80211_MGMT_EID_MIC                            = 140,
    ZT_80211_MGMT_EID_DESTINATION_URI                = 141,
    ZT_80211_MGMT_EID_UAPSD_COEX                     = 142,
    ZT_80211_MGMT_EID_WAKEUP_SCHEDULE                = 143,
    ZT_80211_MGMT_EID_EXT_SCHEDULE                   = 144,
    ZT_80211_MGMT_EID_STA_AVAILABILITY               = 145,
    ZT_80211_MGMT_EID_DMG_TSPEC                      = 146,
    ZT_80211_MGMT_EID_DMG_AT                         = 147,
    ZT_80211_MGMT_EID_DMG_CAP                        = 148,
    /* 149 reserved for Cisco */
    ZT_80211_MGMT_EID_CISCO_VENDOR_SPECIFIC          = 150,
    ZT_80211_MGMT_EID_DMG_OPERATION                  = 151,
    ZT_80211_MGMT_EID_DMG_BSS_PARAM_CHANGE           = 152,
    ZT_80211_MGMT_EID_DMG_BEAM_REFINEMENT            = 153,
    ZT_80211_MGMT_EID_CHANNEL_MEASURE_FEEDBACK       = 154,
    /* 155-156 reserved for Cisco */
    ZT_80211_MGMT_EID_AWAKE_WINDOW                   = 157,
    ZT_80211_MGMT_EID_MULTI_BAND                     = 158,
    ZT_80211_MGMT_EID_ADDBA_EXT                      = 159,
    ZT_80211_MGMT_EID_NEXT_PCP_LIST                  = 160,
    ZT_80211_MGMT_EID_PCP_HANDOVER                   = 161,
    ZT_80211_MGMT_EID_DMG_LINK_MARGIN                = 162,
    ZT_80211_MGMT_EID_SWITCHING_STREAM               = 163,
    ZT_80211_MGMT_EID_SESSION_TRANSITION             = 164,
    ZT_80211_MGMT_EID_DYN_TONE_PAIRING_REPORT        = 165,
    ZT_80211_MGMT_EID_CLUSTER_REPORT                 = 166,
    ZT_80211_MGMT_EID_RELAY_CAP                      = 167,
    ZT_80211_MGMT_EID_RELAY_XFER_PARAM_SET           = 168,
    ZT_80211_MGMT_EID_BEAM_LINK_MAINT                = 169,
    ZT_80211_MGMT_EID_MULTIPLE_MAC_ADDR              = 170,
    ZT_80211_MGMT_EID_U_PID                          = 171,
    ZT_80211_MGMT_EID_DMG_LINK_ADAPT_ACK             = 172,
    /* 173 reserved for Symbol */
    ZT_80211_MGMT_EID_MCCAOP_ADV_OVERVIEW            = 174,
    ZT_80211_MGMT_EID_QUIET_PERIOD_REQ               = 175,
    /* 176 reserved for Symbol */
    ZT_80211_MGMT_EID_QUIET_PERIOD_RESP              = 177,
    /* 178-179 reserved for Symbol */
    /* 180 reserved for ISO/IEC 20011 */
    ZT_80211_MGMT_EID_EPAC_POLICY                    = 182,
    ZT_80211_MGMT_EID_CLISTER_TIME_OFF               = 183,
    ZT_80211_MGMT_EID_INTER_AC_PRIO                  = 184,
    ZT_80211_MGMT_EID_SCS_DESCRIPTOR                 = 185,
    ZT_80211_MGMT_EID_QLOAD_REPORT                   = 186,
    ZT_80211_MGMT_EID_HCCA_TXOP_UPDATE_COUNT         = 187,
    ZT_80211_MGMT_EID_HL_STREAM_ID                   = 188,
    ZT_80211_MGMT_EID_GCR_GROUP_ADDR                 = 189,
    ZT_80211_MGMT_EID_ANTENNA_SECTOR_ID_PATTERN      = 190,
    ZT_80211_MGMT_EID_VHT_CAPABILITY                 = 191,
    ZT_80211_MGMT_EID_VHT_OPERATION                  = 192,
    ZT_80211_MGMT_EID_EXTENDED_BSS_LOAD              = 193,
    ZT_80211_MGMT_EID_WIDE_BW_CHANNEL_SWITCH         = 194,
    ZT_80211_MGMT_EID_VHT_TX_POWER_ENVELOPE          = 195,
    ZT_80211_MGMT_EID_CHANNEL_SWITCH_WRAPPER         = 196,
    ZT_80211_MGMT_EID_AID                            = 197,
    ZT_80211_MGMT_EID_QUIET_CHANNEL                  = 198,
    ZT_80211_MGMT_EID_OPMODE_NOTIF                   = 199,

    ZT_80211_MGMT_EID_VENDOR_SPECIFIC                = 221,
    ZT_80211_MGMT_EID_QOS_PARAMETER                  = 222,
} zt_80211_mgmt_eid_e;

/* Action category code */
typedef enum
{
    ZT_80211_CATEGORY_SPECTRUM_MGMT                 = 0,
    ZT_80211_CATEGORY_QOS                           = 1,
    ZT_80211_CATEGORY_DLS                           = 2,
    ZT_80211_CATEGORY_BACK                          = 3,
    ZT_80211_CATEGORY_PUBLIC                        = 4,
    ZT_80211_CATEGORY_RADIO_MEASUREMENT             = 5,
    ZT_80211_CATEGORY_HT                            = 7,
    ZT_80211_CATEGORY_SA_QUERY                      = 8,
    ZT_80211_CATEGORY_PROTECTED_DUAL_OF_ACTION      = 9,
    ZT_80211_CATEGORY_WNM                           = 10,
    ZT_80211_CATEGORY_WNM_UNPROTECTED               = 11,
    ZT_80211_CATEGORY_TDLS                          = 12,
    ZT_80211_CATEGORY_MESH_ACTION                   = 13,
    ZT_80211_CATEGORY_MULTIHOP_ACTION               = 14,
    ZT_80211_CATEGORY_SELF_PROTECTED                = 15,
    ZT_80211_CATEGORY_DMG                           = 16,
    ZT_80211_CATEGORY_WMM                           = 17,
    ZT_80211_CATEGORY_FST                           = 18,
    ZT_80211_CATEGORY_UNPROT_DMG                    = 20,
    ZT_80211_CATEGORY_VHT                           = 21,
    ZT_80211_CATEGORY_VENDOR_SPECIFIC_PROTECTED     = 126,
    ZT_80211_CATEGORY_P2P                           = 127,
} zt_80211_category_e;

/* SPECTRUM_MGMT action code */
typedef enum
{
    ZT_80211_ACTION_SPCT_MSR_REQ        = 0,
    ZT_80211_ACTION_SPCT_MSR_RPRT       = 1,
    ZT_80211_ACTION_SPCT_TPC_REQ        = 2,
    ZT_80211_ACTION_SPCT_TPC_RPRT       = 3,
    ZT_80211_ACTION_SPCT_CHL_SWITCH     = 4,
} zt_80211_spectrum_mgmt_actioncode_e;

/* HT action codes */
typedef enum
{
    ZT_80211_HT_ACTION_NOTIFY_CHANWIDTH     = 0,
    ZT_80211_HT_ACTION_SMPS                 = 1,
    ZT_80211_HT_ACTION_PSMP                 = 2,
    ZT_80211_HT_ACTION_PCO_PHASE            = 3,
    ZT_80211_HT_ACTION_CSI                  = 4,
    ZT_80211_HT_ACTION_NONCOMPRESSED_BF     = 5,
    ZT_80211_HT_ACTION_COMPRESSED_BF        = 6,
    ZT_80211_HT_ACTION_ASEL_IDX_FEEDBACK    = 7,
} zt_80211_ht_actioncode_e;

/* VHT action codes */
typedef enum
{
    ZT_80211_VHT_ACTION_COMPRESSED_BF   = 0,
    ZT_80211_VHT_ACTION_GROUPID_MGMT    = 1,
    ZT_80211_VHT_ACTION_OPMODE_NOTIF    = 2,
} zt_80211_vht_actioncode_e;

/* Self Protected Action codes */
typedef enum
{
    ZT_80211_SP_RESERVED                = 0,
    ZT_80211_SP_MESH_PEERING_OPEN       = 1,
    ZT_80211_SP_MESH_PEERING_CONFIRM    = 2,
    ZT_80211_SP_MESH_PEERING_CLOSE      = 3,
    ZT_80211_SP_MGK_INFORM              = 4,
    ZT_80211_SP_MGK_ACK                 = 5,
} zt_80211_self_protected_actioncode_e;

/* Mesh action codes */
typedef enum
{
    ZT_80211_MESH_ACTION_LINK_METRIC_REPORT,
    ZT_80211_MESH_ACTION_HWMP_PATH_SELECTION,
    ZT_80211_MESH_ACTION_GATE_ANNOUNCEMENT,
    ZT_80211_MESH_ACTION_CONGESTION_CONTROL_NOTIFICATION,
    ZT_80211_MESH_ACTION_MCCA_SETUP_REQUEST,
    ZT_80211_MESH_ACTION_MCCA_SETUP_REPLY,
    ZT_80211_MESH_ACTION_MCCA_ADVERTISEMENT_REQUEST,
    ZT_80211_MESH_ACTION_MCCA_ADVERTISEMENT,
    ZT_80211_MESH_ACTION_MCCA_TEARDOWN,
    ZT_80211_MESH_ACTION_TBTT_ADJUSTMENT_REQUEST,
    ZT_80211_MESH_ACTION_TBTT_ADJUSTMENT_RESPONSE,
} zt_80211_mesh_actioncode_t;


typedef struct
{
    zt_u8 element_id;
    zt_u8 len;
    zt_u8 data[0];
} zt_80211_mgmt_ie_t;

/* U-APSD queue for WMM IEs sent by AP */
#define ZT_80211_MGMT_WMM_IE_AP_QOSINFO_UAPSD       (1<<7)
#define ZT_80211_MGMT_WMM_IE_AP_QOSINFO_PARAM_SET_CNT_MASK  0x0f
/* U-APSD queues for WMM IEs sent by STA */
#define ZT_80211_MGMT_WMM_IE_STA_QOSINFO_AC_VO      (1<<0)
#define ZT_80211_MGMT_WMM_IE_STA_QOSINFO_AC_VI      (1<<1)
#define ZT_80211_MGMT_WMM_IE_STA_QOSINFO_AC_BK      (1<<2)
#define ZT_80211_MGMT_WMM_IE_STA_QOSINFO_AC_BE      (1<<3)
#define ZT_80211_MGMT_WMM_IE_STA_QOSINFO_AC_MASK    0x0f
/* U-APSD max SP length for WMM IEs sent by STA */
#define ZT_80211_MGMT_WMM_IE_STA_QOSINFO_SP_ALL     0x00
#define ZT_80211_MGMT_WMM_IE_STA_QOSINFO_SP_2       0x01
#define ZT_80211_MGMT_WMM_IE_STA_QOSINFO_SP_4       0x02
#define ZT_80211_MGMT_WMM_IE_STA_QOSINFO_SP_6       0x03
#define ZT_80211_MGMT_WMM_IE_STA_QOSINFO_SP_MASK    0x03
#define ZT_80211_MGMT_WMM_IE_STA_QOSINFO_SP_SHIFT   5

/* 802.11n HT capability MSC set */
#define ZT_80211_MGMT_HT_MCS_RX_HIGHEST_MASK            0x3ff
#define ZT_80211_MGMT_HT_MCS_TX_DEFINED                 0x01
#define ZT_80211_MGMT_HT_MCS_TX_RX_DIFF                 0x02
/* value 0 == 1 stream etc */
#define ZT_80211_MGMT_HT_MCS_TX_MAX_STREAMS_MASK        0x0C
#define ZT_80211_MGMT_HT_MCS_TX_MAX_STREAMS_SHIFT       2
#define ZT_80211_MGMT_HT_MCS_TX_MAX_STREAMS             4
#define ZT_80211_MGMT_HT_MCS_TX_UNEQUAL_MODULATION      0x10
/*
 * 802.11n D5.0 20.3.5 / 20.6 says:
 * - indices 0 to 7 and 32 are single spatial stream
 * - 8 to 31 are multiple spatial streams using equal modulation
 *   [8..15 for two streams, 16..23 for three and 24..31 for four]
 * - remainder are multiple spatial streams using unequal modulation
 */
#define ZT_80211_MGMT_HT_MCS_UNEQUAL_MODULATION_START   33
#define ZT_80211_MGMT_HT_MCS_UNEQUAL_MODULATION_START_BYTE \
    (ZT_80211_MGMT_HT_MCS_UNEQUAL_MODULATION_START / 8)


/* 802.11n HT capabilities masks (for cap_info) */
#define ZT_80211_MGMT_HT_CAP_LDPC_CODING                0x0001
#define ZT_80211_MGMT_HT_CAP_SUP_WIDTH_20_40            0x0002
#define ZT_80211_MGMT_HT_CAP_SM_PS                      0x000C
#define ZT_80211_MGMT_HT_CAP_SM_PS_SHIFT                2
#define ZT_80211_MGMT_HT_CAP_GRN_FLD                    0x0010
#define ZT_80211_MGMT_HT_CAP_SGI_20                     0x0020
#define ZT_80211_MGMT_HT_CAP_SGI_40                     0x0040
#define ZT_80211_MGMT_HT_CAP_TX_STBC                    0x0080
#define ZT_80211_MGMT_HT_CAP_RX_STBC                    0x0300
#define ZT_80211_MGMT_HT_CAP_RX_STBC_1R                 0x0100
#define ZT_80211_MGMT_HT_CAP_RX_STBC_2R                 0x0200
#define ZT_80211_MGMT_HT_CAP_RX_STBC_3R                 0x0300
#define ZT_80211_MGMT_HT_CAP_RX_STBC_SHIFT              8
#define ZT_80211_MGMT_HT_CAP_DELAY_BA                   0x0400
#define ZT_80211_MGMT_HT_CAP_MAX_AMSDU                  0x0800
#define ZT_80211_MGMT_HT_CAP_DSSSCCK40                  0x1000
#define ZT_80211_MGMT_HT_CAP_RESERVED                   0x2000
#define ZT_80211_MGMT_HT_CAP_40MHZ_INTOLERANT           0x4000
#define ZT_80211_MGMT_HT_CAP_LSIG_TXOP_PROT             0x8000
/* 802.11n HT capability AMPDU settings (for ampdu_params_info) */
#define ZT_80211_MGMT_HT_AMPDU_PARM_FACTOR              0x03
#define ZT_80211_MGMT_HT_AMPDU_PARM_DENSITY             0x1C
#define ZT_80211_MGMT_HT_AMPDU_PARM_DENSITY_SHIFT       2
/* 802.11n HT extended capabilities masks (for extended_ht_cap_info) */
#define ZT_80211_MGMT_HT_EXT_CAP_PCO                    0x0001
#define ZT_80211_MGMT_HT_EXT_CAP_PCO_TIME               0x0006
#define ZT_80211_MGMT_HT_EXT_CAP_PCO_TIME_SHIFT         1
#define ZT_80211_MGMT_HT_EXT_CAP_MCS_FB                 0x0300
#define ZT_80211_MGMT_HT_EXT_CAP_MCS_FB_SHIFT           8
#define ZT_80211_MGMT_HT_EXT_CAP_HTC_SUP                0x0400
#define ZT_80211_MGMT_HT_EXT_CAP_RD_RESPONDER           0x0800
/*
 * Maximum length of AMPDU that the STA can receive in high-throughput (HT).
 * Length = 2 ^ (13 + max_ampdu_length_exp) - 1 (octets)
 */
enum zt_80211_mgmt_ht_max_ampdu_length_exp
{
    ZT_80211_MGMT_HT_MAX_AMPDU_8K   = 0,
    ZT_80211_MGMT_HT_MAX_AMPDU_16K  = 1,
    ZT_80211_MGMT_HT_MAX_AMPDU_32K  = 2,
    ZT_80211_MGMT_HT_MAX_AMPDU_64K  = 3,
} ;
#define ZT_80211_MGMT_HT_MAX_AMPDU_FACTOR                13
/* Minimum MPDU start spacing */
enum zt_80211_mgmt_ht_min_mpdu_spacing
{
    ZT_80211_MGMT_HT_MPDU_DENSITY_NONE  = 0, /* No restriction */
    ZT_80211_MGMT_HT_MPDU_DENSITY_0_25  = 1, /* 1/4 usec */
    ZT_80211_MGMT_HT_MPDU_DENSITY_0_5   = 2, /* 1/2 usec */
    ZT_80211_MGMT_HT_MPDU_DENSITY_1     = 3, /* 1 usec */
    ZT_80211_MGMT_HT_MPDU_DENSITY_2     = 4, /* 2 usec */
    ZT_80211_MGMT_HT_MPDU_DENSITY_4     = 5, /* 4 usec */
    ZT_80211_MGMT_HT_MPDU_DENSITY_8     = 6, /* 8 usec */
    ZT_80211_MGMT_HT_MPDU_DENSITY_16    = 7, /* 16 usec */
} ;

/* for ht_param */
#define ZT_80211_MGMT_HT_OP_PARAM_CHA_SEC_OFFSET            0x03
#define ZT_80211_MGMT_HT_OP_PARAM_CHA_SEC_NONE              0x00
#define ZT_80211_MGMT_HT_OP_PARAM_CHA_SEC_ABOVE             0x01
#define ZT_80211_MGMT_HT_OP_PARAM_CHA_SEC_BELOW             0x03
#define ZT_80211_MGMT_HT_OP_PARAM_CHAN_WIDTH_ANY            0x04
#define ZT_80211_MGMT_HT_OP_PARAM_RIFS_MODE                 0x08
/* for operation_mode */
#define ZT_80211_MGMT_HT_OP_MODE_PROTECTION                 0x0003
#define ZT_80211_MGMT_HT_OP_MODE_PROTECTION_NONE            0
#define ZT_80211_MGMT_HT_OP_MODE_PROTECTION_NONMEMBER       1
#define ZT_80211_MGMT_HT_OP_MODE_PROTECTION_20MHZ           2
#define ZT_80211_MGMT_HT_OP_MODE_PROTECTION_NONHT_MIXED     3
#define ZT_80211_MGMT_HT_OP_MODE_NON_GF_STA_PRSNT           0x0004
#define ZT_80211_MGMT_HT_OP_MODE_NON_HT_STA_PRSNT           0x0010
/* for stbc_param */
#define ZT_80211_MGMT_HT_STBC_PARAM_DUAL_BEACON             0x0040
#define ZT_80211_MGMT_HT_STBC_PARAM_DUAL_CTS_PROT           0x0080
#define ZT_80211_MGMT_HT_STBC_PARAM_STBC_BEACON             0x0100
#define ZT_80211_MGMT_HT_STBC_PARAM_LSIG_TXOP_FULLPROT      0x0200
#define ZT_80211_MGMT_HT_STBC_PARAM_PCO_ACTIVE              0x0400
#define ZT_80211_MGMT_HT_STBC_PARAM_PCO_PHASE               0x0800



#define ZT_80211_MAX_SSID_LEN   32
typedef zt_u8 zt_80211_mgmt_ssid_t[ZT_80211_MAX_SSID_LEN + 1];

#define ZT_80211_IES_SIZE_MAX   768
#define ZT_WLAN_MGMT_TAG_PROBEREQ_P2P_SIZE_MAX \
    (ZT_OFFSETOF(zt_80211_mgmt_t, probe_req.variable) + ZT_80211_IES_SIZE_MAX)

#define ZT_80211_MGMT_BEACON_SIZE_MAX \
    (ZT_OFFSETOF(zt_80211_mgmt_t, beacon.variable) + ZT_80211_IES_SIZE_MAX)
#define ZT_80211_MGMT_PROBERSP_SIZE_MAX \
    (ZT_OFFSETOF(zt_80211_mgmt_t, probe_resp.variable) + ZT_80211_IES_SIZE_MAX)
#define ZT_80211_MGMT_AUTH_SIZE_MAX \
    (ZT_OFFSETOF(zt_80211_mgmt_t, auth_seq3) + sizeof(struct auth_seq3_ie) + 12) /* +12: fix for apple smart remote terminal */
#define ZT_80211_MGMT_DEAUTH_SIZE_MIN \
    (ZT_OFFSETOF(zt_80211_mgmt_t, deauth) + sizeof(struct deauth_ie))
#define ZT_80211_MGMT_DEAUTH_SIZE_MAX   ZT_80211_MGMT_DEAUTH_SIZE_MIN
#define ZT_80211_MGMT_ASSOC_SIZE_MAX \
    (ZT_OFFSETOF(zt_80211_mgmt_t, assoc_req.variable) + ZT_80211_IES_SIZE_MAX)
#define ZT_80211_MGMT_DISASSOC_SIZE_MAX \
    (ZT_OFFSETOF(zt_80211_mgmt_t, disassoc) + sizeof(struct disassoc_ie))
#define ZT_80211_MGMT_BA_RSP_SIZE_MAX \
    (ZT_OFFSETOF(zt_add_ba_parm_st, dialog))

/* cipher suite selectors */
#define ZT_80211_RSN_CIPHER_SUITE_USE_GROUP     0x000FAC00
#define ZT_80211_RSN_CIPHER_SUITE_WEP40         0x000FAC01
#define ZT_80211_RSN_CIPHER_SUITE_TKIP          0x000FAC02
/* reserved: 0x000FAC03 */
#define ZT_80211_RSN_CIPHER_SUITE_CCMP          0x000FAC04
#define ZT_80211_RSN_CIPHER_SUITE_WEP104        0x000FAC05
#define ZT_80211_RSN_CIPHER_SUITE_AES_CMAC      0x000FAC06
#define ZT_80211_RSN_CIPHER_SUITE_GCMP          0x000FAC08
#define ZT_80211_RSN_CIPHER_SUITE_GCMP_256      0x000FAC09
#define ZT_80211_RSN_CIPHER_SUITE_CCMP_256      0x000FAC0A
#define ZT_80211_RSN_CIPHER_SUITE_BIP_GMAC_128  0x000FAC0B
#define ZT_80211_RSN_CIPHER_SUITE_BIP_GMAC_256  0x000FAC0C
#define ZT_80211_RSN_CIPHER_SUITE_BIP_CMAC_256  0x000FAC0D

#define ZT_80211_RSN_CIPHER_SUITE_SMS4          0x00147201

/* AKM suite selectors */
#define ZT_80211_AKM_SUITE_8021X                0x000FAC01
#define ZT_80211_AKM_SUITE_PSK                  0x000FAC02
#define ZT_80211_AKM_SUITE_8021X_SHA256         0x000FAC05
#define ZT_80211_AKM_SUITE_PSK_SHA256           0x000FAC06
#define ZT_80211_AKM_SUITE_TDLS                 0x000FAC07
#define ZT_80211_AKM_SUITE_SAE                  0x000FAC08
#define ZT_80211_AKM_SUITE_FT_OVER_SAE          0x000FAC09

#define ZT_80211_MAX_KEY_LEN                    32

#define ZT_80211_PMKID_LEN                      16

#define ZT_80211_OUI_WFA                        0x506f9a
#define ZT_80211_OUI_TYPE_WFA_P2P               9
#define ZT_80211_OUI_MICROSOFT                  0x0050f2
#define ZT_80211_OUI_TYPE_MICROSOFT_WPA         1
#define ZT_80211_OUI_TYPE_MICROSOFT_WMM         2
#define ZT_80211_OUI_TYPE_MICROSOFT_WPS         4

#define ZT_80211_QOS_CTL_LEN                    2
/* 1d tag mask */
#define ZT_80211_QOS_CTL_TAG1D_MASK             0x0007
/* TID mask */
#define ZT_80211_QOS_CTL_TID_MASK               0x000f
/* EOSP */
#define ZT_80211_QOS_CTL_EOSP                   0x0010
/* ACK policy */
#define ZT_80211_QOS_CTL_ACK_POLICY_NORMAL      0x0000
#define ZT_80211_QOS_CTL_ACK_POLICY_NOACK       0x0020
#define ZT_80211_QOS_CTL_ACK_POLICY_NO_EXPL     0x0040
#define ZT_80211_QOS_CTL_ACK_POLICY_BLOCKACK    0x0060
#define ZT_80211_QOS_CTL_ACK_POLICY_MASK        0x0060
#define ZT_80211_QOS_CTL_ACK_POLICY_SHIFT       5
/* A-MSDU 802.11n */
#define ZT_80211_QOS_CTL_A_MSDU_PRESENT         0x0080
#define ZT_80211_QOS_CTL_A_MSDU_PRESENT_SHIFT   7
/* Mesh Control 802.11s */
#define ZT_80211_QOS_CTL_MESH_CONTROL_PRESENT   0x0100

/* Mesh Power Save Level */
#define ZT_80211_QOS_CTL_MESH_PS_LEVEL          0x0200
/* Mesh Receiver Service Period Initiated */
#define ZT_80211_QOS_CTL_RSPI                   0x0400


/* Authentication algorithms */
typedef enum
{
    ZT_80211_AUTH_ALGO_OPEN = 0,
    ZT_80211_AUTH_ALGO_SHARED_KEY,
    ZT_80211_AUTH_ALGO_FT,
    ZT_80211_AUTH_ALGO_SAE,
    ZT_80211_AUTH_ALGO_LEAP = 128,
    ZT_80211_AUTH_ALGO_AUTO,
} zt_80211_auth_algo_e;

typedef enum
{
    ZT_80211_AUTH_SEQ_1 = 1,
    ZT_80211_AUTH_SEQ_2,
    ZT_80211_AUTH_SEQ_3,
    ZT_80211_AUTH_SEQ_4,
} zt_80211_auth_seq_e;

typedef enum
{
    ZT_80211_HIDDEN_SSID_NOT_IN_USE = 0,
    ZT_80211_HIDDEN_SSID_ZERO_LEN,
    ZT_80211_HIDDEN_SSID_ZERO_CONTENTS,

    ZT_80211_HIDDEN_SSID_UNKNOWN,
} zt_80211_hidden_ssid_e;

#define ZT_80211_AUTH_CHALLENGE_LEN             128

zt_u8 *zt_wlan_get_ie(zt_u8 *pbuf, zt_s32 index, zt_s32 *len, zt_s32 limit);
zt_u8 *zt_wlan_get_wps_ie(zt_u8 *temp_ie, zt_u32 temp_len, zt_u8 *wps_ie,
                          zt_u32 *ie_len);
zt_u8 *zt_wlan_get_wps_attr(zt_u8 *wps_ie, zt_u32 wps_ielen,
                            zt_u16 target_attr_id, zt_u8 *buf_attr, zt_u32 *len_attr, zt_u8 flag);
zt_u8 *zt_wlan_get_wps_attr_content(zt_u8 flag, zt_u8 *wps_ie, zt_u32 wps_ielen,
                                    zt_u16 target_attr_id, zt_u8 *buf_content, zt_u32 *len_content);



zt_s32 zt_80211_mgmt_ies_search(void *pies, zt_u16 ies_len, zt_u8 cmp_id,
                                zt_80211_mgmt_ie_t **ppie);


zt_s32 zt_80211_mgmt_ies_search_with_oui(void *pies, zt_u16 ies_len,
        zt_u8 cmp_id, zt_u8 *oui,
        zt_80211_mgmt_ie_t **ppie);


zt_u8 *zt_80211_set_fixed_ie(zt_u8 *pbuf, zt_u32 len, zt_u8 *source,
                             zt_u16 *frlen);

zt_bool zt_80211_is_snap_hdr(zt_u8 *phdr);
zt_s32 zt_80211_mgmt_wpa_parse(void *pwpa, zt_u16 len,
                               zt_u32 *pmulticast_cipher, zt_u32 *punicast_cipher);
zt_s32 zt_80211_mgmt_wpa_survey(void *data, zt_u16 data_len,
                                void **pwpa_ie, zt_u16 *pwpa_ie_len,
                                zt_u32 *pmulticast_cipher, zt_u32 *punicast_cipher);
zt_s32 zt_80211_mgmt_rsn_parse(void *prsn, zt_u16 len,
                               zt_u32 *pgroup_cipher, zt_u32 *pairwise_cipher);
zt_s32 zt_80211_mgmt_rsn_survey(void *data, zt_u16 data_len,
                                void **prsn_ie, zt_u16 *prsn_ie_len,
                                zt_u32 *pgroup_cipher, zt_u32 *pairwise_cipher);
zt_s32 zt_80211_mgmt_wmm_parse(void *pwmm, zt_u16 len);
zt_s32 zt_wlan_get_sec_ie(zt_u8 *in_ie, zt_u32 in_len,
                          zt_u8 *rsn_ie, zt_u16 *rsn_len,
                          zt_u8 *wpa_ie, zt_u16 *wpa_len,
                          zt_u8 flag);

enum MGN_RATE
{
    MGN_1M = 0x02,
    MGN_2M = 0x04,
    MGN_5_5M = 0x0B,
    MGN_6M = 0x0C,
    MGN_9M = 0x12,
    MGN_11M = 0x16,
    MGN_12M = 0x18,
    MGN_18M = 0x24,
    MGN_24M = 0x30,
    MGN_36M = 0x48,
    MGN_48M = 0x60,
    MGN_54M = 0x6C,
    MGN_MCS32 = 0x7F,
    MGN_MCS0,
    MGN_MCS1,
    MGN_MCS2,
    MGN_MCS3,
    MGN_MCS4,
    MGN_MCS5,
    MGN_MCS6,
    MGN_MCS7,
    MGN_UNKNOWN
};


enum RATEID_IDX
{
    RATEID_IDX_B,
    RATEID_IDX_G,
    RATEID_IDX_BG,
    RATEID_IDX_GN,
    RATEID_IDX_BGN_20M,
    RATEID_IDX_BGN_40M,
};


enum VCS_TYPE
{
    NONE_VCS,
    RTS_CTS,
    CTS_TO_SELF
};

#define ZT_80211_WEP_KEYS       4
#define ZT_80211_WEP_KEY_LEN    13

#define ZT_80211_CRYPT_ALG_NAME_LEN     16


#define ZT_CIPHER_SUITE_NONE     ZT_BIT(0)
#define ZT_CIPHER_SUITE_WEP40    ZT_BIT(1)
#define ZT_CIPHER_SUITE_WEP104   ZT_BIT(2)
#define ZT_CIPHER_SUITE_TKIP     ZT_BIT(3)
#define ZT_CIPHER_SUITE_CCMP     ZT_BIT(4)

enum NETWORK_TYPE
{
    WIRELESS_INVALID = 0,
    WIRELESS_11B = ZT_BIT(0),
    WIRELESS_11G = ZT_BIT(1),
    WIRELESS_11_24N = ZT_BIT(3),
    WIRELESS_AUTO = ZT_BIT(5),

    WIRELESS_11BG = (WIRELESS_11B | WIRELESS_11G),
    WIRELESS_11G_24N = (WIRELESS_11G | WIRELESS_11_24N),
    WIRELESS_11B_24N = (WIRELESS_11B | WIRELESS_11_24N),
    WIRELESS_11BG_24N = (WIRELESS_11B | WIRELESS_11G | WIRELESS_11_24N),

    WIRELESS_MODE_MAX = (WIRELESS_11B | WIRELESS_11G | WIRELESS_11_24N),
};


#define ZT_80211_CCK_RATE_1MB              0x02
#define ZT_80211_CCK_RATE_2MB              0x04
#define ZT_80211_CCK_RATE_5MB              0x0B
#define ZT_80211_CCK_RATE_11MB             0x16
#define ZT_80211_OFDM_RATE_LEN             8
#define ZT_80211_OFDM_RATE_6MB             0x0C
#define ZT_80211_OFDM_RATE_9MB             0x12
#define ZT_80211_OFDM_RATE_12MB            0x18
#define ZT_80211_OFDM_RATE_18MB            0x24
#define ZT_80211_OFDM_RATE_24MB            0x30
#define ZT_80211_OFDM_RATE_36MB            0x48
#define ZT_80211_OFDM_RATE_48MB            0x60
#define ZT_80211_OFDM_RATE_54MB            0x6C
#define IEEE80211_BASIC_RATE_MASK           0x80

#define ZT_80211_CCK_RATE_1MB_MASK         ZT_BIT(0)
#define ZT_80211_CCK_RATE_2MB_MASK         ZT_BIT(1)
#define ZT_80211_CCK_RATE_5MB_MASK         ZT_BIT(2)
#define ZT_80211_CCK_RATE_11MB_MASK        ZT_BIT(3)
#define ZT_80211_OFDM_RATE_6MB_MASK        ZT_BIT(4)
#define ZT_80211_OFDM_RATE_9MB_MASK        ZT_BIT(5)
#define ZT_80211_OFDM_RATE_12MB_MASK       ZT_BIT(6)
#define ZT_80211_OFDM_RATE_18MB_MASK       ZT_BIT(7)
#define ZT_80211_OFDM_RATE_24MB_MASK       ZT_BIT(8)
#define ZT_80211_OFDM_RATE_36MB_MASK       ZT_BIT(9)
#define ZT_80211_OFDM_RATE_48MB_MASK       ZT_BIT(10)
#define ZT_80211_OFDM_RATE_54MB_MASK       ZT_BIT(11)


#define ZT_80211_BASIC_RATE_NUM            8

#define zt_80211_is_mcast_addr(a)   ((a)[0] != 0xff && (0x01 & (a)[0]))
#define zt_80211_is_bcast_addr(a) \
    (((a)[0] & (a)[1] & (a)[2] & (a)[3] & (a)[4] & (a)[5]) == 0xff)
#define zt_80211_is_zero_addr(a) \
    (!((a)[0] | (a)[1] | (a)[2] | (a)[3] | (a)[4] | (a)[5]))

#define zt_80211_is_valid_bssid(a) \
    (!zt_80211_is_mcast_addr(a) && \
     !zt_80211_is_bcast_addr(a) && \
     !zt_80211_is_zero_addr(a))

#define zt_80211_is_valid_unicast(a) \
    (!zt_80211_is_mcast_addr(a) && \
     !zt_80211_is_bcast_addr(a) && \
     !zt_80211_is_zero_addr(a))

#define zt_80211_is_same_addr(a1, a2) (!zt_memcmp(a1, a2, sizeof(zt_80211_addr_t)))

zt_s32 zt_ch_2_freq(zt_s32 ch);
zt_s32 freq_2_ch(zt_s32 freq);

#define ZT_MAX_WPA_IE_LEN         (256)
#define ZT_MAX_WPS_IE_LEN         (512)
#define ZT_RSN_HD_LEN            4

#if defined(_WIN32) || defined(_WIN64)
#pragma pack(1)
#endif

struct zt_ethhdr
{
    zt_u8  dest[ZT_80211_MAC_ADDR_LEN];
    zt_u8  src[ZT_80211_MAC_ADDR_LEN];
    zt_u16 type;
} zt_packed;


struct wl_ieee80211_hdr
{
    zt_u16 frame_ctl;
    zt_u16 duration_id;
    zt_u8 addr1[ZT_80211_MAC_ADDR_LEN];
    zt_u8 addr2[ZT_80211_MAC_ADDR_LEN];
    zt_u8 addr3[ZT_80211_MAC_ADDR_LEN];
    zt_u16 seq_ctl;
    zt_u8 addr4[ZT_80211_MAC_ADDR_LEN];
} zt_packed;


struct wl_ieee80211_hdr_3addr
{
    zt_u16 frame_ctl;
    zt_u16 duration_id;
    zt_u8 addr1[ZT_80211_MAC_ADDR_LEN];
    zt_u8 addr2[ZT_80211_MAC_ADDR_LEN];
    zt_u8 addr3[ZT_80211_MAC_ADDR_LEN];
    zt_u16 seq_ctl;
} zt_packed;

struct wl_ieee80211_ht_oper_info
{
    zt_u8 control_chan;
    zt_u8 ht_param;
    zt_u16 operation_mode;
    zt_u16 stbc_param;
    zt_u8 basic_set[16];
} zt_packed;

struct ADDBA_request
{
    zt_u8 dialog_token;
    zt_u16 BA_para_set;
    zt_u16 BA_timeout_value;
    zt_u16 ba_starting_seqctrl;
} zt_packed;

typedef struct
{
    zt_80211_frame_ctrl_t frame_control;
    zt_80211_duration_t duration;
    zt_80211_addr_t da;
    zt_80211_addr_t sa;
    zt_80211_bssid_t bssid;
    zt_80211_seq_ctrl_t seq_ctrl;
    union
    {
        /* beacon */
        struct beacon_ie
        {
            zt_80211_mgmt_timestamp_t timestamp;
            zt_80211_mgmt_beacon_interval_t intv;
            zt_80211_mgmt_capab_t capab;
            /* followed by some of SSID, Supported rates,
             * FH Params, DS Params, CF Params, IBSS Params, TIM */
            zt_u8 variable[];
        } zt_packed beacon, probe_resp;
        /* probe */
        struct
        {
            /* only variable items: SSID, Supported rates */
            zt_u8 variable[1];
        } zt_packed probe_req;
        /* auth */
        struct auth_ie
        {
            zt_u16 auth_alg;
            zt_u16 auth_transaction;
            zt_u16 status_code;
            /* possibly followed Challenge text */
            zt_u8 variable[];
        } zt_packed auth;
        struct auth_seq3_ie
        {
            zt_u32 iv;
            zt_u16 auth_alg;
            zt_u16 auth_transaction;
            zt_u16 status_code;
            /* possibly followed Challenge text */
            zt_u8 variable[ZT_OFFSETOF(zt_80211_mgmt_ie_t,
                                       data) + ZT_80211_AUTH_CHALLENGE_LEN];
            zt_u32 icv;
        } zt_packed auth_seq3;
        struct deauth_ie
        {
            zt_u16 reason_code;
        } zt_packed deauth;
        /* assoc */
        struct disassoc_ie
        {
            zt_u16 reason_code;
        } zt_packed disassoc;
        struct assoc_req_ie
        {
            zt_u16 capab_info;
            zt_u16 listen_interval;
            /* followed by SSID and Supported rates */
            zt_u8 variable[];
        } zt_packed assoc_req;
        struct
        {
            zt_u16 capab_info;
            zt_u16 status_code;
            zt_u16 aid;
            /* followed by Supported rates */
            zt_u8 variable[];
        } zt_packed assoc_resp;
        struct
        {
            zt_u16 capab_info;
            zt_u16 listen_interval;
            zt_80211_addr_t current_ap;
            /* followed by SSID and Supported rates */
            zt_u8 variable[];
        } zt_packed reassoc_req;
        struct
        {
            zt_u16 capab_info;
            zt_u16 status_code;
            zt_u16 aid;
            /* followed by Supported rates */
            zt_u8 variable[];
        } zt_packed reassoc_resp;
        struct action_ie
        {
            zt_u8 action_category;
            zt_u8 action_field;
            zt_u8 variable[];
        } zt_packed action;
    };
} zt_packed zt_80211_mgmt_t;

/*
 * data frame
 */
typedef struct
{
    zt_80211_frame_ctrl_t frame_control;
    zt_80211_duration_t duration;
    zt_80211_addr_t addr1;
    zt_80211_addr_t addr2;
    zt_80211_bssid_t addr3;
    zt_80211_seq_ctrl_t seq_ctrl;
    zt_u8 body[0];
} zt_packed zt_80211_data_t;


/* snap header */
typedef struct zt_80211_snap_header
{
    zt_u8 dsap;
    zt_u8 ssap;
    zt_u8 ctrl;
    zt_u8 oui[3];
} zt_packed zt_80211_snap_header_t;
#define ZT_80211_SNAP_HDR_SIZE  sizeof(zt_80211_snap_header_t)

/**
 * zt_80211_mgmt_ht_operation_t - DSSS Parameter Set element
 *
 * This structure is the "DSSS Parameter" as
 * described in 802.11n-2016 9.4.2.4
 */
typedef struct
{
    zt_u8 current_channel;
} zt_packed zt_80211_mgmt_dsss_parameter_t;

typedef struct
{
    zt_80211_frame_ctrl_t frame_control;
    zt_u16 duration_id;
    zt_80211_addr_t addr1;
    zt_80211_addr_t addr2;
    zt_80211_addr_t addr3;
    zt_80211_seq_ctrl_t seq_ctrl;
    zt_80211_addr_t addr4;
} zt_packed zt_80211_hdr_t;

typedef struct
{
    zt_80211_frame_ctrl_t frame_control;
    zt_u16 duration_id;
    zt_80211_addr_t addr1;
    zt_80211_addr_t addr2;
    zt_80211_addr_t addr3;
    zt_80211_seq_ctrl_t seq_ctrl;
} zt_packed zt_80211_hdr_3addr_t;

typedef zt_u16 zt_80211_aid;
typedef struct
{
    zt_80211_frame_ctrl_t frame_control;
    zt_80211_duration_t duration_id;
    zt_80211_addr_t addr1;
    zt_80211_addr_t addr2;
    zt_80211_addr_t addr3;
    zt_80211_seq_ctrl_t seq_ctrl;
    union
    {
        struct
        {
            zt_80211_addr_t addr4;
            zt_u16 a4_qos_ctrl;
        };
        zt_u16 qos_ctrl;
    };
} zt_packed zt_80211_qos_hdr_t;


/* Channel switch timing */
typedef struct
{
    zt_u16 switch_time;
    zt_u16 switch_timeout;
} zt_packed zt_80211_ch_switch_timing_t;


/* Management MIC information element (IEEE 802.11w) */
typedef struct
{
    zt_u8 element_id;
    zt_u8 length;
    zt_u16 key_id;
    zt_u8 sequence_number[6];
    zt_u8 mic[8];
} zt_packed zt_80211_mmie_t;

/* Management MIC information element (IEEE 802.11w) for GMAC and CMAC-256 */
typedef struct
{
    zt_u8 element_id;
    zt_u8 length;
    zt_u16 key_id;
    zt_u8 sequence_number[6];
    zt_u8 mic[16];
} zt_packed zt_80211_mmie_16_t;

typedef struct
{
    zt_u8 element_id;
    zt_u8 len;
    zt_u8 oui[3];
    zt_u8 oui_type;
} zt_packed zt_80211_vendor_ie_t;

typedef struct
{
    zt_u8 aci_aifsn; /* AIFSN, ACM, ACI */
    zt_u8 cw; /* ECWmin, ECWmax (CW = 2^ECW - 1) */
    zt_u16 txop_limit;
} zt_packed zt_80211_wmm_ac_param_t;

typedef struct
{
    zt_u8 element_id; /* Element ID: 221 (0xdd); */
    zt_u8 len; /* Length: 24 */
    /* required fields for WMM version 1 */
    zt_u8 oui[3]; /* 00:50:f2 */
    zt_u8 oui_type; /* 2 */
    zt_u8 oui_subtype; /* 0 */
    zt_u8 version; /* 1 for WMM version 1.0 */
    zt_u8 qos_info; /* AP/STA specific QoS info */
    zt_u8 reserved; /* 0 */
    /* AC_BE, AC_BK, AC_VI, AC_VO */
    zt_80211_wmm_ac_param_t ac[4];
} zt_packed zt_80211_wmm_param_ie_t;

typedef struct
{
    zt_u8 element_id; /* Element ID: 221 (0xdd); */
    zt_u8 len; /* Length: 24 */
    zt_u8 oui[3]; /* 50:6F:9A */
    zt_u8 oui_type; /* Identifying the type or version of P2P IE. Setting to 0x09 indicates Wi-Fi Alliance P2P v1.0. */
    zt_u8 p2p_attrs[0];/*One of more P2P attributes appear in the P2P IE.*/
} zt_packed zt_80211_p2p_param_ie_t;

/* mcs element */
/**
 * zt_80211_mgmt_ht_cap_mcs_info_t - MCS information
 * @rx_mask: RX mask
 * @rx_highest: highest supported RX rate. If set represents
 *  the highest supported RX data rate in units of 1 Mbps.
 *  If this field is 0 this value should not be used to
 *  consider the highest RX data rate supported.
 * @tx_params: TX parameters
 */
#define ZT_80211_MGMT_HT_MCS_MASK_LEN                   10
typedef struct
{
    zt_u8 rx_mask[ZT_80211_MGMT_HT_MCS_MASK_LEN];
    zt_u16 rx_highest;
    zt_u8 tx_params;
    zt_u8 reserved[3];
} zt_packed zt_80211_mgmt_ht_cap_mcs_info_t;

/**
 * struct ieee80211_ht_cap - HT capabilities
 *
 * This structure is the "HT capabilities element" as
 * described in 802.11n D5.0 7.3.2.57
 */
typedef struct
{
    zt_u16 cap_info;
    zt_u8 ampdu_params_info;
    /* 16 bytes MCS information */
    union
    {
        zt_u8 supp_mcs_set[ZT_MCS_NUM];
        zt_80211_mgmt_ht_cap_mcs_info_t mcs_info;
    };
    zt_u16 extended_ht_cap_info;
    zt_u32 tx_BF_cap_info;
    zt_u8 antenna_selection_info;
} zt_packed zt_80211_mgmt_ht_cap_t;

/**
 * struct ieee80211_ht_operation - HT operation IE
 *
 * This structure is the "HT operation element" as
 * described in 802.11n-2009 7.3.2.57
 */
typedef struct
{
    zt_u8 primary_chan;
    zt_u8 ht_param;
    zt_u16 operation_mode;
    zt_u16 stbc_param;
    zt_u8 basic_set[16];
} zt_packed zt_80211_mgmt_ht_operation_t;

#if defined(_WIN32) || defined(_WIN64)
#pragma pack()
#endif

#define _80211_hdr(hdr)     ((zt_80211_hdr_t *)(hdr))
#define _80211_qos_hdr(hdr) ((zt_80211_qos_hdr_t *)(hdr))

#define hdr_fctrl(hdr)          (_80211_hdr(hdr)->frame_control)
#define hdr_fctrl_get(hdr, msk) (zt_le16_to_cpu(hdr_fctrl(hdr)) & (msk))
#define hdr_fctrl_set(hdr, msk, val) \
    hdr_fctrl(hdr) = (zt_cpu_to_le16((hdr_fctrl(hdr) & ~(msk)) | (val)))

static zt_inline
zt_80211_ftype_t zt_80211_hdr_ftype_get(void *hdr)
{
    return hdr_fctrl_get(hdr, ZT_80211_FCTL_FTYPE);
}
static zt_inline
zt_80211_stype_t zt_80211_hdr_stype_get(void *hdr)
{
    return hdr_fctrl_get(hdr, ZT_80211_FCTL_STYPE);
}
static zt_inline
zt_80211_frame_e zt_80211_hdr_type_get(void *hdr)
{
    return (zt_80211_frame_e)hdr_fctrl_get(hdr, ZT_80211_FCTL_STYPE |
                                           ZT_80211_FCTL_FTYPE);
}
static zt_inline
void zt_80211_hdr_type_set(void *hdr, zt_80211_frame_e type)
{
    hdr_fctrl_set(hdr, ZT_80211_FCTL_STYPE | ZT_80211_FCTL_FTYPE, type);
}

#define zt_80211_hdr_ds_get(hdr) \
    hdr_fctrl_get(hdr, ZT_80211_FCTL_FROMDS | ZT_80211_FCTL_TODS)
#define zt_80211_hdr_ds_set(hdr, ds) \
    hdr_fctrl_set(hdr, ZT_80211_FCTL_FROMDS | ZT_80211_FCTL_TODS, ds)

#define zt_80211_hdr_moreflags_get(hdr) \
    hdr_fctrl_get(hdr, ZT_80211_FCTL_MOREFRAGS)

#define zt_80211_hdr_retry_get(hdr) hdr_fctrl_get(hdr, ZT_80211_FCTL_RETRY)

#define zt_80211_hdr_pm_get(hdr)        hdr_fctrl_get(hdr, ZT_80211_FCTL_PM)
#define zt_80211_hdr_pm_set(hdr, en) \
    hdr_fctrl_set(hdr, ZT_80211_FCTL_PM, en ? ZT_80211_FCTL_PM : 0)

#define zt_80211_hdr_moredata_get(hdr)  hdr_fctrl_get(hdr, ZT_80211_FCTL_MOREDATA)
#define zt_80211_hdr_moredata_set(hdr, b) \
    hdr_fctrl_set(hdr, ZT_80211_FCTL_MOREDATA, b ? ZT_80211_FCTL_MOREDATA : 0)

#define zt_80211_hdr_protected_get(hdr) \
    hdr_fctrl_get(hdr, ZT_80211_FCTL_PROTECTED)
#define zt_80211_hdr_protected_set(hdr, en) \
    hdr_fctrl_set(hdr, ZT_80211_FCTL_PROTECTED, en ? ZT_80211_FCTL_PROTECTED : 0)

#define zt_80211_hdr_order_get(hdr) hdr_fctrl_get(hdr, ZT_80211_FCTL_ORDER)

#define zt_80211_hdr_addr1(hdr) (_80211_hdr(hdr)->addr1)
#define zt_80211_hdr_addr2(hdr) (_80211_hdr(hdr)->addr2)
#define zt_80211_hdr_addr3(hdr) (_80211_hdr(hdr)->addr3)
#define zt_80211_hdr_addr4(hdr) (_80211_hdr(hdr)->addr4)

#define hdr_sctrl(hdr)          (_80211_hdr(hdr)->seq_ctrl)
#define hdr_sctrl_get(hdr, msk) (zt_le16_to_cpu(hdr_sctrl(hdr)) & (msk))

#define zt_80211_hdr_sctrl_get(hdr) zt_le16_to_cpu(hdr_sctrl(hdr))
#define zt_80211_hdr_sctrl_num_get(hdr) \
    ((zt_le16_to_cpu(hdr_sctrl(hdr)) & ZT_80211_SCTL_SEQ_MASK) >> ZT_80211_SCTL_SEQ_SHIFT)
#define zt_80211_hdr_sctrl_frag_num_get(hdr) \
    ((zt_le16_to_cpu(hdr_sctrl(hdr)) & ZT_80211_SCTL_FRAG_MASK) >> ZT_80211_SCTL_FRAG_SHIFT)

#define zt_80211_hdr_has_a4(hdr) \
    (zt_80211_hdr_ds_get(hdr) == (ZT_80211_FCTL_FROMDS | ZT_80211_FCTL_TODS))

#define hdr_qctrl(hdr) \
    (zt_80211_hdr_has_a4(hdr) ? \
     _80211_qos_hdr(hdr)->a4_qos_ctrl : _80211_qos_hdr(hdr)->qos_ctrl)

#define zt_80211_hdr_qos_tid_get(hdr) \
    (zt_le16_to_cpu(hdr_qctrl(hdr)) & ZT_80211_QOS_CTL_TID_MASK)
#define zt_80211_hdr_qos_ack_policy_get(hdr) \
    ((zt_le16_to_cpu(hdr_qctrl(hdr)) & ZT_80211_QOS_CTL_ACK_POLICY_MASK) >> ZT_80211_QOS_CTL_ACK_POLICY_SHIFT)
#define zt_80211_hdr_qos_amsdu_get(hdr) \
    ((zt_le16_to_cpu(hdr_qctrl(hdr)) & ZT_80211_QOS_CTL_A_MSDU_PRESENT) >> ZT_80211_QOS_CTL_A_MSDU_PRESENT_SHIFT)

#endif

