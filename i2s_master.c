/*****************************************************************************
* File Name  : i2s_master.c
*
* Description: This file contains function definitions for implementing the
*              I2S Master interface.
*
*******************************************************************************
* Copyright 2020-2022, Cypress Semiconductor Corporation (an Infineon company) or
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
#include "i2s_master.h"

/*******************************************************************************
*                                 Local Functions
*******************************************************************************/
static void I2S_Master_WriteUntilFull(stc_i2s_master_context_t *context);

/*******************************************************************************
*                                 Global Variables
*******************************************************************************/
/** SPI Configuration */
const cy_stc_scb_spi_config_t i2s_spi_config = 
{
    .spiMode = CY_SCB_SPI_MASTER,
    .subMode = CY_SCB_SPI_MOTOROLA,
    .sclkMode = CY_SCB_SPI_CPHA0_CPOL1,
    .oversample = 4,
    .rxDataWidth = 16UL,
    .txDataWidth = 16UL,
    .enableMsbFirst = true,
    .enableInputFilter = false,
    .enableFreeRunSclk = false,
    .enableMisoLateSample = true,
    .enableTransferSeperation = false,
    .ssPolarity = ((CY_SCB_SPI_ACTIVE_LOW << CY_SCB_SPI_SLAVE_SELECT0) | \
                                         (CY_SCB_SPI_ACTIVE_LOW << CY_SCB_SPI_SLAVE_SELECT1) | \
                                         (CY_SCB_SPI_ACTIVE_LOW << CY_SCB_SPI_SLAVE_SELECT2) | \
                                         (CY_SCB_SPI_ACTIVE_LOW << CY_SCB_SPI_SLAVE_SELECT3)),
    .enableWakeFromSleep = false,
    .rxFifoTriggerLevel = 31UL,
    .rxFifoIntEnableMask = 0UL,
    .txFifoTriggerLevel = 31UL,
    .txFifoIntEnableMask = 0UL,
    .masterSlaveIntEnableMask = 0UL,
};

/** Smart I/O Configuration */
static const cy_stc_smartio_lutcfg_t i2s_smartio_lutCfg0 = 
{
    .tr0 = CY_SMARTIO_LUTTR_CHIP0,
    .tr1 = CY_SMARTIO_LUTTR_CHIP0,
    .tr2 = CY_SMARTIO_LUTTR_CHIP0,
    .opcode = CY_SMARTIO_LUTOPC_COMB,
    .lutMap = 128,
};
static const cy_stc_smartio_lutcfg_t i2s_smartio_lutCfg1 = 
{
    .tr0 = CY_SMARTIO_LUTTR_CHIP3,
    .tr1 = CY_SMARTIO_LUTTR_CHIP3,
    .tr2 = CY_SMARTIO_LUTTR_CHIP3,
    .opcode = CY_SMARTIO_LUTOPC_COMB,
    .lutMap = 128,
};
static const cy_stc_smartio_lutcfg_t i2s_smartio_lutCfg2 = 
{
    .tr0 = CY_SMARTIO_LUTTR_CHIP2,
    .tr1 = CY_SMARTIO_LUTTR_CHIP2,
    .tr2 = CY_SMARTIO_LUTTR_CHIP2,
    .opcode = CY_SMARTIO_LUTOPC_GATED_OUT,
    .lutMap = 128,
};
static const cy_stc_smartio_lutcfg_t i2s_smartio_lutCfg3 = 
{
    .tr0 = CY_SMARTIO_LUTTR_DU_OUT,
    .tr1 = CY_SMARTIO_LUTTR_LUT3_OUT,
    .tr2 = CY_SMARTIO_LUTTR_LUT3_OUT,
    .opcode = CY_SMARTIO_LUTOPC_GATED_OUT,
    .lutMap = 90,
};
static const cy_stc_smartio_ducfg_t i2s_smartio_duCfg = 
{
    .tr0 = CY_SMARTIO_DUTR_LUT1_OUT,
    .tr1 = CY_SMARTIO_DUTR_LUT2_OUT,
    .tr2 = CY_SMARTIO_DUTR_DU_OUT,
    .data0 = CY_SMARTIO_DUDATA_ZERO,
    .data1 = CY_SMARTIO_DUDATA_DATAREG,
    .opcode = CY_SMARTIO_DUOPC_INCR_WRAP,
    .size = CY_SMARTIO_DUSIZE_6,
    .dataReg = 63,
};
static const cy_stc_smartio_config_t i2s_smartio_config = 
{
    .clkSrc = CY_SMARTIO_CLK_DIVACT,
    .bypassMask = 0u|CY_SMARTIO_CHANNEL1|0u|0u|CY_SMARTIO_CHANNEL4|CY_SMARTIO_CHANNEL5|CY_SMARTIO_CHANNEL6|CY_SMARTIO_CHANNEL7,
    .ioSyncEn = 0u|0u|0u|0u|0u|0u|0u|0u,
    .chipSyncEn = 0u|0u|0u|0u|0u|0u|0u|0u,
    .lutCfg0 = &i2s_smartio_lutCfg0,
    .lutCfg1 = &i2s_smartio_lutCfg1,
    .lutCfg2 = &i2s_smartio_lutCfg2,
    .lutCfg3 = &i2s_smartio_lutCfg3,
    .lutCfg4 = NULL,
    .lutCfg5 = NULL,
    .lutCfg6 = NULL,
    .lutCfg7 = NULL,
    .duCfg = &i2s_smartio_duCfg,
    .hldOvr = false,
};

