# requires: BIN, SRCS

# based on:
# - https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# - Makefiles from the Nordic nRF5 SDK

SRCS := \
	$(NRF5_MDK)/system_nrf52840.c $(NRF5_MDK)/gcc_startup_nrf52840.S \
	$(SEGGER_RTT)/RTT/SEGGER_RTT.c $(SEGGER_RTT)/RTT/SEGGER_RTT_printf.c \
	$(SRCS)

BUILD := _build
OUT := $(BUILD)/$(BIN)

OBJS := $(patsubst %,$(OUT)/%.o,$(basename $(SRCS)))
DEPS := $(patsubst %,$(OUT)/%.d,$(basename $(SRCS)))

CFLAGS := \
	-Wall -Werror \
	-mcpu=cortex-m4 -mabi=aapcs \
	-ffunction-sections -fdata-sections -Og -g3 -flto \
	-I $(NRF5_MDK) -I $(S140_INCLUDE) \
	-I $(NRF5_SDK)/components/toolchain/cmsis/include \
	-I $(SEGGER_RTT)/RTT \
	-D DEBUG \
	-D NRF52840_XXAA \
	$(CFLAGS)
ASMFLAGS := -mcpu=cortex-m4
LDFLAGS := \
	-mcpu=cortex-m4 -mabi=aapcs \
	--specs=nano.specs -lc -lnosys \
	-Wl,-Map=$(OUT)/$(BIN).map \
	-Wl,--gc-sections \
	-L $(NRF5_MDK) -T config.ld

DEPFLAGS = -MT $@ -MD -MP -MF $(OUT)/$*.Td

POSTCOMPILE = mv -f $(OUT)/$*.Td $(OUT)/$*.d

all : $(OUT)/$(BIN).hex

.PHONY: flash flash_softdevice clean build_dirs

flash: $(OUT)/$(BIN).hex
	nrfjprog --program $< -f nrf52 --sectorerase
	nrfjprog --reset -f nrf52

flash_softdevice:
	nrfjprog --program $(S130_HEX) -f nrf52 --sectorerase
	nrfjprog --reset -f nrf51

clean:
	$(RM) -r $(BUILD)

build_dirs:
	@mkdir -p $(dir $(OBJS))

$(OUT)/$(BIN).hex : $(OUT)/$(BIN)
	$(OBJCOPY) -O ihex $< $@

$(OUT)/$(BIN) : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(OUT)/%.o : %.c
$(OUT)/%.o : %.c $(OUT)/%.d | build_dirs
	$(CC) $(DEPFLAGS) $(CFLAGS) -c -o $@ $<
	$(POSTCOMPILE)

$(OUT)/%.o : %.S
$(OUT)/%.o : %.S $(OUT)/%.d | build_dirs
	$(CC) $(DEPFLAGS) $(ASMFLAGS) -c -o $@ $<
	$(POSTCOMPILE)

.PRECIOUS = $(OUT)/%.d
$(OUT)/%.d : ;

-include $(DEPS)
