PLAYGROUND_ROOT := ..
include $(PLAYGROUND_ROOT)/config.mk

BIN := uart_nrfx

SRCS := main.c

CFLAGS += -D USE_APP_CONFIG

NRFX := $(NRF5_SDK)/modules/nrfx
CFLAGS += -I $(NRFX) -I $(NRFX)/drivers/include -I $(NRF5_SDK)/integration/nrfx -I .
SRCS += $(NRFX)/drivers/src/nrfx_clock.c
SRCS += $(NRFX)/drivers/src/nrfx_uarte.c

CFLAGS += -I $(NRF5_SDK)/components/libraries/atomic
SRCS += $(NRF5_SDK)/components/libraries/atomic/nrf_atomic.c

CFLAGS += -I $(NRF5_SDK)/components/libraries/balloc
SRCS += $(NRF5_SDK)/components/libraries/balloc/nrf_balloc.c

CFLAGS += -I $(NRF5_SDK)/components/libraries/delay

CFLAGS += -I $(NRF5_SDK)/components/libraries/experimental_section_vars

CFLAGS += -I $(NRF5_SDK)/components/libraries/memobj
SRCS += $(NRF5_SDK)/components/libraries/memobj/nrf_memobj.c

CFLAGS += -I $(NRF5_SDK)/components/libraries/log
CFLAGS += -I $(NRF5_SDK)/components/libraries/log/src
SRCS += $(NRF5_SDK)/components/libraries/log/src/nrf_log_backend_rtt.c
SRCS += $(NRF5_SDK)/components/libraries/log/src/nrf_log_backend_serial.c
SRCS += $(NRF5_SDK)/components/libraries/log/src/nrf_log_default_backends.c
SRCS += $(NRF5_SDK)/components/libraries/log/src/nrf_log_frontend.c
SRCS += $(NRF5_SDK)/components/libraries/log/src/nrf_log_str_formatter.c

CFLAGS += -I $(NRF5_SDK)/components/libraries/ringbuf
SRCS += $(NRF5_SDK)/components/libraries/ringbuf/nrf_ringbuf.c

CFLAGS += -I $(NRF5_SDK)/components/libraries/strerror
SRCS += $(NRF5_SDK)/components/libraries/strerror/nrf_strerror.c

CFLAGS += -I $(NRF5_SDK)/components/libraries/timer
SRCS += $(NRF5_SDK)/components/libraries/timer/app_timer.c

CFLAGS += -I $(NRF5_SDK)/components/libraries/util
SRCS += $(NRF5_SDK)/components/libraries/util/app_error.c
SRCS += $(NRF5_SDK)/components/libraries/util/app_error_handler_gcc.c
SRCS += $(NRF5_SDK)/components/libraries/util/app_error_weak.c
SRCS += $(NRF5_SDK)/components/libraries/util/app_util_platform.c
SRCS += $(NRF5_SDK)/components/libraries/util/nrf_assert.c

CFLAGS += -I $(NRF5_SDK)/external/fprintf
SRCS += $(NRF5_SDK)/external/fprintf/nrf_fprintf.c
SRCS += $(NRF5_SDK)/external/fprintf/nrf_fprintf_format.c

include $(PLAYGROUND_ROOT)/common.mk
