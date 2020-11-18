/*****************************************************************************
* File Name  : i2s_master.h
*
* Description: This file contains definitions of constants and structures for
*              the I2S Master implementation using the SPI and Smart I/O.
*
*******************************************************************************
* Copyright (2020), Cypress Semiconductor Corporation.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and 
* foreign), United States copyright laws and international treaty provisions. 
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the 
* Cypress source code and derivative works for the sole purpose of creating 
* custom software in support of licensee product, such licensee product to be
* used only in conjunction with Cypress's integrated circuit as specified in the
* applicable agreement. Any reproduction, modification, translation, compilation,
* or representation of this Software except as specified above is prohibited 
* without the express written permission of Cypress.
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the Right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use
* of Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use as critical components in any products 
* where a malfunction or failure may reasonably be expected to result in 
* significant injury or death ("ACTIVE Risk Product"). By including Cypress's 
* product in a ACTIVE Risk Product, the manufacturer of such system or application
* assumes all risk of such use and in doing so indemnifies Cypress against all
* liability. Use of this Software may be limited by and subject to the applicable
* Cypress software license agreement.
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