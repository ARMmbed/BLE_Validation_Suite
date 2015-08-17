/* mbed Microcontroller Library 
 *******************************************************************************
 * Copyright (c) 2015 WIZnet Co.,Ltd. All rights reserved.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of ARM Limited nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************
 */
 
#include "PeripheralPins.h"
#include "PeripheralNames.h"
#include "pinmap.h"


//*** ADC ***
const PinMap PinMap_ADC[] = {
    {PC_15, ADC_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF3)}, // ADC0_IN0
    {PC_14, ADC_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF3)}, // ADC0_IN1
    {PC_13, ADC_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF3)}, // ADC0_IN2
    {PC_12, ADC_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF3)}, // ADC0_IN3
    {PC_11, ADC_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF3)}, // ADC0_IN4
    {PC_10, ADC_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF3)}, // ADC0_IN5
    {PC_9 , ADC_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF3)}, // ADC0_IN6
    {PC_8 , ADC_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF3)}, // ADC0_IN7
    {NC,   NC,    0}
};


//*** SERIAL ***
const PinMap PinMap_UART_TX[] = {
    {PA_13, UART_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF0)},
    {PC_2,  UART_1, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF0)},
    {NC,    NC,     0}
};

const PinMap PinMap_UART_RX[] = {
    {PA_14, UART_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF0)},
    {PC_3,  UART_1, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_PULLUP, Px_AFSR_AF0)},
    {NC,    NC,     0}
};

//*** I2C ***
const PinMap PinMap_I2C_SDA[] = {
    {PA_10, I2C_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)},
    {NC,    NC,    0}
};

const PinMap PinMap_I2C_SCL[] = {
    {PA_9, I2C_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)},
    {NC,    NC,    0}
};

//*** SPI ***

const PinMap PinMap_SPI_SCLK[] = {
    {PA_6 , SPI_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)},
    {PB_1 , SPI_1, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)},
    {PC_12, SPI_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {PA_12, SPI_1, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {NC   , NC   , 0}
};

const PinMap PinMap_SPI_MOSI[] = {
    {PA_8 , SPI_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)},
    {PB_3 , SPI_1, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)},
    {PC_10, SPI_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {PA_14, SPI_1, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {NC   , NC   , 0}
};

const PinMap PinMap_SPI_MISO[] = {
    {PA_7 , SPI_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)},
    {PB_2 , SPI_1, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)},
    {PC_11, SPI_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {PA_13, SPI_1, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {NC   , NC   , 0}
};

const PinMap PinMap_SPI_SSEL[] = {
    {PA_5 , SPI_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)},
    {PB_0 , SPI_1, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)},
    {PC_13, SPI_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {PA_11, SPI_1, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {NC   , NC   , 0}
};

const PinMap PinMap_PWM[] = {
    {PA_0 , PWM_6, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF2)},
    {PA_1 , PWM_7, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF2)},
    {PA_5 , PWM_2, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {PA_7 , PWM_4, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {PA_8 , PWM_5, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {PA_9 , PWM_6, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {PA_10, PWM_7, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF3)},
    {PC_0 , PWM_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF2)},
    {PC_2 , PWM_2, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF2)},
    {PC_4 , PWM_4, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF2)},
    {PC_5 , PWM_5, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF2)},
    {PC_8 , PWM_0, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)},
    {PC_10, PWM_2, WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF2)},
    {NC   , NC   , WIZ_PIN_DATA(WIZ_MODE_AF, WIZ_GPIO_NOPULL, Px_AFSR_AF0)}
};

