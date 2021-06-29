/*****************************************************************************
* File Name  : i2s_master.h
*
* Description: This file contains definitions of constants and structures for
*              the I2S Master implementation using the SPI and Smart I/O.
*
*******************************************************************************
* Copyright 2020-2021, Cypress Semiconductor Corporation (an Infineon company) or
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
#ifndef I2S_MASTER_H_
#define I2S_MASTER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "cy_pdl.h"

/*******************************************************************************
*                              Enumerated Types
*******************************************************************************/
typedef enum
{
    /** Operation completed successfully */
    I2S_MASTER_SUCCESS   = 0u,

    /** One ore more input parameters are invalid */
    I2S_MASTER_BAD_PARAM = 1u,

    /** Failed to init one of the internal hardware blocks  */
    I2S_MASTER_HW_FAILED = 2u,

} en_i2s_master_status_t;

/*******************************************************************************
*                                 API Constants
*******************************************************************************/
#define I2S_MASTER_FIFO_DEPTH       (64u)

/*******************************************************************************
*                              Type Definitions
*******************************************************************************/
typedef void (* cb_i2s_master_callback_t)(void);

/** Config Structure */
typedef struct
{
    uint32_t word_length;
} stc_i2s_master_config_t;

/** Context Structure */
typedef struct
{
    // Constants
    uint32_t spi_width;
    CySCB_Type *spi_base;
    SMARTIO_PRT_Type *smartio_base;
    cb_i2s_master_callback_t callback;

    // Variables
    volatile size_t tx_length;
    volatile const void *tx_buff;
} stc_i2s_master_context_t;


/*******************************************************************************
*                            Function Prototypes
*******************************************************************************/
en_i2s_master_status_t I2S_Master_Init(CySCB_Type *spi_base,
                                       SMARTIO_PRT_Type *smartio_base,
                                       stc_i2s_master_config_t const *config,
                                       stc_i2s_master_context_t *context);
en_i2s_master_status_t I2S_Master_Deinit(stc_i2s_master_context_t *context);
en_i2s_master_status_t I2S_Master_Enable(stc_i2s_master_context_t *context);
en_i2s_master_status_t I2S_Master_Disable(stc_i2s_master_context_t *context);
void I2S_Master_RegisterCallback(cb_i2s_master_callback_t callback,
                                 stc_i2s_master_context_t *context);
bool I2S_Master_IsBusy(stc_i2s_master_context_t *context);
void I2S_Master_Clear(stc_i2s_master_context_t *context);
void I2S_Master_Write(const void *frame, size_t length, stc_i2s_master_context_t *context);
void I2S_Master_Interrupt(stc_i2s_master_context_t *context);

#endif /* I2S_MASTER_H_ */
