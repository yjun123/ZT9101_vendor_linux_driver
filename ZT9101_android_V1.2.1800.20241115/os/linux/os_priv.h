/*
 * os_priv.h
 *
 * used for .....
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
#ifndef __OS_PRIV_H__
#define __OS_PRIV_H__

typedef __kernel_size_t         SIZE_T;
typedef __kernel_ssize_t        SSIZE_T;

typedef zt_ptr                      zt_irq;
typedef struct tasklet_struct       zt_tasklet;
#define zt_tasklet_hi_sched         tasklet_hi_schedule
#define zt_tasklet_sched            tasklet_schedule
#define zt_tasklet_init             tasklet_init

#define ZT_OFFSETOF(type, field)            offsetof(type, field)
#define ZT_FIELD_SIZEOF(t, f)               (sizeof(((t*)0)->f))
#define ZT_CONTAINER_OF(ptr, type, field)   container_of(ptr, type, field)

#define ZT_HZ                       HZ
#define zt_os_api_do_div            do_div
#define zt_yield                    yield
#define zt_inline                   __always_inline

#define zt_strchr                   strchr

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(7,2,0))
	#define zt_strncpy                  strscpy_pad 
#else

	#define zt_strncpy                  strncpy
#endif

#define zt_strncmp                  strncmp
#define zt_strcmp                   strcmp
#define zt_memcpy                   memcpy
#define zt_memcmp                   memcmp
#define zt_memset                   memset
#define zt_strlen                   strlen
#define zt_mdelay                   mdelay
#define zt_udelay                   udelay
#define zt_msleep                   msleep
#define zt_sprintf                  sprintf
#define zt_strncat                  strncat
#define zt_snprintf                 snprintf


#define zt_kzalloc(sz)              kzalloc(sz, in_interrupt()? GFP_ATOMIC : GFP_KERNEL)
#define zt_kfree                    kfree

#define zt_vmalloc                  vmalloc
#define zt_vfree                    vfree
#define zt_alloc_skb(sz)            __dev_alloc_skb(sz, in_interrupt()? GFP_ATOMIC : GFP_KERNEL)
#define zt_free_skb                 dev_kfree_skb_any

#define zt_packed                   __attribute__((__packed__))

#define ZT_BUG                      BUG

#define ZT_KERN_LEVELS_ALERT        KERN_ALERT   /* this use to set printk funcation output
                                                  level with specify level. the set value
                                                  should highter than system default console
                                                  level(usually equal to KERN_WARNING). */

#define ZT_KERN_LEVELS_DEBUG        KERN_DEBUG
#define ZT_KERN_LEVELS_INFO         KERN_INFO
#define ZT_KERN_LEVELS_WARNING      KERN_WARNING
#define ZT_KERN_LEVELS_ERR          KERN_ERR
#define ZT_LOG_PRINT                printk

typedef struct work_struct zt_work_struct;

typedef struct workqueue_struct  zt_workqueue_struct;
typedef void (*work_func)(zt_work_struct *work);
typedef struct zt_workqueue_mgnt_st_ zt_workqueue_mgnt_st;

typedef struct
{
    zt_s8 *workqueue_name;
    work_func func;
} zt_workqueue_func_param_st;

typedef struct mutex            zt_lock_mutex_t;
typedef struct semaphore        zt_os_api_sema_t;
typedef struct timer_list       zt_os_api_timer_t;
typedef struct file             zt_file;

#endif
