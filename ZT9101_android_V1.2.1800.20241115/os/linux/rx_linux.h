/*
 * rx_linux.h
 *
 * used for frame xmit for linux
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
#ifndef __RX_LINUX__
#define __RX_LINUX__
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,0,0))
#define netif_rx_ni(x) netif_rx(x)
#endif
void mpdu_process(struct net_device *ndev, zt_u8 *buf, zt_u16 buf_size);

#endif
