################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ti_cgt_arm_llvm_4.0.1.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mlittle-endian -mthumb -I"C:/ti/ti_cgt_arm_llvm_4.0.1.LTS/include/c" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/board/ethphy/enet/rtos_drivers/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/board/ethphy/port" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/config/am243x/r5f" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/utils/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/utils/V3" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/include/phy" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/include/core" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/soc/k3/am64x_am243x" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/hw_include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/hw_include/mdio/V4" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/lwip/lwip-stack/src/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/lwip/lwip-port/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/lwip/lwip-port/freertos/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/lwipif/inc" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/lwip/lwip-stack/contrib" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/lwip/lwip-config/am243x" -DSOC_AM243X -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/AnasAhmed/UDP Server/enet_cpsw_udpclient_am243x-lp_r5fss0-0_freertos_ti-arm-clang/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-2071933585: C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/examples/lwip/enet_cpsw_udpclient/am243x-lp/r5fss0-0_freertos/example.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs2030/ccs/utils/sysconfig_1.25.0/sysconfig_cli.bat" --script "C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/examples/lwip/enet_cpsw_udpclient/am243x-lp/r5fss0-0_freertos/example.syscfg" -o "syscfg" -s "C:/ti/mcu_plus_sdk_am243x_11_00_00_15/.metadata/product.json" -p "ALX" -r "ALX" --context "r5fss0-0" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_dpl_config.c: build-2071933585 C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/examples/lwip/enet_cpsw_udpclient/am243x-lp/r5fss0-0_freertos/example.syscfg
syscfg/ti_dpl_config.h: build-2071933585
syscfg/ti_drivers_config.c: build-2071933585
syscfg/ti_drivers_config.h: build-2071933585
syscfg/ti_drivers_open_close.c: build-2071933585
syscfg/ti_drivers_open_close.h: build-2071933585
syscfg/ti_pinmux_config.c: build-2071933585
syscfg/ti_power_clock_config.c: build-2071933585
syscfg/ti_board_config.c: build-2071933585
syscfg/ti_board_config.h: build-2071933585
syscfg/ti_board_open_close.c: build-2071933585
syscfg/ti_board_open_close.h: build-2071933585
syscfg/ti_enet_config.c: build-2071933585
syscfg/ti_enet_config.h: build-2071933585
syscfg/ti_enet_open_close.c: build-2071933585
syscfg/ti_enet_open_close.h: build-2071933585
syscfg/ti_enet_soc.c: build-2071933585
syscfg/ti_enet_lwipif.c: build-2071933585
syscfg/ti_enet_lwipif.h: build-2071933585
syscfg/linker.cmd: build-2071933585
syscfg/linker_defines.h: build-2071933585
syscfg: build-2071933585

syscfg/%.o: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ti_cgt_arm_llvm_4.0.1.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mlittle-endian -mthumb -I"C:/ti/ti_cgt_arm_llvm_4.0.1.LTS/include/c" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/board/ethphy/enet/rtos_drivers/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/board/ethphy/port" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/config/am243x/r5f" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/utils/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/utils/V3" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/include/phy" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/include/core" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/soc/k3/am64x_am243x" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/hw_include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/hw_include/mdio/V4" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/lwip/lwip-stack/src/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/lwip/lwip-port/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/lwip/lwip-port/freertos/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/enet/core/lwipif/inc" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/lwip/lwip-stack/contrib" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/networking/lwip/lwip-config/am243x" -DSOC_AM243X -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -MMD -MP -MF"syscfg/$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/AnasAhmed/UDP Server/enet_cpsw_udpclient_am243x-lp_r5fss0-0_freertos_ti-arm-clang/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


