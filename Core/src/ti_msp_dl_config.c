/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0L130X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_TIMER_0_init();
    SYSCFG_DL_TIMER_1_init();
    SYSCFG_DL_UART_1_init();
    SYSCFG_DL_TIMER_Cross_Trigger_init();
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_TimerG_reset(TIMER_0_INST);
    DL_TimerG_reset(TIMER_1_INST);
    DL_UART_Main_reset(UART_1_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_TimerG_enablePower(TIMER_0_INST);
    DL_TimerG_enablePower(TIMER_1_INST);
    DL_UART_Main_enablePower(UART_1_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_1_IOMUX_TX, GPIO_UART_1_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_1_IOMUX_RX, GPIO_UART_1_IOMUX_RX_FUNC);

    DL_GPIO_initDigitalOutput(GPIO_GRP_0_CS_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_0_LED_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_0_D0_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_0_D1_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_0_DC_IOMUX);

    DL_GPIO_initDigitalOutput(MATRIX_V1_IOMUX);

    DL_GPIO_initDigitalOutput(MATRIX_V2_IOMUX);

    DL_GPIO_initDigitalOutput(MATRIX_V3_IOMUX);

    DL_GPIO_initDigitalOutput(MATRIX_V4_IOMUX);

    DL_GPIO_initDigitalInput(MATRIX_H1_IOMUX);

    DL_GPIO_initDigitalInput(MATRIX_H2_IOMUX);

    DL_GPIO_initDigitalInput(MATRIX_H3_IOMUX);

    DL_GPIO_initDigitalInput(MATRIX_H4_IOMUX);

    DL_GPIO_initDigitalOutput(LEDLIGHTS_LEDlight_IOMUX);

    DL_GPIO_initDigitalOutput(LEDLIGHTS_LEDlight2_IOMUX);

    DL_GPIO_initDigitalOutput(BUZZER_SDA_IOMUX);

    DL_GPIO_initDigitalOutput(BUZZER_SCL_IOMUX);

    DL_GPIO_clearPins(GPIOA, GPIO_GRP_0_LED_PIN |
		GPIO_GRP_0_D0_PIN |
		GPIO_GRP_0_D1_PIN |
		GPIO_GRP_0_DC_PIN |
		MATRIX_V1_PIN |
		MATRIX_V2_PIN |
		MATRIX_V3_PIN |
		MATRIX_V4_PIN |
		BUZZER_SCL_PIN);
    DL_GPIO_setPins(GPIOA, GPIO_GRP_0_CS_PIN |
		LEDLIGHTS_LEDlight_PIN |
		LEDLIGHTS_LEDlight2_PIN |
		BUZZER_SDA_PIN);
    DL_GPIO_enableOutput(GPIOA, GPIO_GRP_0_CS_PIN |
		GPIO_GRP_0_LED_PIN |
		GPIO_GRP_0_D0_PIN |
		GPIO_GRP_0_D1_PIN |
		GPIO_GRP_0_DC_PIN |
		MATRIX_V1_PIN |
		MATRIX_V2_PIN |
		MATRIX_V3_PIN |
		MATRIX_V4_PIN |
		LEDLIGHTS_LEDlight_PIN |
		LEDLIGHTS_LEDlight2_PIN |
		BUZZER_SDA_PIN |
		BUZZER_SCL_PIN);
    DL_GPIO_setLowerPinsPolarity(GPIOA, DL_GPIO_PIN_0_EDGE_FALL |
		DL_GPIO_PIN_1_EDGE_FALL |
		DL_GPIO_PIN_7_EDGE_FALL |
		DL_GPIO_PIN_12_EDGE_FALL);
    DL_GPIO_clearInterruptStatus(GPIOA, MATRIX_H1_PIN |
		MATRIX_H2_PIN |
		MATRIX_H3_PIN |
		MATRIX_H4_PIN);
    DL_GPIO_enableInterrupt(GPIOA, MATRIX_H1_PIN |
		MATRIX_H2_PIN |
		MATRIX_H3_PIN |
		MATRIX_H4_PIN);

}


SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_enableMFCLK();
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);

}



/*
 * Timer clock configuration to be sourced by LFCLK /  (32768 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32768 Hz = 32768 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gTIMER_0ClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_LFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMER_0_INST_LOAD_VALUE = (1 s * 32768 Hz) - 1
 */