/*******************************************************************************
* Function Name: I2S_Master_Init
****************************************************************************//**
*
* Initialize the I2S Master block.
*
* \param spi_base
* The pointer to the SPI.
*
* \param smartio_base
* The pointer to the Smart I/O.
*
* \param config
* The pointer to the configuration structure
*
* \param context
* The pointer to the context structure \ref stc_i2s_master_context_t allocated
* by the user. The structure is used during the I2S Master operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
* \return 
* Returns the status of this operation. If I2S_MASTER_SUCCESS is not received, 
* the operation failed.
*
*******************************************************************************/
en_i2s_master_status_t I2S_Master_Init(CySCB_Type *spi_base,
                                       SMARTIO_PRT_Type *smartio_base,
                                       stc_i2s_master_config_t const *config,
                                       stc_i2s_master_context_t *context)
{
    cy_en_smartio_status_t smartio_status;
    cy_en_scb_spi_status_t spi_status;

    if (context == NULL || spi_base == NULL || smartio_base == NULL)
    {
        return I2S_MASTER_BAD_PARAM;
    }

    /* Set variables to the context */
    context->spi_base = spi_base;
    context->smartio_base = smartio_base;
    context->spi_width = config->word_length;

    /* Init the Smart I/O block */
    smartio_status = Cy_SmartIO_Init(smartio_base, &i2s_smartio_config);
    if (smartio_status != CY_SMARTIO_SUCCESS)
    {
        return I2S_MASTER_HW_FAILED;
    }

    /* Init the SPI block */
    spi_status = Cy_SCB_SPI_Init(spi_base, &i2s_spi_config, NULL);
    if (spi_status != CY_SCB_SPI_SUCCESS)
    {
        return I2S_MASTER_HW_FAILED;
    }

    /* Clear callback */
    context->callback = NULL;
    
    return I2S_MASTER_SUCCESS;
}

/*******************************************************************************
* Function Name: I2S_Master_Deinit
****************************************************************************//**
*
* De-initialize the I2S Master.
*
* \param context
* The pointer to the context structure \ref stc_i2s_master_context_t allocated
* by the user. The structure is used during the I2S Master operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
* \return 
* Returns the status of this operation. If I2S_MASTER_SUCCESS is not received, 
* the operation failed.
*
*******************************************************************************/
en_i2s_master_status_t I2S_Master_Deinit(stc_i2s_master_context_t *context)
{
    if (context == NULL)
    {
        return I2S_MASTER_BAD_PARAM;
    }

    /* De-init hardware blocks */
    Cy_SmartIO_Deinit(context->smartio_base);
    Cy_SCB_SPI_DeInit(context->spi_base);

    /* Clear variables in context */
    context->callback = NULL;
    context->spi_base = NULL;
    context->smartio_base = NULL;

    return I2S_MASTER_SUCCESS;
}

