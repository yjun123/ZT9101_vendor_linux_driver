/*
 * usb.c
 *
 * used for .....
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


#include <linux/usb.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/skbuff.h>

#include "common.h"
#include "usb.h"
#include "hif.h"
#include "power.h"

#define USB_DBG(fmt, ...)       LOG_D("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)
#define USB_INFO(fmt, ...)      LOG_I("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)
#define USB_WARN(fmt, ...)      LOG_W("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)
#define USB_ERROR(fmt, ...)     LOG_E("[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__)

#define MAX_USBCTRL_VENDORREQ_TIMES 3
#define MAX_CONTINUAL_IO_ERR 4
#define N_BYTE_ALIGMENT(value, aligment) ((aligment == 1) ? (value) : (((value + aligment - 1) / aligment) * aligment))

static zt_s32 zt_usb_read_port(hif_node_st *hif_node, zt_u32 addr, zt_u8 *rdata,
                               zt_u32 rlen);
static zt_s32 zt_usb_read(struct hif_node_ *node, zt_u8 flag,
                          zt_u32 addr, zt_s8 *data, zt_s32 datalen);

static zt_s32 usb_ctrl_read(struct usb_device *pusb_dev, zt_s8 *ctlBuf,
                            zt_u32 addr, zt_s8 *data, zt_s32 datalen);
static zt_s32 usb_ctrl_write(struct usb_device *pusb_dev, zt_s8 *ctlBuf,
                             zt_u32 addr, zt_s8 *data, zt_s32 datalen);

static inline zt_s32 endpoint_is_int_out(const struct
        usb_endpoint_descriptor *epd)
{
    return (usb_endpoint_is_int_out(epd));
}

static inline zt_s32 endpoint_is_int_in(const struct
                                        usb_endpoint_descriptor *epd)
{
    return (usb_endpoint_is_bulk_out(epd));
}

static inline zt_s32 endpoint_is_bulk_in(const struct
        usb_endpoint_descriptor *epd)
{
    return (usb_endpoint_is_bulk_in(epd));
}

static inline zt_s32 endpoint_is_bulk_out(const struct
        usb_endpoint_descriptor *epd)
{
    return (usb_endpoint_is_bulk_out(epd));
}

static inline zt_s32 endpoint_num(const struct usb_endpoint_descriptor *epd)
{
    return usb_endpoint_num(epd);
}

zt_s32 zt_endpoint_init(struct usb_interface *pusb_intf,
                        hif_usb_mngt *pusb_mngt)
{
    zt_s32 status                                      = 0;
    struct usb_host_config *phost_cfg               = NULL;
    struct usb_config_descriptor *pcfg_desc         = NULL;
    struct usb_host_interface *phost_itface         = NULL;
    struct usb_interface_descriptor *pitface_desc   = NULL;
    struct usb_host_endpoint *phost_endpt           = NULL;
    struct usb_endpoint_descriptor *pendpt_desc     = NULL;
    struct usb_device *pusb_dev                     = NULL;
    zt_u8 i                                         = 0;

    pusb_mngt->pusb_intf = pusb_intf;
    pusb_mngt->pusb_dev = interface_to_usbdev(pusb_intf);

    pusb_mngt->n_in_pipes = 0;
    pusb_mngt->n_out_pipes = 0;
    pusb_dev = pusb_mngt->pusb_dev;

    phost_cfg = pusb_dev->actconfig;
    pcfg_desc = &phost_cfg->desc;

    phost_itface = &pusb_intf->altsetting[0];
    pitface_desc = &phost_itface->desc;

    pusb_mngt->n_interfaces = pcfg_desc->bNumInterfaces;
    pusb_mngt->intface_num = pitface_desc->bInterfaceNumber;
    pusb_mngt->n_endpoints = pitface_desc->bNumEndpoints;

    for (i = 0; i < pusb_mngt->n_endpoints; i++)
    {
        phost_endpt = &phost_itface->endpoint[i];
        if (phost_endpt)
        {
            pendpt_desc = &phost_endpt->desc;

            if (endpoint_is_bulk_in(pendpt_desc))
            {
                USB_DBG("EP_IN  = num[%d]", endpoint_num(pendpt_desc));

                pusb_mngt->in_endp_addr[pusb_mngt->n_in_pipes] = endpoint_num(pendpt_desc);
                pusb_mngt->i_bulk_pipe_sz[pusb_mngt->n_in_pipes] =  pendpt_desc->bLength;
                pusb_mngt->n_in_pipes++;
            }
            else if (endpoint_is_bulk_out(pendpt_desc))
            {
                USB_DBG("EP_OUT = num[%d]", endpoint_num(pendpt_desc));
                pusb_mngt->out_endp_addr[pusb_mngt->n_out_pipes] = endpoint_num(pendpt_desc);
                pusb_mngt->o_bulk_pipe_sz[pusb_mngt->n_out_pipes] = pendpt_desc->wMaxPacketSize;
                pusb_mngt->n_out_pipes++;
            }
        }
    }

    switch (pusb_dev->speed)
    {
        case USB_SPEED_LOW:
            USB_DBG("USB_SPEED_LOW\n");
            pusb_mngt->usb_speed = ZT_USB_SPEED_1_1;
            break;
        case USB_SPEED_FULL:
            USB_DBG("USB_SPEED_FULL\n");
            pusb_mngt->usb_speed = ZT_USB_SPEED_1_1;
            break;
        case USB_SPEED_HIGH:
            USB_DBG("USB_SPEED_HIGH\n");
            pusb_mngt->usb_speed = ZT_USB_SPEED_2;
            break;
        default:
            USB_WARN("USB_SPEED_UNKNOWN(%x)\n", pusb_dev->speed);
            pusb_mngt->usb_speed = ZT_USB_SPEED_UNKNOWN;
            break;
    }

    if (pusb_mngt->usb_speed == ZT_USB_SPEED_UNKNOWN)
    {
        return -1;
    }

    usb_get_dev(pusb_dev);

    return status;
}

zt_s32 zt_endpoint_deinit(struct usb_interface *pusb_intf)
{
    hif_node_st  *hif_node = usb_get_intfdata(pusb_intf);
    hif_usb_mngt *pusb_mngt = &hif_node->u.usb;

    usb_set_intfdata(pusb_intf, NULL);
    if (pusb_mngt)
    {
        if ((pusb_mngt->n_interfaces != 2 && pusb_mngt->n_interfaces != 3)
                || (pusb_mngt->intface_num == 1))
        {
            if (interface_to_usbdev(pusb_intf)->state != USB_STATE_NOTATTACHED)
            {
                LOG_W("usb attached..., try to reset usb device\n");
                usb_reset_device(interface_to_usbdev(pusb_intf));
            }
        }
    }

    usb_put_dev(interface_to_usbdev(pusb_intf));

    return 0;
}


zt_s32 zt_usb_init(struct hif_node_  *hif_node)
{
    return 0;
}

zt_s32 zt_usb_deinit(struct hif_node_  *hif_node)
{
    USB_DBG("usb_deinit");
    return 0;
}


static inline zt_u32 zt_usb_ffaddr2pipe(hif_usb_mngt *usb, zt_u32 addr)
{
    zt_u32 pipe   = 0;
    zt_u8 ep_num        = 0;

    if (addr == READ_QUEUE_INX)
    {
        return usb_rcvbulkpipe(usb->pusb_dev, usb->in_endp_addr[0]);
    }

    switch (addr)
    {
        case BE_QUEUE_INX:
        case BK_QUEUE_INX:
        case VI_QUEUE_INX:
        case VO_QUEUE_INX:
            ep_num = usb->out_endp_addr[1];
            break;
        case CMD_QUEUE_INX:  // hjy
            if (usb->n_out_pipes > 3)
            {
                ep_num = usb->out_endp_addr[3];
            }
            else
            {
                ep_num = usb->out_endp_addr[1];
            }
            break;
        case MGT_QUEUE_INX:
        case BCN_QUEUE_INX:
        case HIGH_QUEUE_INX:
            ep_num = usb->out_endp_addr[0];
            break;

        default:
            ep_num = usb->out_endp_addr[1];
            break;
    }

    pipe = usb_sndbulkpipe(usb->pusb_dev, ep_num);

    if ((addr != BE_QUEUE_INX) && (addr != MGT_QUEUE_INX))
    {
        //USB_DBG("USB write addr:%d  ep_num:%d", addr, ep_num);
    }

    return pipe;

}

zt_bool zt_free_tx_need_wake(hif_node_st *hif_node);
static inline void usb_write_port_complete(struct urb *purb)
{
    zt_s32 ret                         = 0;
    data_queue_node_st   *qnode     = (data_queue_node_st *)purb->context;
    hif_node_st   *hnode            = (hif_node_st *)qnode->hif_node;
    data_queue_mngt_st *dqm         = &hnode->trx_pipe;
    zt_s32 i                        = 0;

    if (0 == purb->status) //usb work well
    {
        qnode->state = TX_STATE_COMPETE;
        if (qnode->tx_callback_func)
        {
            ret = qnode->tx_callback_func(qnode->tx_info, qnode->param);
        }

        //qnode->u.purb = NULL;
        zt_data_queue_insert(&dqm->free_tx_queue, qnode);
    }
    else
    {
        LOG_I("[%s]:usb write work bad, urb->status:%d", __func__, purb->status);
        zt_data_queue_insert(&dqm->free_tx_queue, qnode);
        switch (purb->status)
        {
            case -EINVAL:
                USB_INFO("[%s] EINVAL", __func__);;
                break;
            case -EPIPE:
                USB_INFO("[%s] EPIPE", __func__);;
                break;
            case -ENODEV:
                USB_INFO("[%s] ENODEV", __func__);;
                break;
            case -ESHUTDOWN:
                USB_INFO("[%s] ESHUTDOWN", __func__);;
                break;

            case -ENOENT:
                USB_INFO("[%s] ENOENT", __func__);
                break;
            case -EPROTO:
                USB_INFO("[%s] EPROTO", __func__);;
                break;
            case -EILSEQ:
                USB_INFO("[%s] EILSEQ", __func__);;
                break;
            case -ETIME:
                USB_INFO("[%s] ETIME", __func__);;
                break;
            case -ECOMM:
                USB_INFO("[%s] ECOMM", __func__);;
                break;
            case -EOVERFLOW:
                USB_INFO("[%s] EOVERFLOW", __func__);;
                break;
            case -EINPROGRESS:
                USB_INFO("ERROR: URB IS IN PROGRESS!");
                break;
            default:
                USB_INFO("[%s] default", __func__);;
                break;
        }
    }


    if (hnode->nic_number == 2)
    {
        if (zt_free_tx_need_wake(hnode))
        {
            for (i = 0; i < hnode->nic_number; i++)
            {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
                netif_tx_wake_all_queues(hnode->nic_info[i]->ndev);
#else
                netif_wake_queue(hnode->nic_info[i]->ndev);
#endif
            }
            dqm->is_free_tx_stopped = zt_false;
        }
    }

    // if (purb)
    // {
    //     usb_free_urb(purb);
    // }

}

static inline void usb_read_port_complete(struct urb *purb)
{
    data_queue_node_st *qnode   = (data_queue_node_st *)purb->context;
    hif_node_st *hnode          = (hif_node_st *)qnode->hif_node;
    struct sk_buff *skb         = NULL;
    zt_s32 ret                     = -1;
    hif_usb_mngt *pusb_mngt     = &(hnode->u.usb);
    skb = (struct sk_buff *)qnode->buff;

    if (0 == purb->status) //usb work well
    {
#define MIN_RXD_SIZE      16
        //USB_DBG("usb recv length is %d", purb->actual_length);
        if (purb->actual_length < MIN_RXD_SIZE)
        {
            skb_trim(skb, 0);

            zt_usb_read_port(hnode, READ_QUEUE_INX, (zt_u8 *)qnode,
                             ZT_MAX_RECV_BUFF_LEN_USB);
        }
        else//this is normal way, the data has been read to qnode->buffer
        {
            pusb_mngt->blk_continue_io_error = 0;
            qnode->real_size = purb->actual_length;
            skb_put(skb, purb->actual_length);
            ZT_ASSERT(qnode->buff);

            ret = zt_rx_data_len_check(hnode->nic_info[0], skb->data, skb->len);
            if (ret == -1)
            {
                if (skb)
                {
                    skb_trim(skb, 0);
                    zt_usb_read_port(hnode, READ_QUEUE_INX, (zt_u8 *)qnode,
                                     ZT_MAX_RECV_BUFF_LEN_USB);
                }
            }
            else
            {
                if (zt_rx_data_type(skb->data) == ZT_PKT_TYPE_FRAME)
                {
                    skb_queue_tail(&hnode->trx_pipe.rx_queue, skb);

                    if (skb_queue_len(&hnode->trx_pipe.rx_queue) <= 1)
                    {
                        zt_tasklet_hi_sched(&hnode->trx_pipe.recv_task);
                    }

                    qnode->buff = NULL;
                    zt_usb_read_port(hnode, READ_QUEUE_INX, (zt_u8 *)qnode,
                                     ZT_MAX_RECV_BUFF_LEN_USB);
                }
                else
                {
                    if (zt_rx_cmd_check(skb->data, skb->len) == 0)
                    {
                        switch (zt_rx_data_type(skb->data))
                        {
                            case ZT_PKT_TYPE_CMD:
                                zt_hif_bulk_cmd_post(hnode, skb->data, skb->len);
                                break;

                            case ZT_PKT_TYPE_FW:
                                zt_hif_bulk_fw_post(hnode, skb->data, skb->len);
                                break;

                            default:
                                USB_WARN("recv rxd type error");
                                break;
                        }
                    }

                    if (skb)
                    {
                        skb_trim(skb, 0);
                        zt_usb_read_port(hnode, READ_QUEUE_INX, (zt_u8 *)qnode,
                                         ZT_MAX_RECV_BUFF_LEN_USB);
                    }
                }
            }
        }
    }
    else//usb work bad
    {
        skb_queue_tail(&hnode->trx_pipe.free_rx_queue_skb, skb);
        zt_data_queue_insert(&hnode->trx_pipe.free_rx_queue, qnode);
        //LOG_I("[%s]:usb read work bad, urb->status:%d  node_id:%d", __func__, purb->status, qnode->node_id);
        switch (purb->status)
        {
            case -EINVAL:
            {
                USB_WARN("[%s] EINVAL", __func__);
            }
            case -EPIPE:
            {
                USB_WARN("[%s] EPIPE", __func__);
            }
            case -ENODEV:
            {
                USB_WARN("[%s] ENODEV", __func__);
            }
            case -ESHUTDOWN:
            {
                USB_WARN("[%s] ESHUTDOWN", __func__);
            }
            case -ENOENT:
            {
                USB_WARN("[%s] ENOENT", __func__);
                hnode->dev_removed = zt_true;
                break;
            }
            case -EPROTO:
            {
                USB_WARN("[%s] EPROTO", __func__);
            }
            case -EILSEQ:
            {
                USB_WARN("[%s] EILSEQ", __func__);
            }
            case -ETIME:
            {
                USB_WARN("[%s] ETIME", __func__);
            }
            case -ECOMM:
            {
                USB_WARN("[%s] ECOMM", __func__);
            }
            case -EOVERFLOW:
            {
                USB_WARN("[%s] EOVERFLOW", __func__);
                break;
            }
            case -EINPROGRESS:
            {
                USB_WARN("ERROR: URB IS IN PROGRESS!");
                break;
            }
            default:
            {
                USB_WARN("Unknown status:%d", purb->status);
                break;
            }
        }
        purb->status = 0;
    }
}

static zt_s32 zt_usb_read_port(hif_node_st *hif_node, zt_u32 addr, zt_u8 *rdata,
                               zt_u32 rlen)
{
    zt_s32 ret                         = 0;
    data_queue_node_st *qnode      = (data_queue_node_st *)rdata;
    zt_u32 pipe               = 0;
    struct usb_device *pusbd        = hif_node->u.usb.pusb_dev;
    struct urb *purb               = qnode->hif_dev;
    struct sk_buff *pskb            = NULL;
    data_queue_mngt_st *trxq        = (data_queue_mngt_st *)&hif_node->trx_pipe;
    //LOG_D("qnode->node_id:%d", qnode->node_id);

    if ((hm_get_mod_removed() == zt_false) && (hif_node->dev_removed == zt_true))
    {
        return ZT_RETURN_FAIL;
    }

    trxq->rx_queue_cnt++;
    qnode->state = TX_STATE_FLOW_CTL;
    if (qnode->buff == NULL)
    {
        qnode->buff = (zt_u8 *)skb_dequeue(&hif_node->trx_pipe.free_rx_queue_skb);
    }

    if (NULL == qnode->buff)
    {
        if (hif_node->trx_pipe.alloc_cnt < HIF_MAX_ALLOC_CNT)
        {
            LOG_W("[%s] zt_alloc_skb again", __func__);
            hif_node->trx_pipe.alloc_cnt++;
            zt_hif_queue_alloc_skb(&hif_node->trx_pipe.free_rx_queue_skb,
                                   hif_node->hif_type);
        }
        else
        {
            LOG_W("[%s] zt_alloc_skb skip", __func__);
        }

        zt_data_queue_insert(&hif_node->trx_pipe.free_rx_queue, qnode);
        return ZT_RETURN_OK;
    }

    pipe    = zt_usb_ffaddr2pipe(&hif_node->u.usb, addr);
    pskb = (struct sk_buff *)qnode->buff;
    usb_fill_bulk_urb(purb, pusbd, pipe, pskb->data,  ZT_MAX_RECV_BUFF_LEN_USB,
                      usb_read_port_complete, qnode);
    ret = usb_submit_urb(purb, GFP_ATOMIC);
    if (ret && ret != (-EPERM))
    {
        LOG_E("cannot submit rx in-token(ret = %d), urb_status = %d\n", ret,
              purb->status);
        if (pskb)
        {
            skb_trim(pskb, 0);
            skb_queue_tail(&hif_node->trx_pipe.free_rx_queue_skb, pskb);

            qnode->buff = NULL;
            zt_data_queue_insert(&hif_node->trx_pipe.free_rx_queue, qnode);
        }

        return ZT_RETURN_FAIL;
    }

    return ZT_RETURN_OK;
}

static zt_s32 zt_usb_write_port(hif_node_st *hif_node, zt_u32 addr,
                                zt_u8 *sdata,
                                zt_u32 slen)
{

    zt_s32 ret                                 = 0;
    data_queue_node_st *data_queue_node    = (data_queue_node_st *)sdata;
    zt_u32 pipe                       = 0;
    struct usb_device *pusbd                = hif_node->u.usb.pusb_dev;
    struct urb *purb                       = data_queue_node->hif_dev;
    data_queue_mngt_st *trxq                = (data_queue_mngt_st *)
            &hif_node->trx_pipe;
    nic_info_st *nic_info                   = (nic_info_st *)hif_node->nic_info[0];

#if 0
    {
        zt_u16 icmp_seq = 0;
        static zt_u16 icmp_seq_recoder = 0;

        if (*(data_queue_node->buff + TXDESC_SIZE + 43) == 1)
        {
            icmp_seq = (*(data_queue_node->buff + TXDESC_SIZE + 60) << 8) | (*
                       (data_queue_node->buff + TXDESC_SIZE + 61));

            if (icmp_seq != 0)
            {
                icmp_seq_recoder = icmp_seq;
            }

            if (icmp_seq_recoder == icmp_seq)
            {
                USB_INFO("[%s, %d] icmp_seq:%d", __func__, __LINE__, icmp_seq);
                icmp_seq_recoder++;
            }
            else
            {
                USB_WARN("[%s, %d] icmp_seq error:%d", __func__, __LINE__, icmp_seq);
            }

        }
    }
#endif

    trxq->tx_queue_cnt++;
    pipe = zt_usb_ffaddr2pipe(&hif_node->u.usb, addr);

    // purb = usb_alloc_urb(0, GFP_KERNEL);
    // data_queue_node->u.purb = purb;

    usb_fill_bulk_urb(purb, pusbd, pipe, data_queue_node->buff,  slen,
                      usb_write_port_complete, data_queue_node);

    purb->transfer_flags |= URB_ZERO_PACKET;
    data_queue_node->state = TX_STATE_SENDING;
    ret = usb_submit_urb(purb, GFP_ATOMIC);

    if (!ret)
    {
        ret = ZT_RETURN_OK;
    }
    else
    {
        switch (ret)
        {
            case -ENODEV:
                nic_info->is_driver_stopped = zt_true;
                break;
            default:
                break;
        }
    }
    if (ret != ZT_RETURN_OK)
    {
        if (data_queue_node->tx_callback_func)
        {
            data_queue_node->tx_callback_func(data_queue_node->tx_info,
                                              data_queue_node->param);
            LOG_I("usb write port failed, free buf");
        }
        zt_data_queue_insert(&trxq->free_tx_queue, data_queue_node);
    }

    return ret;
}


static zt_s32 usb_ctrl_write(struct usb_device *pusb_dev, zt_s8 *ctlBuf,
                             zt_u32 addr, zt_s8 *data, zt_s32 datalen)
{
    zt_u8 brequest      = USB_REQUEST_SET_ADDRESS;
    zt_u8 brequesttype  = USB_REQUEST_TYPE_VENDOR_OUT;
    zt_u16 wvalue       = addr;
    zt_u16 windex       = 0;
    zt_u16 wlength      = datalen;
    zt_u16 timeout      = USB_CONTROL_MSG_TIMEOUT;
    zt_s32 ret             = 0;
    zt_u32 pipe         = usb_sndctrlpipe(pusb_dev, 0);
    zt_u8 retryCnt      = 0;

    ZT_ASSERT(data != NULL);
    ZT_ASSERT(pusb_dev != NULL);

    if (datalen > USB_CONTROL_MSG_BF_LEN)
    {
        USB_WARN("datalen > USB_CONTROL_MSG_BF_LEN");
        return -1;
    }

    zt_memset(ctlBuf, 0, datalen);
    zt_memcpy(ctlBuf, data, datalen);

    for (retryCnt = 0; retryCnt < MAX_USBCTRL_VENDORREQ_TIMES; retryCnt++)
    {
        ret = usb_control_msg(pusb_dev, pipe, brequest, brequesttype, wvalue, windex, \
                              ctlBuf, wlength, timeout);
        if (ret == datalen)
        {
            return 0;
        }
        else
        {
            if (ret == (-ESHUTDOWN) || ret == -ENODEV)
            {
                LOG_E("usb_control_msg error. need ShutDown!!");
                return -1;
            }

            LOG_W("usb_control_msg: retry send %d times", retryCnt);
        }
    }

    if (retryCnt == MAX_USBCTRL_VENDORREQ_TIMES)
    {
        hif_exception_handle();
    }

    return -1;
}


static zt_s32 usb_ctrl_read(struct usb_device *pusb_dev, zt_s8 *ctlBuf,
                            zt_u32 addr, zt_s8 *data, zt_s32 datalen)
{
    zt_u8 brequest      = USB_REQUEST_SET_ADDRESS;
    zt_u8 brequesttype  = USB_REQUEST_TYPE_VENDOR_IN;
    zt_u16 wvalue       = addr;
    zt_u16 windex       = 0;
    zt_u16 wlength      = datalen;
    zt_u16 timeout      = USB_CONTROL_MSG_TIMEOUT;
    zt_s32 ret             = 0;
    zt_u32 pipe         = usb_rcvctrlpipe(pusb_dev, 0);
    zt_s32 vendorreq_times = 0;

    ZT_ASSERT(ctlBuf != NULL);
    ZT_ASSERT(pusb_dev != NULL);
    ZT_ASSERT(data != NULL);

    if (datalen > USB_CONTROL_MSG_BF_LEN)
    {
        USB_WARN("datalen > USB_CONTROL_MSG_BF_LEN");
        return -1;
    }

    zt_memset(ctlBuf, 0, datalen);
    while (++vendorreq_times <= MAX_USBCTRL_VENDORREQ_TIMES)
    {
        ret = usb_control_msg(pusb_dev, pipe, brequest, brequesttype, wvalue, windex,
                              ctlBuf, wlength, timeout);
        if (ret == datalen)
        {
            zt_memcpy(data, ctlBuf, datalen);
            ret = 0;
            break;
        }
        else
        {
            USB_DBG("reg 0x%x, usb %s %u fail, status:%d value = 0x%x, vendorreq_times:%d\n",
                    addr, "read", datalen, ret, *(zt_u32 *) data, vendorreq_times);
        }
    }

    return ret;
}


#define BULK_SYNC_TIMEOUT   1000
static zt_s32 usb_bulk_write_sync(struct hif_node_ *node, zt_u32 addr,
                                  zt_s8 *data, zt_s32 datalen)
{
    zt_u32 pipe                 = 0;
    zt_s32 status                  = 0;
    zt_s32 actual_len              = 0;
    hif_usb_mngt *pusb_mngt    = &(node->u.usb);
    struct usb_device *pusb_dev = pusb_mngt->pusb_dev;

    ZT_ASSERT(pusb_mngt != NULL);
    ZT_ASSERT(data != NULL);
    ZT_ASSERT(pusb_dev != NULL);

    if (datalen == 0)
    {
        USB_WARN("wirte len = 0");
        return -1;
    }

    pipe = zt_usb_ffaddr2pipe(&node->u.usb, addr);
    status = usb_bulk_msg(pusb_dev, pipe, data, datalen, &actual_len,
                          BULK_SYNC_TIMEOUT);

    if (status)
    {
        USB_INFO("-->usb_write_port error, errno no is %d", status);
        return -1;
    }

    return actual_len;
}

static zt_s32 zt_usb_write(struct hif_node_ *node, zt_u8 flag,
                           zt_u32 addr, zt_s8 *data, zt_s32 datalen)
{
    zt_s32 ret                     = 0;
    hif_usb_mngt *pusb_mngt     = &(node->u.usb);
    zt_u8 *pchar                = pusb_mngt->ctrl_msg_buffer;
    struct usb_device *pusb_dev = pusb_mngt->pusb_dev;

    if (hm_get_mod_removed() == zt_false && node->dev_removed == zt_true)
    {
        return -1;
    }
    else
    {
        if (ZT_USB_CTL_MSG == flag)
        {
            ret = usb_ctrl_write(pusb_dev, pchar, addr, data, datalen);
        }
        else if (ZT_USB_BLK_SYNC == flag)
        {
            ret = usb_bulk_write_sync(node, addr, data, datalen);
        }
        else
        {
            ret = zt_usb_write_port(node, addr, data, datalen);
        }
    }
    return ret;
}

static zt_s32 zt_usb_read(struct hif_node_ *node, zt_u8 flag,
                          zt_u32 addr, zt_s8 *data, zt_s32 datalen)
{
    zt_s32 ret                     = 0;
    hif_usb_mngt *pusb_mngt     = &(node->u.usb);
    zt_u8 *pchar                = pusb_mngt->ctrl_msg_buffer;
    struct usb_device *pusb_dev = pusb_mngt->pusb_dev;

    if (hm_get_mod_removed() == zt_false && node->dev_removed == zt_true)
    {
        return -1;
    }
    else
    {
        if (ZT_USB_CTL_MSG == flag)
        {
            ret = usb_ctrl_read(pusb_dev, pchar, addr, data, datalen);
        }
        else
        {
            ret = zt_usb_read_port(node, addr, data, datalen);
        }
    }

    return ret;
}

static zt_s32 zt_usb_show(struct hif_node_ *node)
{
    return 0;
}

static zt_s32 zt_usb_alloc_urb(data_queue_node_st *data_node)
{
    data_node->hif_dev = usb_alloc_urb(0, GFP_KERNEL);
    if (NULL == data_node->hif_dev)
    {
        return -1;
    }

    return 0;
}

static zt_s32 zt_usb_free_urb(data_queue_node_st *data_node)
{
    usb_kill_urb(data_node->hif_dev);
    usb_free_urb(data_node->hif_dev);

    return 0;
}


static struct hif_node_ops  usb_node_ops =
{
    .hif_read                   = zt_usb_read,
    .hif_show                   = zt_usb_show,
    .hif_write                  = zt_usb_write,
    .hif_init                   = zt_usb_init,
    .hif_exit                   = zt_usb_deinit,
    .hif_tx_queue_insert        = zt_tx_queue_insert,
    .hif_tx_queue_empty         = zt_tx_queue_empty,
    .hif_alloc_buff             = zt_usb_alloc_urb,
    .hif_free_buff              = zt_usb_free_urb,
};

static zt_s32 zt_usb_probe(struct usb_interface *pusb_intf,
                           const struct usb_device_id *pdid)
{
    hif_node_st  *hif_node  = NULL;
    zt_u8 *pctrl_buffer     = NULL;
    hif_usb_mngt *pusb_mngt = NULL;
    //zt_u32 version          = 0;
    zt_s32 ret                 = 0;

    LOG_D("************USB CONNECT*************");

    USB_DBG("[usb] match usb_device !!");

    pusb_mngt = zt_kzalloc(sizeof(hif_usb_mngt));
    if (NULL == pusb_mngt)
    {
        LOG_E("zt_kzalloc for usb_mngt failed");
        ret = -ENODEV;
        goto exit;
    }

    ret = zt_endpoint_init(pusb_intf, pusb_mngt);
    if (ret < 0)
    {
        USB_WARN("[usb] zt_endpoint_init error");
        ret = -ENODEV;
        goto exit;
    }

    pctrl_buffer = zt_kzalloc(USB_CONTROL_MSG_BF_LEN);
    if (NULL == pctrl_buffer)
    {
        USB_WARN("[usb] no memmory for usb hif transmit");
        ret = -ENODEV;
        goto exit;
    }

    /* create hif_node */
    hif_node_register(&hif_node, HIF_USB, &usb_node_ops);
    if (NULL == hif_node)
    {
        USB_WARN("[usb] hif_node_register for HIF_USB failed");
        ret = -ENODEV;
        goto exit;
    }

    hif_node->drv_ops = (struct device_info_ops *)pdid->driver_info;
    /*add usb handle into hif_node */
    pusb_mngt->usb_id = hm_new_usb_id(NULL);
    pusb_mngt->ctrl_msg_buffer = pctrl_buffer;
    zt_memcpy(&hif_node->u.usb, pusb_mngt, sizeof(hif_usb_mngt));

    /* save hif_node in usb_intf */
    usb_set_intfdata(pusb_intf, hif_node);

    /* insert dev on hif_node */
    if (hif_dev_insert(hif_node) < 0)
    {
        USB_WARN("[usb] hif dev insert error !!");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
        usb_put_dev(interface_to_usbdev(pusb_intf));
#endif
        ret = -ENODEV;
        goto exit;
    }

