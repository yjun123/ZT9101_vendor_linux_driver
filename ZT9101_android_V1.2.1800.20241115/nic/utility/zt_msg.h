/*
 * zt_msg.h
 *
 * This file contains all the prototypes for the zt_msg.c file
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
#ifndef __ZT_MSG_H__
#define __ZT_MSG_H__

/* macro */

/* |-------------tag--------------|
   |-domain-|--priority--|---id---|
   |b15  b12|b11       b7|b6    b0| */
#define ZT_MSG_TAG_DOM_OFS      12
#define ZT_MSG_TAG_DOM_MSK      0xF
#define ZT_MSG_TAG_PRI_OFS      7
#define ZT_MSG_TAG_PRI_MSK      0x1F
#define ZT_MSG_TAG_ID_OFS       0
#define ZT_MSG_TAG_ID_MSK       0x7F
#define ZT_MSG_TAG_SET(dom, pri, id) \
    ((((dom) & ZT_MSG_TAG_DOM_MSK) << ZT_MSG_TAG_DOM_OFS) | \
     (((pri) & ZT_MSG_TAG_PRI_MSK) << ZT_MSG_TAG_PRI_OFS) | \
     (((id)  & ZT_MSG_TAG_ID_MSK)  << ZT_MSG_TAG_ID_OFS))
#define ZT_MSG_TAG_DOM(tag) \
    ((zt_msg_tag_dom_t)(((tag) >> ZT_MSG_TAG_DOM_OFS) & ZT_MSG_TAG_DOM_MSK))
#define ZT_MSG_TAG_PRI(tag) \
    ((zt_msg_tag_pri_t)(((tag) >> ZT_MSG_TAG_PRI_OFS) & ZT_MSG_TAG_PRI_MSK))
#define ZT_MSG_TAG_ID(tag) \
    ((zt_msg_tag_id_t)(((tag) >> ZT_MSG_TAG_ID_OFS) & ZT_MSG_TAG_ID_MSK))

/* type */
typedef struct
{
    zt_os_api_lock_t lock;
    zt_que_t pend;
    zt_que_t free;
} zt_msg_que_t;

typedef zt_u16 zt_msg_tag_t;
typedef zt_u8  zt_msg_tag_dom_t;
typedef zt_u8  zt_msg_tag_pri_t;
typedef zt_u8  zt_msg_tag_id_t;

typedef struct
{
    zt_que_list_t list;
    zt_que_t *pque;
    zt_u32 alloc_value_size;
    zt_msg_tag_t tag;
    zt_u32 len;
    zt_u8 value[];
} zt_msg_t;

/* function */

zt_inline static zt_u32 zt_msg_count(zt_msg_que_t *pmsg_que)
{
    return pmsg_que->pend.cnt;
}

zt_inline static zt_bool zt_msg_is_empty(zt_msg_que_t *pmsg_que)
{
    return zt_que_is_empty(&pmsg_que->pend);
}

zt_s32 zt_msg_new(zt_msg_que_t *pmsg_que, zt_msg_tag_t tag,
                  zt_msg_t **pnew_msg);
zt_s32 zt_msg_push(zt_msg_que_t *pmsg_que, zt_msg_t *pmsg);
zt_s32 zt_msg_push_head(zt_msg_que_t *pmsg_que, zt_msg_t *pmsg);
zt_s32 msg_get(zt_msg_que_t *pmsg_que, zt_msg_t **pmsg,
               zt_bool bpop, zt_bool btail);

zt_inline static zt_s32 zt_msg_get(zt_msg_que_t *pmsg_que, zt_msg_t **pmsg)
{
    return msg_get(pmsg_que, pmsg, zt_false, zt_false);
}

zt_inline static zt_s32 zt_msg_get_tail(zt_msg_que_t *pmsg_que, zt_msg_t **pmsg)
{
    return msg_get(pmsg_que, pmsg, zt_false, zt_true);
}

zt_inline static zt_s32 zt_msg_pop(zt_msg_que_t *pmsg_que, zt_msg_t **pmsg)
{
    return msg_get(pmsg_que, pmsg, zt_true, zt_false);
}

zt_inline static zt_s32 zt_msg_pop_tail(zt_msg_que_t *pmsg_que, zt_msg_t **pmsg)
{
    return msg_get(pmsg_que, pmsg, zt_true, zt_true);
}

zt_s32 msg_get_dom(zt_msg_que_t *pmsg_que, zt_msg_tag_dom_t dom,
                   zt_msg_t **pmsg,
                   zt_bool bpop, zt_bool btail);

zt_inline static
zt_s32 zt_msg_get_dom(zt_msg_que_t *pmsg_que,
                      zt_msg_tag_dom_t dom, zt_msg_t **pmsg)
{
    return msg_get_dom(pmsg_que, dom, pmsg, zt_false, zt_false);
}

zt_inline static
zt_s32 zt_msg_get_dom_tail(zt_msg_que_t *pmsg_que,
                           zt_msg_tag_dom_t dom, zt_msg_t **pmsg)
{
    return msg_get_dom(pmsg_que, dom, pmsg, zt_false, zt_true);
}

zt_inline static
zt_s32 zt_msg_pop_dom(zt_msg_que_t *pmsg_que,
                      zt_msg_tag_dom_t dom, zt_msg_t **pmsg)
{
    return msg_get_dom(pmsg_que, dom, pmsg, zt_true, zt_false);
}

zt_inline static
zt_s32 zt_msg_pop_dom_tail(zt_msg_que_t *pmsg_que,
                           zt_msg_tag_dom_t dom, zt_msg_t **pmsg)
{
    return msg_get_dom(pmsg_que, dom, pmsg, zt_true, zt_true);
}

zt_s32 zt_msg_del(zt_msg_que_t *pmsg_que, zt_msg_t *pmsg);
zt_s32 zt_msg_alloc(zt_msg_que_t *pmsg_que,
                    zt_msg_tag_t tag, zt_u32 size, zt_u8 num);
zt_s32 zt_msg_free(zt_msg_que_t *pmsg_que);

zt_inline static void zt_msg_init(zt_msg_que_t *pmsg_que)
{
    zt_os_api_lock_init(&pmsg_que->lock, ZT_LOCK_TYPE_IRQ);
    zt_que_init(&pmsg_que->pend, ZT_LOCK_TYPE_NONE);
    zt_que_init(&pmsg_que->free, ZT_LOCK_TYPE_NONE);
}

zt_inline static void zt_msg_deinit(zt_msg_que_t *pmsg_que)
{
    zt_msg_free(pmsg_que);
    zt_os_api_lock_term(&pmsg_que->lock);
    zt_que_deinit(&pmsg_que->pend);
    zt_que_deinit(&pmsg_que->free);
}

#endif