/*******************************************************************************
* Function Name: I2S_Master_Enable
****************************************************************************//**
*
* Enable the I2S Master.
*
* \param context
* The pointer to the context structure \ref stc_i2s_master_context_t allocated
* by the user. The structure is used during the I2S Master operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
* \return 
* Returns the status of this operation. If I2S_MASTER_SUCCESS is not received, 
* the operation failed.
*
*******************************************************************************/
en_i2s_master_status_t I2S_Master_Enable(stc_i2s_master_context_t *context)
{
    if (context == NULL)
    {
        return I2S_MASTER_BAD_PARAM;
    }

    /* Enable SPI and Smart I/O */
    Cy_SCB_SPI_Enable(context->spi_base);
    Cy_SmartIO_Enable(context->smartio_base);

    return I2S_MASTER_SUCCESS;
}

/*******************************************************************************
* Function Name: I2S_Master_Disable
****************************************************************************//**
*
* Disable the I2S Master.
*
* \param context
* The pointer to the context structure \ref stc_i2s_master_context_t allocated
* by the user. The structure is used during the I2S Master operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
* \return 
* Returns the status of this operation. If I2S_MASTER_SUCCESS is not received, 
* the operation failed.
*
*******************************************************************************/
en_i2s_master_status_t I2S_Master_Disable(stc_i2s_master_context_t *context)
{
    if (context == NULL)
    {
        return I2S_MASTER_BAD_PARAM;
    }

    /* Disable SPI and Smart I/O */
    Cy_SmartIO_Disable(context->smartio_base);
    Cy_SCB_SPI_Disable(context->spi_base, NULL);

    return I2S_MASTER_SUCCESS;
}

/*******************************************************************************
* Function Name: I2S_Master_RegisterCallback
****************************************************************************//**
*
* Register the callback to be executed on end of transmission.
*
* \param callback
* Function to be executed.
*
* \param context
* The pointer to the context structure \ref stc_i2s_master_context_t allocated
* by the user. The structure is used during the I2S Master operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void I2S_Master_RegisterCallback(cb_i2s_master_callback_t callback,
                                 stc_i2s_master_context_t *context)
{
    context->callback = callback;
}

/*******************************************************************************
* Function Name: I2S_Master_IsBusy
****************************************************************************//**
*
* Return if the I2S Master is transmitting data.
*
* \param context
* The pointer to the context structure \ref stc_i2s_master_context_t allocated
* by the user. The structure is used during the I2S Master operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
* \return
* True if busy, otherwise false.
*
*******************************************************************************/
bool I2S_Master_IsBusy(stc_i2s_master_context_t *context)
{
    return (Cy_SCB_SPI_GetNumInTxFifo(context->spi_base) > 0);
}

/*******************************************************************************
* Function Name: I2S_Master_Clear
****************************************************************************//**
*
* Clear internal FIFO.
*
* \param context
* The pointer to the context structure \ref stc_i2s_master_context_t allocated
* by the user. The structure is used during the I2S Master operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void I2S_Master_Clear(stc_i2s_master_context_t *context)
{
    Cy_SCB_SPI_ClearTxFifo(context->spi_base);
}

/*******************************************************************************
* Function Name: I2S_Master_Write
****************************************************************************//**
*
* Write data to the I2S TX FIFO. This function is non-blocking.
*
* \param frame
* Pointer to the frame to be transmitted.
*
* \param length
* Size of the frame.
*
* \param context
* The pointer to the context structure \ref stc_i2s_master_context_t allocated
* by the user. The structure is used during the I2S Master operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void I2S_Master_Write(const void *frame, size_t length, stc_i2s_master_context_t *context)
{
    context->tx_buff = frame;
    context->tx_length = length;

    /* Disable SPI TX interrupt */
    Cy_SCB_SetTxInterruptMask(context->spi_base, 0u);
    I2S_Master_WriteUntilFull(context);

    /* Enable the interrupt */
    Cy_SCB_SetTxInterruptMask(context->spi_base, CY_SCB_TX_INTR_LEVEL);
}