exit :
    if (pusb_mngt)
    {
        zt_kfree(pusb_mngt);
    }

    if (ret < 0)
    {
        if (pctrl_buffer)
        {
            zt_kfree(pctrl_buffer);
        }
    }

    return ret;
}

static void zt_usb_disconnect(struct usb_interface *pusb_intf)
{
    hif_node_st *hif_node   = usb_get_intfdata(pusb_intf);
    zt_s32 ret                 = 0;

    LOG_D("************USB DISCONNECT*************");

    /* ndev unregister should been do first */
    ndev_unregister_all(hif_node->nic_info, hif_node->nic_number);

    hif_dev_removed(hif_node);

    if (NULL != hif_node)
    {
        ret = hm_del_usb_id(hif_node->u.usb.usb_id);
        if (ret)
        {
            USB_WARN("hm_del_usb_id [%d] failed", hif_node->u.usb.usb_id);
        }
        zt_kfree(hif_node->u.usb.ctrl_msg_buffer);
        zt_endpoint_deinit(pusb_intf);
        hif_node_unregister(hif_node);
    }
    else
    {
        USB_WARN("[usb] zt_usb_disconnect failed");
    }
}

static zt_s32 zt_usb_suspend(struct usb_interface *pusb_intf,
                             pm_message_t message)
{
    hif_node_st *hif_node = usb_get_intfdata(pusb_intf);
    zt_u32 i = 0;

    USB_DBG();

    LOG_W("zt_usb_suspend start");

    if (!hif_node)
    {
        USB_ERROR("hif is null");
        return -1;
    }

    for (i = 0; i < hif_node->nic_number; i++)
    {
        nic_info_st *pnic_info = hif_node->nic_info[i];
        if (pnic_info)
        {
            pwr_info_st *ppwr_info = pnic_info->pwr_info;
            if (!ppwr_info)
            {
                USB_DBG("ppwr_info null");
                return -1;
            }

            if (ppwr_info->bInSuspend)
            {
                USB_WARN("nic%d suspend repeated", i);
                return 0;
            }
            ppwr_info->bInSuspend = zt_true;

            if (nic_suspend(pnic_info))
            {
                USB_ERROR("nic_susprnd fail");
                return -1;
            }
        }
    }

    if (power_suspend(hif_node))
    {
        USB_ERROR("power_suspend fail !!!");
        return -1;
    }

    LOG_W("zt_usb_suspend end");

    return 0;
}

