################################################################################
################################################################################
##
## Makefile -- project specific makefile to build ZTOP Wifi driver
##
## (C) Copyright (c) 2021 Shandong ZTop Microelectronics Co., Ltds
##
## Mandatory settings:
##
## o TOPDIR                        = the toplevel directory (using slashes as path separator)
## o SUBDIR                        = the make file folder
## o CT                            = display driver compile time(y)
## o CONFIG_DEBUG_LEVEL            = log level: 0: turn on all printing 1: turn on printing except debug, 2: turn on warn and error printing, 3: turn on error printing only, 4: turn off printing
## o CONFIG_DRIVER_VER             = null(use svn version), else this is version
## o CONFIG_MODULE_IMPORT_NS       = whether import ns or not(y/n)
## o CONFIG_FW_FILE                = firmware is file or array(y)
## o CONFIG_WIFI_INTERFACE_TWO     = Second WiFi interface (y)
## o CONFIG_TX_SOFT_AGG            = switch for tx soft agg(n)
## o CONFIG_RX_SOFT_AGG            = switch for rx soft agg(y)
## o CONFIG_WIFI_MODE              = all(sta/ap/adhoc/monitor), sta, ap, adhoc
## o CONFIG_WIFI_FRAMEWORK         = wext, nl80211, mp
## o CONFIG_HIF_PORT               = usb, sdio, both
## o CONFIG_CLOCK_24MHZ            = y/n  sdio if use 24MHZ clock
## o CONFIG_ZT9101XV20_SUPPORT     = enable support ZT9101xV20
## o CONFIG_ZT9101XV30_SUPPORT     = enable support ZT9101xV30
## o CONFGI_NAME                   = any string(zt9101_ztopmac)
################################################################################
  export WDRV_DIR ?= $(shell pwd)
  SUBDIR = mak
  PLATDIR = platform
  CT                       ?= n
  CONFIG_DEBUG_LEVEL        = 3
  CONFIG_DRIVER_VER         = V1.2.xxx.xxxxxxxx
  CONFIG_MODULE_IMPORT_NS   = n
  CONFIG_FW_FILE            = y
  CONFIG_STA_AND_AP_MODE    = n
  CONFIG_TX_SOFT_AGG        = n
  CONFIG_RX_SOFT_AGG        = n
  CONFIG_WIFI_MODE          = all
  CONFIG_WIFI_FRAMEWORK     = nl80211
  CONFIG_HIF_PORT           = usb
  CONFIG_CLOCK_24MHZ        = n
  CONFIG_ZT9101XV20_SUPPORT = y
  CONFIG_ZT9101XV30_SUPPORT = y
  CONFIG_POWER_SAVING       = n
  CONFIG_NAME               = zt9101_ztopmac

include $(WDRV_DIR)/$(PLATDIR)/platform.mak

include $(WDRV_DIR)/$(SUBDIR)/linux/Makefile


