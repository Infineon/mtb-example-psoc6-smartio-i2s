/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Smart I/O I2S Master (TX)
*              for ModusToolbox.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2020-2023, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cycfg.h"
#include "cy_retarget_io.h"

#include "wave.h"
#include "i2s_master.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* Master Clock (MCLK) Settings */
#define MCLK_FREQ_HZ        4095000u    /* in Hz */
#define MCLK_DUTY_CYCLE     50.0f       /* in %  */

/* PWM MCLK Pin */
#define MCLK_PIN            P5_6

/* Debounce delay for the button */
#define DEBOUNCE_DELAY_MS   10u         /* in ms */
#define I2S_PRIORITY 7UL

/*******************************************************************************
* Function Prototypes
********************************************************************************/
static void i2s_interrupt_handler(void);
static void i2s_callback(void);

/*******************************************************************************
* Global Variables
********************************************************************************/
/* HAL Objects */
cyhal_pwm_t mclk_pwm;

/* I2S Master Configuration */
stc_i2s_master_config_t i2s_config = 
{
    .word_length = 16
};

/* I2S Master Context */
static stc_i2s_master_context_t i2s_context;


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CM4 CPU. It does the following:
*    1. Initialize the hardware
*    2. Check for button presses. If pressed, plays the record
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize the User LED */
    cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    /* Initialize the User Button */
    cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);

    /* Initialize the Master Clock with a PWM */
    cyhal_pwm_init(&mclk_pwm, MCLK_PIN, NULL);
    cyhal_pwm_set_duty_cycle(&mclk_pwm, MCLK_DUTY_CYCLE, MCLK_FREQ_HZ);
    cyhal_pwm_start(&mclk_pwm);

    /* Setup I2S/SPI Interrupt */
    cyhal_system_set_isr(SPI_I2S_IRQ, SPI_I2S_IRQ, I2S_PRIORITY, i2s_interrupt_handler);
    NVIC_EnableIRQ(SPI_I2S_IRQ);

    /* Initialize the I2S Master */
    I2S_Master_Init(SPI_I2S_HW, SMARTIO_HW, &i2s_config, &i2s_context);
    I2S_Master_RegisterCallback(i2s_callback, &i2s_context);
    I2S_Master_Enable(&i2s_context);

    for (;;)
    {
        /* Check if the button was pressed */
        if (cyhal_gpio_read(CYBSP_USER_BTN) == CYBSP_BTN_PRESSED)
        {
            /* Delay to debounce the button press */
            cyhal_system_delay_ms(DEBOUNCE_DELAY_MS);

            /* Turn LED on */
            cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);

            /* Start playing */
            I2S_Master_Write(wave_data, WAVE_SIZE, &i2s_context);
        }
    }
}

/*******************************************************************************
* Function Name: i2s_interrupt_handler
********************************************************************************
* Summary:
* Interrupt handler for the I2S Master.
*
*******************************************************************************/
static void i2s_interrupt_handler(void)
{
    I2S_Master_Interrupt(&i2s_context);
}

/*******************************************************************************
* Function Name: i2s_callback
********************************************************************************
* Summary:
* I2S callback function. Turns the LED off.
*
*******************************************************************************/
static void i2s_callback(void)
{
    /* Turn LED off */
    cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
}

/* [] END OF FILE */
