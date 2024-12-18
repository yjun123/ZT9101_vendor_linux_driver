/*
 * p2p_wowlan.h
 *
 * used for p2p
 *
 * Author: kanglin
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
#ifndef __P2P_WOWLAN_H__
#define __P2P_WOWLAN_H__

enum P2P_WOWLAN_RECV_FRAME_TYPE
{
    P2P_WOWLAN_RECV_NEGO_REQ = 0,
    P2P_WOWLAN_RECV_INVITE_REQ = 1,
    P2P_WOWLAN_RECV_PROVISION_REQ = 2,
};

struct p2p_wowlan_info
{
    zt_u8 is_trigger;
    enum P2P_WOWLAN_RECV_FRAME_TYPE wowlan_recv_frame_type;
    zt_u8 wowlan_peer_addr[ZT_80211_MAC_ADDR_LEN];
    zt_u16 wowlan_peer_wpsconfig;
    zt_u8 wowlan_peer_is_persistent;
    zt_u8 wowlan_peer_invitation_type;
};

#endif