static zt_s32 zt_usb_resume(struct usb_interface *pusb_intf)
{
    hif_node_st *hif_node = usb_get_intfdata(pusb_intf);
    zt_u32 i = 0;

    USB_DBG();

    if (power_resume(hif_node))
    {
        USB_ERROR("power_resume false");
        return ZT_RETURN_FAIL;
    }

    for (i = 0; i < hif_node->nic_number; i++)
    {
        nic_info_st *pnic_info = hif_node->nic_info[i];
        if (pnic_info)
        {
            pwr_info_st *ppwr_info = pnic_info->pwr_info;

            if (nic_resume(pnic_info))
            {
                USB_ERROR("nic_resume fail");
                return ZT_RETURN_FAIL;
            }

            if (!ppwr_info->bInSuspend)
            {
                USB_WARN("nic%d has no suspend before", i);
                return ZT_RETURN_FAIL;
            }
            ppwr_info->bInSuspend = zt_false;
        }
    }

    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0))
static void zt_usb_shutdown(struct usb_interface *intf)
#else
static void zt_usb_shutdown(struct device *dev)
#endif
{
}

static struct usb_device_id zt_usb_id_tbl[] =
{
#ifdef CONFIG_ZT9101XV20_SUPPORT
    {
        USB_DEVICE_AND_INTERFACE_INFO(0x350b, 0x9101, 0xff, 0xff, 0xff),
        .driver_info = (unsigned long) &ZT9101XV20_Info
    },
    {
        USB_DEVICE_AND_INTERFACE_INFO(0x2310, 0x9086, 0xff, 0xff, 0xff),
        .driver_info = (unsigned long) &ZT9101XV20_Info
    },
#endif
#ifdef CONFIG_ZT9101XV30_SUPPORT
    {
        USB_DEVICE_AND_INTERFACE_INFO(0x350b, 0x9086, 0xff, 0xff, 0xff),
        .driver_info = (unsigned long) &ZT9101XV30_Info
    },
    {
        USB_DEVICE_AND_INTERFACE_INFO(0x350b, 0x9106, 0xff, 0xff, 0xff),
        .driver_info = (unsigned long) &ZT9101XV30_Info
    },
#endif
    {}
};