/*******************************************************************************
* Function Name: I2S_Master_Interrupt
****************************************************************************//**
*
* Interrupt handler for the I2S Master.
*
* \param context
* The pointer to the context structure \ref stc_i2s_master_context_t allocated
* by the user. The structure is used during the I2S Master operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void I2S_Master_Interrupt(stc_i2s_master_context_t *context)
{
    /* Disable SPI TX interrupt */
    Cy_SCB_SetTxInterruptMask(context->spi_base, 0u);
    /* Write till full */
    I2S_Master_WriteUntilFull(context);
        
    if (context->tx_length == 0)
    {
        if (context->callback != NULL)
        {
            context->callback();
        }
    }
    else
    {
        /* Enable the interrupt */
        Cy_SCB_SetTxInterruptMask(context->spi_base, CY_SCB_TX_INTR_LEVEL);
    }

    Cy_SCB_ClearTxInterrupt(context->spi_base, CY_SCB_TX_INTR_LEVEL);
}

/*******************************************************************************
* Function Name: I2S_Master_WriteUntilFull
****************************************************************************//**
*
* Write data to the internal FIFO till full. It handles alignment for 8/16/32 
* bits.
*
* \param context
* The pointer to the context structure \ref stc_i2s_master_context_t allocated
* by the user. The structure is used during the I2S Master operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
static void I2S_Master_WriteUntilFull(stc_i2s_master_context_t *context)
{
    /* The I2S frame channel length is fixed to 32 bits. 
     * Need to fill ZEROs in the remaining bits */
    if (context->spi_width <= 8)
    {
        /* Cast buffer */
        const int8_t *cast_buffer = (const int8_t *) context->tx_buff;
        
        /* Write to the SPI buffer till full */
        while (context->tx_length > 0 && Cy_SCB_SPI_GetNumInTxFifo(context->spi_base) < (I2S_MASTER_FIFO_DEPTH-1))
        {
            int16_t val = ((int16_t) (*cast_buffer)) << 8;
            Cy_SCB_SPI_Write(context->spi_base, val);
            Cy_SCB_SPI_Write(context->spi_base, 0x0000);
            ++cast_buffer;
            --(context->tx_length);
        }
        /* Update the current buffer pointer */
        context->tx_buff = (void*) cast_buffer;
    }
    else if (context->spi_width <= 16)
    {
        /* Cast buffer */
        const int16_t *cast_buffer = (const int16_t *) context->tx_buff;
        
        /* Write to the SPI buffer till full */
        while (context->tx_length > 0 && Cy_SCB_SPI_GetNumInTxFifo(context->spi_base) < (I2S_MASTER_FIFO_DEPTH-1))
        {
            int16_t val = ((int16_t) (*cast_buffer));
            Cy_SCB_SPI_Write(context->spi_base, val);
            Cy_SCB_SPI_Write(context->spi_base, 0x0000);
            ++cast_buffer;
            --(context->tx_length);            
        }

        /* Update the current buffer pointer */
        context->tx_buff = (void*) cast_buffer;
    }
    else
    {
        /* Cast buffer */
        const int32_t *cast_buffer = (const int32_t *) context->tx_buff;

        /* Write to the SPI buffer till full */
        while (context->tx_length > 0 && Cy_SCB_SPI_GetNumInTxFifo(context->spi_base) < (I2S_MASTER_FIFO_DEPTH-1))
        {
            int32_t val = ((int32_t) (*cast_buffer));
            Cy_SCB_SPI_Write(context->spi_base, CY_HI16(val));
            Cy_SCB_SPI_Write(context->spi_base, CY_LO16(val));
            ++cast_buffer;
            --(context->tx_length);            
        }

        /* Update the current buffer pointer */
        context->tx_buff = (void*) cast_buffer;
    }
}