static const DL_TimerG_TimerConfig gTIMER_0TimerConfig = {
    .period     = TIMER_0_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_START,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMER_0_init(void) {

    DL_TimerG_setClockConfig(TIMER_0_INST,
        (DL_TimerG_ClockConfig *) &gTIMER_0ClockConfig);

    DL_TimerG_initTimerMode(TIMER_0_INST,
        (DL_TimerG_TimerConfig *) &gTIMER_0TimerConfig);
    DL_TimerG_enableInterrupt(TIMER_0_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
	NVIC_SetPriority(TIMER_0_INST_INT_IRQN, 0);
    DL_TimerG_enableClock(TIMER_0_INST);




     /* DL_TIMER_CROSS_TRIG_SRC is a Don't Care field when Cross Trigger Source is set to Software */
    DL_TimerG_configCrossTrigger(TIMER_0_INST, DL_TIMER_CROSS_TRIG_SRC_FSUB0,
	DL_TIMER_CROSS_TRIGGER_INPUT_DISABLED, DL_TIMER_CROSS_TRIGGER_MODE_ENABLED
		);
    /*
     * Determines the external triggering event to trigger the module (self-triggered in main configuration)
     * and triggered by specific timer in secondary configuration
     */
    DL_TimerG_setExternalTriggerEvent(TIMER_0_INST,DL_TIMER_EXT_TRIG_SEL_TRIG_1);
    DL_TimerG_enableExternalTrigger(TIMER_0_INST);
}

/*
 * Timer clock configuration to be sourced by LFCLK /  (32768 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32768 Hz = 32768 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gTIMER_1ClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_LFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMER_1_INST_LOAD_VALUE = (1 s * 32768 Hz) - 1
 */
static const DL_TimerG_TimerConfig gTIMER_1TimerConfig = {
    .period     = TIMER_1_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMER_1_init(void) {

    DL_TimerG_setClockConfig(TIMER_1_INST,
        (DL_TimerG_ClockConfig *) &gTIMER_1ClockConfig);

    DL_TimerG_initTimerMode(TIMER_1_INST,
        (DL_TimerG_TimerConfig *) &gTIMER_1TimerConfig);
    DL_TimerG_enableInterrupt(TIMER_1_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
	NVIC_SetPriority(TIMER_1_INST_INT_IRQN, 0);
    DL_TimerG_enableClock(TIMER_1_INST);




     /* DL_TIMER_CROSS_TRIG_SRC is a Don't Care field when Cross Trigger Source is set to Software */
    DL_TimerG_configCrossTrigger(TIMER_1_INST, DL_TIMER_CROSS_TRIG_SRC_FSUB0,
	DL_TIMER_CROSS_TRIGGER_INPUT_DISABLED, DL_TIMER_CROSS_TRIGGER_MODE_ENABLED
		);
    /*
     * Determines the external triggering event to trigger the module (self-triggered in main configuration)
     * and triggered by specific timer in secondary configuration
     */
    DL_TimerG_setExternalTriggerEvent(TIMER_1_INST,DL_TIMER_EXT_TRIG_SEL_TRIG_0);
    DL_TimerG_enableExternalTrigger(TIMER_1_INST);
}

SYSCONFIG_WEAK void SYSCFG_DL_TIMER_Cross_Trigger_init(void) {
    DL_TimerG_generateCrossTrigger(TIMER_0_INST);
}


static const DL_UART_Main_ClockConfig gUART_1ClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_1Config = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_1_init(void)
{
    DL_UART_Main_setClockConfig(UART_1_INST, (DL_UART_Main_ClockConfig *) &gUART_1ClockConfig);

    DL_UART_Main_init(UART_1_INST, (DL_UART_Main_Config *) &gUART_1Config);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115211.52
     */
    DL_UART_Main_setOversampling(UART_1_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_1_INST, UART_1_IBRD_32_MHZ_115200_BAUD, UART_1_FBRD_32_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART_1_INST,
                                 DL_UART_MAIN_INTERRUPT_RX |
                                 DL_UART_MAIN_INTERRUPT_TX);


    DL_UART_Main_enable(UART_1_INST);
}