MODULE_DEVICE_TABLE(usb, zt_usb_id_tbl);


static struct usb_driver zt_usb_driver =
{
    .name           =   KBUILD_MODNAME,
    .id_table       =   zt_usb_id_tbl,
    .probe          =   zt_usb_probe,
    .disconnect     =   zt_usb_disconnect,
    .suspend        =   zt_usb_suspend,
    .resume         =   zt_usb_resume,
    .reset_resume   =   zt_usb_resume,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0))
    .shutdown       =   zt_usb_shutdown,
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0))
    .driver.shutdown = zt_usb_shutdown,	
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)) && (LINUX_VERSION_CODE < KERNEL_VERSION(6, 8, 0))
    .drvwrap.driver.shutdown    = zt_usb_shutdown,
#endif
    .supports_autosuspend       = 1,
};

zt_s32 usb_init(void)
{
    zt_s32 ret = 0;
    USB_DBG("[usb] usb_init!!\n");
    ret = usb_register(&zt_usb_driver);

    if (ret != 0)
    {
        USB_INFO("usb_register failed");
    }
    return ret;
}

zt_s32 usb_exit(void)
{
    zt_s32 ret = 0;
    USB_DBG("[usb] usb_exit!!\n");
    usb_deregister(&zt_usb_driver);
    return ret;
}
