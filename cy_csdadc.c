/***************************************************************************//**
* \file cy_csdadc.c
* \version 2.0
*
* 
* This file provides the ADC function implementation of the CSD HW block.
*
********************************************************************************
* \copyright
* Copyright 2018-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#include "cy_device_headers.h"
#include "cy_syslib.h"
#include "cy_sysclk.h"
#include "cy_csdadc.h"
#include "cy_gpio.h"
#include "cy_csd.h"

#if defined(CY_IP_MXCSDV2)


/*******************************************************************************
* Function Prototypes - internal functions
*******************************************************************************/
/**
* \cond SECTION_CAPSENSE_INTERNAL
* \addtogroup group_capsense_internal
* \{
*/
static void Cy_CSDADC_ClearChannels(
                cy_stc_csdadc_context_t * context);
static void Cy_CSDADC_SetAdcChannel(
                uint32_t chId, 
                uint32_t state, 
                const cy_stc_csdadc_context_t * context);
static void Cy_CSDADC_DsInitialize(
                const cy_stc_csdadc_config_t * config, 
                cy_stc_csdadc_context_t * context);
static void Cy_CSDADC_Configure(
                cy_stc_csdadc_context_t * context);
static void Cy_CSDADC_SetClkDivider(
                const cy_stc_csdadc_context_t * context);
static void Cy_CSDADC_StartFSM(
                uint32_t measureMode, 
                cy_stc_csdadc_context_t * context);
static uint8_t Cy_CSDADC_GetNextCh(
                uint8_t currChId, 
                const cy_stc_csdadc_context_t * context);
static uint32_t Cy_CSDADC_StartAndWait(
                uint32_t measureMode, 
                cy_stc_csdadc_context_t * context);

/** \}
* \endcond */


/*******************************************************************************
* Local definition
*******************************************************************************/
/* Definitions for the init bit (bit 0) of the CSDADC status byte */
#define CY_CSDADC_INIT_NEEDED                       (0u)
#define CY_CSDADC_INIT_DONE                         (1u)
#define CY_CSDADC_INIT_MASK                         (0x01u)
/* Definitions for the conversion mode bit (bit 1) of the CSDADC status byte */
#define CY_CSDADC_CONV_MODE_BIT_POS                 (1u)
#define CY_CSDADC_CONV_MODE_MASK                    (0x02u)
/* Definitions for the busy bit (bit 2) of the CSDADC status byte */
#define CY_CSDADC_STATUS_BIT_POS                    (2u)
#define CY_CSDADC_STATUS_IDLE                       (0u)
#define CY_CSDADC_STATUS_BUSY                       (4u)
#define CY_CSDADC_STATUS_BUSY_MASK                  (0x04u)
/* Definitions for the overflow bit (bit 3) of the CSDADC status byte */
#define CY_CSDADC_OVERFLOW_MASK                     (0x08u)
/* Definitions for the stop bits (bit 8 - 9) of the CSDADC status byte */
#define CY_CSDADC_STOP_BITS_POS                     (8u)
#define CY_CSDADC_STOP_BITS_MASK                    (0x300u)

#define CY_CSDADC_RES_8_MAX_VAL                     ((uint32_t)(1uL << 8u) - 1u)
#define CY_CSDADC_RES_10_MAX_VAL                    ((uint32_t)(1uL << 10u) - 1u)
#define CY_CSDADC_RES_8_PLUS_1_MAX_VAL              (511u)
#define CY_CSDADC_RES_10_PLUS_1_MAX_VAL             (2047u)

#define CY_CSDADC_CAL_WATCHDOG_CYCLES_NUM           (0x0000FFFFu)

#define CY_CSDADC_CHAN_DISCONNECT                   (0u)
#define CY_CSDADC_CHAN_CONNECT                      (1u)
#define CY_CSDADC_VDDA_CHANNEL_MASK                 (0x80u)
#define CY_CSDADC_VBUSB_CHANNEL_MASK                (0x40u)

/* Cref common capacity in fF (can vary up to 25% for different devices) */
#define CY_CSDADC_CREF                              (21500u)
#define CY_CSDADC_KILO                              (1000u)
#define CY_CSDADC_MEGA                              (1000000u)
/* IdacB Leg3 LSB current in pA */
#define CY_CSDADC_IDAC_LSB                          (37500u)
#define CY_CSDADC_IDAC_MAX                          (127u)
#define CY_CSDADC_IDACB_CONFIG                      (0x04000080u)

#define CY_CSDADC_MIN_SNSCLK_DIVIDER                (4u)
#define CY_CSDADC_MAX_SNSCLK_DIVIDER                (0xFFFu)
#define CY_CSDADC_MAX_SNSCLK_CYCLES                 (0xFFu)

/* Default filter delay */
#define CY_CSDADC_FILTER_DELAY_DEFAULT              (2u)
#define CY_CSDADC_CSD_REG_CONFIG_FILTER_DELAY_Pos   (4u)

/* The reference voltage macros */
#define CY_CSDADC_VREF_IN_SRSS                      (800u)
#define CY_CSDADC_VREF_IN_PASS                      (1200u)

#define CY_CSDADC_VREF_CALIB_BASE_0_8               (800u)
#define CY_CSDADC_VREF_CALIB_BASE_1_2               (1164u)
#define CY_CSDADC_VREF_CALIB_BASE_1_6               (1600u)
#define CY_CSDADC_VREF_CALIB_BASE_2_1               (2133u)
#define CY_CSDADC_VREF_CALIB_BASE_2_6               (2600u)

#define CY_CSDADC_VREF_DESIRED_1200                 (1200u)
#define CY_CSDADC_VDDA_LIMITATION_2200              (2200u)
#define CY_CSDADC_VDDA_LIMITATION_2750              (2749u)

#define CY_CSDADC_PERCENTAGE_100                    (100u)
#define CY_CSDADC_VREF_TRIM_MAX_DEVIATION           (20u)
#define CY_CSDADC_VREF_GAIN_MAX                     (32u)
#define CY_CSDADC_VREF_VDDA_MIN_DIFF                (600u)

/* CSD HW block CONFIG register definitions */
#define CY_CSDADC_CSD_REG_CONFIG_INIT               (0x80001000uL)
#define CY_CSDADC_CSD_REG_CONFIG_DEFAULT            (CY_CSDADC_CSD_REG_CONFIG_INIT | \
                                                    (uint32_t)((uint32_t)CY_CSDADC_FILTER_DELAY_DEFAULT << CY_CSDADC_CSD_REG_CONFIG_FILTER_DELAY_Pos))

#define CY_CSDADC_ADC_RES_OVERFLOW_MASK             (0x40000000uL)
#define CY_CSDADC_ADC_RES_ABORT_MASK                (0x80000000uL)
#define CY_CSDADC_ADC_RES_HSCMPPOL_MASK             (0x00010000uL)
#define CY_CSDADC_ADC_RES_VALUE_MASK                (0x0000FFFFuL)
#define CY_CSDADC_UNDERFLOW_LIMIT                   (8000u)

/* CSD_INTR register masks */
#define CY_CSDADC_CSD_INTR_SAMPLE_MSK               (0x00000001uL)
#define CY_CSDADC_CSD_INTR_INIT_MSK                 (0x00000002uL)
#define CY_CSDADC_CSD_INTR_ADC_RES_MSK              (0x00000100uL)
#define CY_CSDADC_CSD_INTR_ALL_MSK                  (CY_CSDADC_CSD_INTR_SAMPLE_MSK | \
                                                     CY_CSDADC_CSD_INTR_INIT_MSK | \
                                                     CY_CSDADC_CSD_INTR_ADC_RES_MSK)
/* CSD_INTR_MASK register masks */
#define CY_CSDADC_CSD_INTR_MASK_SAMPLE_MSK          (0x00000001uL)
#define CY_CSDADC_CSD_INTR_MASK_INIT_MSK            (0x00000002uL)
#define CY_CSDADC_CSD_INTR_MASK_ADC_RES_MSK         (0x00000100uL)
#define CY_CSDADC_CSD_INTR_MASK_CLEAR_MSK           (0x00000000uL)

/* Switch definitions */
#define CY_CSDADC_SW_HSP_DEFAULT                    (0x10000000uL)
#define CY_CSDADC_SW_HSN_DEFAULT                    (0x00100000uL)
#define CY_CSDADC_SW_HSP_GETINPOL                   (0x00010000uL)
#define CY_CSDADC_SW_HSN_GETINPOL                   (0x01000000uL)
#define CY_CSDADC_SW_SHIELD_DEFAULT                 (0x00000000uL)
#define CY_CSDADC_SW_SHIELD_VDDA2CSDBUSB            (0x00000100uL)
#define CY_CSDADC_SW_SHIELD_VDDA2CSDBUSC            (0x00010000uL)
#define CY_CSDADC_SW_BYP_DEFAULT                    (0x00110000uL)
#define CY_CSDADC_SW_BYP_VDDA_MEAS                  (0x00000000uL)
#define CY_CSDADC_SW_CMPP_DEFAULT                   (0x00000000uL)
#define CY_CSDADC_SW_CMPN_DEFAULT                   (0x00000000uL)
/* RefGen settings */
#define CY_CSDADC_REFGEN_GAIN_SHIFT                 (0x00000008uL)
#define CY_CSDADC_SW_REFGEN_SGR_SRSS                (0x10000000uL)
#define CY_CSDADC_SW_REFGEN_SGRP_PASS               (0x00100000uL)
#define CY_CSDADC_REFGEN_NORM                       (0x00000041UL)
#define CY_CSDADC_SW_AMUBUF_NORM                    (0x00000000uL)
/* HSCOMP definitions */
#define CY_CSDADC_HSCMP_AZ_DEFAULT                  (0x80000001uL)
#define CY_CSDADC_STATUS_HSCMP_OUT_MASK             (0x00000010uL)

#define CY_CSDADC_SW_FWMOD_DEFAULT                  (0x01100000uL)
#define CY_CSDADC_SW_FWTANK_DEFAULT                 (0x01100000uL)

#define CY_CSDADC_CSD_CONFIG_DEFAULT  {\
    .config         = CY_CSDADC_CSD_REG_CONFIG_DEFAULT,\
    .spare          = 0x00000000uL,\
    .status         = 0x00000000uL,\
    .statSeq        = 0x00000000uL,\
    .statCnts       = 0x00000000uL,\
    .statHcnt       = 0x00000000uL,\
    .resultVal1     = 0x00000000uL,\
    .resultVal2     = 0x00000000uL,\
    .adcRes         = 0x00000000uL,\
    .intr           = 0x00000000uL,\
    .intrSet        = 0x00000000uL,\
    .intrMask       = 0x00000000uL,\
    .intrMasked     = 0x00000000uL,\
    .hscmp          = 0x00000001uL,\
    .ambuf          = 0x00000001uL,\
    .refgen         = 0x00000001uL,\
    .csdCmp         = 0x00000000uL,\
    .swRes          = 0x00000000uL,\
    .sensePeriod    = 0x00000000uL,\
    .senseDuty      = 0x00000000uL,\
    .swHsPosSel     = 0x00000000uL,\
    .swHsNegSel     = 0x00000000uL,\
    .swShieldSel    = 0x00000000uL,\
    .swAmuxbufSel   = 0x00000000uL,\
    .swBypSel       = 0x00000000uL,\
    .swCmpPosSel    = 0x00000000uL,\
    .swCmpNegSel    = 0x00000000uL,\
    .swRefgenSel    = 0x00000000uL,\
    .swFwModSel     = 0x00000000uL,\
    .swFwTankSel    = 0x00000000uL,\
    .swDsiSel       = 0x00000000uL,\
    .ioSel          = 0x00000000uL,\
    .seqTime        = 0x00000000uL,\
    .seqInitCnt     = 0x00000000uL,\
    .seqNormCnt     = 0x00000000uL,\
    .adcCtl         = 0x00000000uL,\
    .seqStart       = 0x00000000uL,\
    .idacA          = 0x00000000uL,\
    .idacB          = 0x00000000uL,\
    }

#define  CY_CSDADC_MEASMODE_OFF                     (0u)
#define  CY_CSDADC_MEASMODE_VREF                    (1u)
#define  CY_CSDADC_MEASMODE_VREFBY2                 (2u)
#define  CY_CSDADC_MEASMODE_VIN                     (3u)
#define  CY_CSDADC_ADC_CTL_MEAS_POS                 (16u)

#define CY_CSDADC_STATUS_FSM_MASK                   (0xF0u)
#define CY_CSDADC_STATUS_FSM_IDLE                   (0x00u)
#define CY_CSDADC_STATUS_CALIBPH1                   (0x10u)
#define CY_CSDADC_STATUS_CALIBPH2                   (0x20u)
#define CY_CSDADC_STATUS_CALIBPH3                   (0x30u)
#define CY_CSDADC_STATUS_CONVERTING                 (0x40u)

#define CY_CSDADC_FSM_ABORT                         (0x08u)
#define CY_CSDADC_FSM_AZ0_SKIP                      (0x100u)
#define CY_CSDADC_FSM_AZ1_SKIP                      (0x200u)

#define CY_CSDADC_FSM_NO_AZ_SKIP                    (0u)
#define CY_CSDADC_FSM_AZ_SKIP_DEFAULT               (CY_CSDADC_FSM_AZ0_SKIP)
#define CY_CSDADC_FSM_START                         (0x00000001uL)

/*******************************************************************************
* Function Name: Cy_CSDADC_Init
****************************************************************************//**
*
* Captures the CSD HW block and configures it to the default state,
* is called by the application program prior to calling any other 
* function of the middleware.
*
* The function:
* * verifies input parameters
* * copies the configuration structure to the context structure
* * disconnects all input channels
* * verifies the CSD HW block state
* * locks the CSD HW block
* * writes the default configuration to the CSD HW block
* * configures the CSDADC middleware to the default state.
*
* \param config
* The pointer to the CSDADC configuration structure.
*
* \param context
* The pointer to the CSDADC context structure.
*
* \return
* The function returns the status of its operation.
* * CY_CSDADC_SUCCESS     - The function performed successfully.
* * CY_CSDADC_HW_LOCKED   - The CSD HW block is already in use by another CSD
*                           function. The CSDADC cannot be initialized 
*                           right now. The user waits until 
*                           the CSD HW block passes to the idle state.
* * CY_CSDADC_BAD_PARAM   - The context pointer is NULL.
*                           The function was not performed.
*
* \funcusage
* 
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_Initialization
* The 'cy_csdadc_context' variable used as the parameter of the
* Cy_CSDADC_Init() and Cy_CSDADC_Enable() functions is declared
* on the application layer according to the example below:<br>
* \snippet csdadc/snippet/main.c snippet_csdadc_context_declaration
* The 'CSD_csdadc_config' variable used as the parameter of the
* Cy_CSDADC_Init() function is declared in the cycfg_capsense.h file if the
* Device Configurator tool is used.
* Refer to the \ref group_csdadc_configuration section for details. 
* The 'CSDADC_csdadc_config' variable is declared and initialized on the
* application layer if the third party IDE is used for development. 
* 
* The CSDADC_ISR_cfg variable is declared by the application
* program according to the examples below:<br>
* For CM0+ core:
* \snippet csdadc/snippet/main.c snippet_m0p_adc_interrupt_source_declaration
*
* For CM4 core:
* \snippet csdadc/snippet/main.c snippet_m4_adc_interrupt_source_declaration
* 
* The CSDADC interrupt handler is declared by the application program
* according to the example below:
* \snippet csdadc/snippet/main.c snippet_CSDADC_Interrupt
* 
* The CSDADC_HW is the pointer to the base register address of 
* the CSD HW block. A macro for the pointer is in the cycfg_peripherals.h 
* file defined as \<Csd_Personality_Name\>_HW. If no name is specified,
* the default name is used csd_\<Block_Number\>_csd_\<Block_Number\>_HW.
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_Init(
                const cy_stc_csdadc_config_t * config,
                cy_stc_csdadc_context_t * context)
{
    cy_en_csdadc_status_t result = CY_CSDADC_BAD_PARAM;

    CY_ASSERT_L1(NULL != config);
    CY_ASSERT_L1(NULL != context);

    if ((NULL != config) && (NULL != context))
    {
        /* Copy the configuration structure to the context */
        context->cfgCopy = *config;
        /* Disconnect all CSDADC channels */
        Cy_CSDADC_ClearChannels(context);
        /* Capture the CSD HW block for the ADC functionality */
        result = Cy_CSDADC_Restore(context);
        if (CY_CSDADC_SUCCESS == result)
        {
            /* Wait for the CSD HW block will enter the mode */
            Cy_SysLib_DelayUs((uint16_t)context->cfgCopy.csdInitTime);
            /* Initialize CSDADC data structure */
            Cy_CSDADC_DsInitialize(config, context);
        }
        else
        {
            result = CY_CSDADC_HW_LOCKED;
        }
    }

    return (result);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_Enable
****************************************************************************//**
* 
* Initializes the CSDADC firmware modules. 
* 
* The Cy_CSDADC_Init() function is to be called and the CSD HW block interrupt
* is to be configured prior to calling this function.
* The following steps are performed for proper CSDADC initialization:
* * Capture the CSD HW block and initialize it to the default state. 
* * Initialize the CSDADC interrupt.
* * Initialize the CSDADC firmware modules.
*
* This function is called by the application program prior to calling
* any other function of the middleware. 
* The function:
* * Configures the CSD HW block to perform CSDADC conversions
* * Calibrates the CSDADC for an accurate measurement
*
* Any subsequent call of this function repeats an initialization process 
* except for the data structure initialization. Therefore, changing the
* middleware configuration from the application program is possible. 
* Do this by writing registers to the data structure and calling this 
* function again. This is also done inside the Cy_CSDACD_WriteConfig() 
* function, when configuration must be updated.
*
* \param context
* The pointer to the CSDADC context structure.
*
* \return
* Returns the status of the initialization process. If CY_RET_SUCCESS is not
* received, some of the initialization fails.
*
* \funcusage
* 
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_Initialization
* The 'cy_csdadc_context' variable used as the parameter of the
* Cy_CSDADC_Init() and Cy_CSDADC_Enable() functions is declared
* on the application layer according to the examples below:<br>
* \snippet csdadc/snippet/main.c snippet_csdadc_context_declaration
* The 'CSD_csdadc_config' variable used as the parameter of the
* Cy_CSDADC_Init() function is declared in the cycfg_capsense.h file if the
* Device Configurator tool is used.
* Refer to the \ref group_csdadc_configuration section for details.
* 
* The CSDADC_ISR_cfg variable should be declared by the application
* program according to the examples below:<br>
* For CM0+ core:
* \snippet csdadc/snippet/main.c snippet_m0p_adc_interrupt_source_declaration
*
* For CM4 core:
* \snippet csdadc/snippet/main.c snippet_m4_adc_interrupt_source_declaration
* 
* The CSDADC interrupt handler is declared by the application program
* according to the example below:
* \snippet csdadc/snippet/main.c snippet_CSDADC_Interrupt
* 
* The CSDADC_HW is the pointer to the base register address of 
* the CSD HW block. A macro for the pointer is in the cycfg_peripherals.h 
* file defined as \<Csd_Personality_Name\>_HW. If no name specified,  
* the default name is used csd_\<Block_Number\>_csd_\<Block_Number\>_HW.
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_Enable(cy_stc_csdadc_context_t * context)
{
    cy_en_csdadc_status_t result = CY_CSDADC_BAD_PARAM;

    CY_ASSERT_L1(NULL != context);

    if (NULL != context)
    {
        /* Configure HW block */
        Cy_CSDADC_Configure(context);

        /* Calibrate CSDADC */
        result = Cy_CSDADC_Calibrate(context);
    }
    return result;
}


/*******************************************************************************
* Function Name: Cy_CSDADC_DeInit
****************************************************************************//**
*
* Stops the middleware operation and releases the CSD HW block.
*
* No input voltage conversion can be executed when the middleware is stopped. 
* This function should be called only when no conversion is in progress. 
* I.e. Cy_CSDADC_IsEndConversion() returns a non-busy status.
*
* After it is stopped, the CSD HW block may be reconfigured by the 
* application program or other middleware for any other use. 
*
* When the middleware operation is stopped by the Cy_CSDADC_DeInit()
* function, a subsequent call of the Cy_CSDADC_Init() function repeats the 
* initialization process. Calling the Cy_CSDADC_Enable() function is not 
* needed a second time. However, to implement the time-multiplexed mode 
* (sharing the CSD HW Block between multiple middleware), the 
* Cy_CSDADC_Save()/Cy_CSDADC_Restore() functions should be used 
* instead of Cy_CSDADC_DeInit()/Cy_CSDADC_Init() functions.
*
* Besides releasing the CSD HW block, this function also configures all input 
* channels to the default state.
*
* \param context
* The pointer to the CSDADC context structure.
*
* \return
* The function returns the status of its operation.
* * CY_CSDADC_SUCCESS   - The function performed successfully.
* * CY_CSDADC_HW_LOCKED - The CSD HW block is busy with ADC conversion. The CSDADC 
*                         can't be de-initialized right now. The user should 
*                         wait until the CSD HW block passes to the idle 
*                         state or use Cy_CSDADC_StopConvert().
* * CY_CSDADC_BAD_PARAM - A context pointer is equal to NULL.
*                         The function was not performed.
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_DeInit(cy_stc_csdadc_context_t * context)
{
    cy_en_csdadc_status_t result = CY_CSDADC_SUCCESS;

    CY_ASSERT_L1(NULL != context);

    if (NULL !=context)
    {
        if (CY_CSDADC_SUCCESS == Cy_CSDADC_IsEndConversion(context))
        {
            if (CY_CSD_SUCCESS != Cy_CSD_DeInit(context->cfgCopy.base, CY_CSD_ADC_KEY, context->cfgCopy.csdCxtPtr))
            {
                result = CY_CSDADC_HW_LOCKED;
            }
            else
            {
                Cy_CSDADC_ClearChannels(context);
                context->status = (uint16_t)CY_CSDADC_INIT_NEEDED;
            }
        }
        else
        {
            result = CY_CSDADC_HW_BUSY;
        }
    }
    else
    {
        result = CY_CSDADC_BAD_PARAM;
    }

    return (result);
}

/*******************************************************************************
* Function Name: Cy_CSDADC_WriteConfig
****************************************************************************//**
*
* Updates the CSDADC middleware with the desired configuration.
*
* This function sets the desired CSDADC middleware configuration.
* The function performs the following:
* * Verifies input parameters. If any of them is equal to NULL, the function
*   does not perform further operations
* * Initializes the CSDADC context structure in the accordance with
*   the specified configuration
* * Initializes the CSD HW block registers with data, passed through
*   the specified configuration
* * Disconnects inputs and sets the CSD HW block to the default state for
*   CSDADC operations
* * Enables CSDADC operations like the Cy_CSDADC_Enable() function. To start
*   a conversion, the user should call the Cy_CSDADC_StartConvert() function.
* * Returns a status code regarding the function execution result
*
* \warning
* This function must be called only in the CSD HW block idle state. 
* A call of this function during a conversion will yield an unpredictable 
* CSD HW block behavior.
*
* \note
* This function as the Cy_CSDADC_Enable() function can be called only after 
* the CSDADC middleware initialization. To do this, use 
* the Cy_CSDADC_Init() function and the CSDADC interrupt enabling as it 
* is shown in the code example for the Cy_CSDADC_Enable() function.
*
* \param config
* The pointer to the CSDADC configuration structure.
*
* \param context
* The pointer to the CSDADC context structure.
*
* \return
* The function returns the status of its operation.
* * CY_CSDADC_SUCCESS   - The function performed successfully.
* * CY_CSDADC_BAD_PARAM - A context pointer or config pointer is equal to NULL. 
*                         The function was not performed.
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_WriteConfig(
                const cy_stc_csdadc_config_t * config,
                cy_stc_csdadc_context_t * context)
{
    cy_en_csdadc_status_t result = CY_CSDADC_SUCCESS;

    CY_ASSERT_L1(NULL != config);
    CY_ASSERT_L1(NULL != context);

    if ((NULL == config) || (NULL ==context))
    {
        result = CY_CSDADC_BAD_PARAM;
    }
    else
    {
        /* Copy the configuration structure to the context */
        context->cfgCopy = * config;
        /* Disconnect all CSDADC channels */
        Cy_CSDADC_ClearChannels(context);
        /* Initialize CSDADC data structure */
        Cy_CSDADC_DsInitialize(config, context);
        /* Configure and calibrate CSDADC */
        result = Cy_CSDADC_Enable(context);
    }

    return (result);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_RegisterCallback
****************************************************************************//**
*
* Registers a callback function, which notifies that a callback event 
* occurred in the CSDADC middleware.
*
* \param callbackFunction
* The pointer to the callback function.
*
* \param context
* The pointer to the CSDADC context structure \ref cy_stc_csdadc_context_t.
*
* \return
* The function returns the status of its operation.
* * CY_CSDADC_SUCCESS      - The processing performed successfully.
* * CY_CSDADC_BAD_PARAM    - The input parameter is invalid.
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_RegisterCallback(
                cy_csdadc_callback_t callbackFunction,
                cy_stc_csdadc_context_t * context)
{
    cy_en_csdadc_status_t retVal = CY_CSDADC_SUCCESS;

    if((NULL != callbackFunction) && (NULL != context))
    {
        context->ptrEOCCallback = callbackFunction;
    }
    else
    {
        retVal = CY_CSDADC_BAD_PARAM;
    }

    return(retVal);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_UnRegisterCallback
****************************************************************************//**
*
* This function unregisters the CSDADC middleware callback.
*
* \param context
* The pointer to the CSDADC context structure \ref cy_stc_csdadc_context_t.
*
* \return
* The function returns the status of its operation.
* * CY_CSDADC_SUCCESS      - The processing performed successfully.
* * CY_CSDADC_BAD_PARAM    - The input parameter is invalid.
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_UnRegisterCallback(
                cy_stc_csdadc_context_t * context)
{
    cy_en_csdadc_status_t retVal = CY_CSDADC_SUCCESS;

    if(NULL != context)
    {
        context->ptrEOCCallback = NULL;
    }
    else
    {
        retVal = CY_CSDADC_BAD_PARAM;
    }

    return(retVal);

}


/*******************************************************************************
* Function Name: Cy_CSDADC_Configure
****************************************************************************//**
*
* Configures the CSD HW block to be used as an ADC.
*
* Configures the IDACB, internal switches, REFGEN, and HSCOMP. This
* function is used by the Cy_CSDADC_Restore() API to set the CSD HW block
* in the same state as before the Cy_CSDADC_Save() API was called.
*
* \param context
* The pointer to the CSDADC context structure.
*
*******************************************************************************/
static void Cy_CSDADC_Configure(cy_stc_csdadc_context_t * context)
{
    uint32_t interruptState;
    uint32_t newRegValue;
    CSD_Type * ptrCsdBaseAdd = context->cfgCopy.base;

    /* Configure clocks */
    Cy_CSDADC_SetClkDivider(context);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SENSE_PERIOD, (uint32_t)context->snsClkDivider - 1u);

    /* Configure the IDAC */
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_CONFIG, CY_CSDADC_CSD_REG_CONFIG_DEFAULT);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_IDACB, CY_CSDADC_IDACB_CONFIG | context->cfgCopy.idac);

    /* Configure AZ Time */
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SEQ_TIME, (uint32_t)context->azCycles - 1u);

    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_CSDCMP, 0u);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_DSI_SEL, 0u);

    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SENSE_DUTY, 0u);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SEQ_INIT_CNT, 1u);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SEQ_NORM_CNT, 2u);

    /* Configure the block-level routing */
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_HS_P_SEL, CY_CSDADC_SW_HSP_DEFAULT);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_HS_N_SEL, CY_CSDADC_SW_HSN_DEFAULT);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_SHIELD_SEL, CY_CSDADC_SW_SHIELD_DEFAULT);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_CMP_P_SEL, CY_CSDADC_SW_CMPP_DEFAULT);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_CMP_N_SEL, CY_CSDADC_SW_CMPN_DEFAULT);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_FW_MOD_SEL, CY_CSDADC_SW_FWMOD_DEFAULT);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_FW_TANK_SEL, CY_CSDADC_SW_FWTANK_DEFAULT);
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_REFGEN_SEL, CY_CSDADC_SW_REFGEN_SGR_SRSS);

    interruptState = Cy_SysLib_EnterCriticalSection();
    newRegValue = Cy_CSD_ReadReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_BYP_SEL);
    newRegValue |= CY_CSDADC_SW_BYP_DEFAULT;
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_BYP_SEL, newRegValue);
    Cy_SysLib_ExitCriticalSection(interruptState);

    /* Config RefGen */
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_REFGEN, CY_CSDADC_REFGEN_NORM |
                                    ((uint32_t)(context->vRefGain) << CY_CSDADC_REFGEN_GAIN_SHIFT));
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_AMUXBUF_SEL, CY_CSDADC_SW_AMUBUF_NORM);

    /* Configure HSCOMP */
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_HSCMP, CY_CSDADC_HSCMP_AZ_DEFAULT);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_Wakeup
****************************************************************************//**
*
* 
* Resumes the middleware after CPU / System Deep Sleep.
*
* This function is used to resume the middleware operation after exiting 
* CPU / System Deep Sleep. After the CSD HW block has been powered off 
* and an extra delay is required to establish correct operation of 
* the CSD HW block.
*
* \param context
* The pointer to the CSDADC context structure.
*
* \return
* The function returns the status of its operation.
* * CY_CSDADC_SUCCESS   - The function performed successfully.
* * CY_CSDADC_BAD_PARAM - A context pointer is equal to NULL.
*                         The function was not performed.
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_Wakeup(const cy_stc_csdadc_context_t * context)
{
    cy_en_csdadc_status_t result = CY_CSDADC_SUCCESS;

    CY_ASSERT_L1(NULL != context);

    if (NULL !=context)
    {
        Cy_SysLib_DelayUs((uint16_t)context->cfgCopy.csdInitTime);
    }
    else
    {
        result = CY_CSDADC_BAD_PARAM;
    }

    return (result);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_DeepSleepCallback
****************************************************************************//**
*
* Handles CPU active to CPU / System Deep Sleep power mode transitions for the CSDADC
* middleware.
*
* Do not call this function directly from the application program. 
* Instead, use Cy_SysPm_CpuEnterDeepSleep() for CPU active to CPU / System Deep Sleep power mode
* transitions.
* \note
* After the CPU Deep Sleep transition, the device automatically goes 
* to System Deep Sleep if all conditions are fulfilled: another core is 
* in CPU Deep Sleep, all the peripherals are ready to System Deep Sleep, etc.
* (see details in the device TRM).
*
* For proper operation of the CSDADC middleware during CPU active to 
* CPU / System Deep Sleep mode transitions, a callback to this function is registered
* using the Cy_SysPm_RegisterCallback() function with the CY_SYSPM_DEEPSLEEP
* type. After the callback is registered, this function is called by the
*  Cy_SysPm_CpuEnterDeepSleep() function to prepare the middleware to the device
* power mode transition.
* 
* When this function is called with CY_SYSPM_CHECK_READY as the input, this
* function returns CY_SYSPM_SUCCESS if no conversion is in progress. Otherwise,
* CY_SYSPM_FAIL is returned. If the CY_SYSPM_FAIL status is returned, a device
* cannot change the power mode without completing the current conversion because
* a transition to CPU / System Deep Sleep during the conversion can disrupt the middleware
* operation.
* 
* For details of the SysPm types and macros, refer to the SysPm section of the 
* PDL documentation
* <a href="https://www.cypress.com/documentation/technical-reference-manuals/psoc-6-mcu-psoc-63-ble-architecture-technical-reference" 
* title="PDL API Reference" >PDL API Reference</a>.
* 
* \param callbackParams
* Refer to the description of the cy_stc_syspm_callback_params_t type in the
* Peripheral Driver Library documentation.
*
* \param mode
* Specifies mode cy_en_syspm_callback_mode_t.
*
* \return
* Returns the status cy_en_syspm_status_t of the operation requested 
* by the mode parameter:
* * CY_SYSPM_SUCCESS  - CPU / System Deep Sleep power mode can be entered.
* * CY_SYSPM_FAIL     - CPU / System Deep Sleep power mode cannot be entered.
*
*******************************************************************************/
cy_en_syspm_status_t Cy_CSDADC_DeepSleepCallback(
                cy_stc_syspm_callback_params_t * callbackParams,
                cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_SUCCESS;
    cy_stc_csdadc_context_t * csdadcCxt = (cy_stc_csdadc_context_t *) callbackParams->context;

    if (CY_SYSPM_CHECK_READY == mode)
    { /* Actions that should be done before entering CPU / System Deep Sleep mode */
        if (CY_CSD_ADC_KEY == Cy_CSD_GetLockStatus(csdadcCxt->cfgCopy.base, csdadcCxt->cfgCopy.csdCxtPtr))
        {
            if (CY_CSDADC_SUCCESS != Cy_CSDADC_IsEndConversion(csdadcCxt))
            {
                retVal = CY_SYSPM_FAIL;
            }
        }
    }
    else
    { /* Does nothing in other modes */
    }

    return(retVal);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_Save
****************************************************************************//**
*
* Saves the state of the CSDADC MW so the functionality can be restored
* using the Cy_CSDACD_Restore() function.
*
* \warning
* The function operates only in the idle state of the CSDADC.
*
* This function, along with the Cy_CSDACD_Restore() function, is specifically 
* designed for ease of use and supports time multiplexing of the CSD HW block 
* among multiple middleware. When the CSD HW block is shared by two or more 
* middleware, this function can be used to save the current state of 
* the CSD HW block and the CSDADC middleware prior to releasing the CSD HW block 
* for use by other middleware.
* 
* This function performs the same tasks as the Cy_CSDADC_DeInit() function 
* and is kept for API consistency among middleware. Use the 
* Cy_CSDADC_Save()/Cy_CSDADC_Restore() functions to implement 
* Time-multiplexed mode instead of the Cy_CSSADC_DeInit()/Cy_CSDADC_Init()
* functions for further compatibility.
* This function:
* * Checks whether CSDADC is in the idle state. If the CSDADC is busy, 
*   the function does nothing. <br>
*   In the idle state:
*     * Releases the CSD HW block
*     * Disconnects channel input pins from analog muxbus B and configures
*       them to the default state.
*
* \param context
* The pointer to the CSDADC context structure.
*
* \return
* Returns the status of the process. If CY_CSDADC_SUCCESS is not received,
* the save process fails and a retry may be required.
*
* \funcusage
*
* An example of sharing the CSD HW block with the CapSense and CSDADC middleware.<br>
* Declares the CapSense_ISR_cfg variable:
* \snippet csdadc/snippet/main.c snippet_m4_capsense_interrupt_source_declaration
* 
* Declares the CSDADC_ISR_cfg variable:
* \snippet csdadc/snippet/main.c snippet_m4_adc_interrupt_source_declaration
* 
* Defines the CapSense interrupt handler:
* \snippet csdadc/snippet/main.c snippet_CapSense_Interrupt
* 
* Defines the CSDADC interrupt handler:
* \snippet csdadc/snippet/main.c snippet_CSDADC_Interrupt
* 
* The part of the main.c FW flow:
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_TimeMultiplex
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_Save(cy_stc_csdadc_context_t * context)
{
    cy_en_csdadc_status_t result = CY_CSDADC_BAD_PARAM;
    cy_en_csd_status_t initStatus = CY_CSD_LOCKED;

    CY_ASSERT_L1(NULL != context);

    if (NULL !=context)
    {
        if (CY_CSDADC_SUCCESS == Cy_CSDADC_IsEndConversion(context))
        {
            /* Release the CSD HW block */
            initStatus = Cy_CSD_DeInit(context->cfgCopy.base, CY_CSD_ADC_KEY, context->cfgCopy.csdCxtPtr);

            if (CY_CSD_SUCCESS == initStatus)
            {
                /* Disconnect input channels pins from analog busses */
                Cy_CSDADC_ClearChannels(context);

                result = CY_CSDADC_SUCCESS;
            }
        }
        else
        {
            result = CY_CSDADC_HW_BUSY;
        }
    }

    return result;
}


/*******************************************************************************
* Function Name: Cy_CSDADC_GetResult
****************************************************************************//**
*
* Returns the most recent result of a specified channel as an ADC code.
*
* The function neither initiates a conversion nor converts the ADC result 
* in millivolts. Instead it returns the most recent conversion result 
* on specified input as an ADC code. The valid range for result is 
* from 0 to 2^ CSDADCresolution - 1.
*
* \param chId
* An ID of the input channel to read the most recent result. Acceptable values
* are between 0 and (chNum - 1).
*
* \param context
* The pointer to the CSDADC context structure.
*
* \return
* Specifies the CSDADC input channel code result between 0 and 2^resolution - 1.
* If a channel number is invalid, CY_CSDADC_MEASUREMENT_FAILED is returned 
* because this function returns a number (not a status).
*
*******************************************************************************/
uint32_t Cy_CSDADC_GetResult(
                uint32_t chId, 
                const cy_stc_csdadc_context_t * context)
{
    uint32_t tmpRetVal = CY_CSDADC_MEASUREMENT_FAILED;

    CY_ASSERT_L1(NULL != context);

    if((NULL != context) && (chId < context->cfgCopy.numChannels))
    {
        tmpRetVal = context->adcResult[chId].code;
    }
    return tmpRetVal;
}


/*******************************************************************************
* Function Name: Cy_CSDADC_GetResultVoltage
****************************************************************************//**
*
* Returns the the most recent result of a specified channel in millivolts.
*
* The function does not initiate a conversion. Instead it returns 
* the most recent conversion result on specified input in millivolts.
*
* \param chId
* An ID of the input channel to read the most recent result. Acceptable values
* are between 0 and (chNum - 1).
*
* \param context
* The pointer to the CSDADC context structure.
*
* \return
* Specifies the CSDADC input channel result in millivolts.
* If a channel number is invalid or the pointer to the CSDADC context is
* equal to NULL, \ref CY_CSDADC_MEASUREMENT_FAILED is returned.
*
*******************************************************************************/
uint32_t Cy_CSDADC_GetResultVoltage(
                uint32_t chId, 
                const cy_stc_csdadc_context_t * context)
{
    uint32_t tmpRetVal = CY_CSDADC_MEASUREMENT_FAILED;

    CY_ASSERT_L1(NULL != context);

    if((NULL != context) && (chId < context->cfgCopy.numChannels ))
    {
        tmpRetVal = context->adcResult[chId].mVolts;
    }
    return tmpRetVal;
}


/*******************************************************************************
* Function Name: Cy_CSDADC_Restore
****************************************************************************//**
*
* Resumes the middleware operation if the Cy_CSDADC_Save() function was
* called previously.
*
* This function, along with the Cy_CSDADC_Save() function, is specifically 
* designed for ease of use and supports time multiplexing of the CSD HW block 
* among multiple middleware. When the CSD HW block is shared by two or more
* middleware, this function can be used to restore the previous state of 
* the CSD HW block and CSDADC middleware saved using the 
* Cy_CSDACD_Save() function.
* This function performs a sub-set of initialization tasks and is used into the 
* Cy_CSDADC_Init() function.
*
* \param context
* The pointer to the CSDADC middleware context structure.
*
* \return
* Returns the status of the resume process. If CY_CSDADC_SUCCESS is not 
* received, the resume process fails and a retry may be required.
*
* \funcusage
* 
* An example of sharing the CSD HW block by CapSense and CSDADC middleware:
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_TimeMultiplex
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_Restore(cy_stc_csdadc_context_t * context)
{
    uint32_t watchdogCounter;

    cy_en_csdadc_status_t result = CY_CSDADC_BAD_PARAM;
    cy_en_csd_key_t mvKey;
    cy_en_csd_status_t initStatus = CY_CSD_LOCKED;
    CSD_Type * ptrCsdBaseAdd = context->cfgCopy.base;
    cy_stc_csd_context_t * ptrCsdCxt = context->cfgCopy.csdCxtPtr;
    const cy_stc_csd_config_t csdCfg = CY_CSDADC_CSD_CONFIG_DEFAULT;

    /* Number of cycle of one for() loop */
    const uint32_t cyclesPerLoop = 5u;
    /* Timeout in microseconds */
    const uint32_t watchdogTimeoutUs = 10000u;

    if (NULL !=context)
    {
        /* Get the CSD HW block status */
        mvKey = Cy_CSD_GetLockStatus(ptrCsdBaseAdd, ptrCsdCxt);
        if(CY_CSD_NONE_KEY == mvKey)
        {
            Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR_MASK, CY_CSDADC_CSD_INTR_MASK_CLEAR_MSK);
            Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SEQ_START, CY_CSDADC_FSM_ABORT);

            /* Initialize Watchdog Counter to prevent a hang */
            watchdogCounter = (watchdogTimeoutUs * (context->cfgCopy.cpuClk / CY_CSDADC_MEGA)) / cyclesPerLoop;
            do
            {
                initStatus = Cy_CSD_GetConversionStatus(ptrCsdBaseAdd, ptrCsdCxt);
                watchdogCounter--;
            }
            while((CY_CSD_BUSY == initStatus) && (0u != watchdogCounter));

            /* Clear all interrupt pending requests */
            Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR, CY_CSDADC_CSD_INTR_ALL_MSK);
            (void)Cy_CSD_ReadReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR);

            /* Capture the CSD HW block for the ADC functionality */
            initStatus = Cy_CSD_Init(ptrCsdBaseAdd, &csdCfg, CY_CSD_ADC_KEY, ptrCsdCxt);

            if (CY_CSD_SUCCESS == initStatus)
            {
                /* Configure the CSD HW block */
                Cy_CSDADC_Configure(context);
                result = CY_CSDADC_SUCCESS;
            }
            else
            {
                result = CY_CSDADC_HW_LOCKED;
            }
        }
    }
    return (result);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_MeasureVdda
****************************************************************************//**
*
* The function measures a VDDA voltage and returns the result in millivolts.
*
* This function measures supply voltage (VDDA) of device without need 
* of explicitly connecting VDDA to a GPIO input of ADC. This capability 
* can be used to measure battery voltage and/or change VDDA dependent 
* parameters of the ADC during run-time.
* 
* The conversion is initiated only if the CSDADC is in IDLE state and a context 
* parameter is not NULL. This function is blocking function, i.e. waits for ADC 
* conversion to be completed prior to returning to caller.
*
* \param context
* The pointer to the CSDADC middleware context structure.
*
* \return
* The function returns measurement result, VDDA in millivolts.
* If the pointer to the CSDADC context is equal to NULL or ADC is not in IDLE
* state, \ref CY_CSDADC_MEASUREMENT_FAILED is returned.
*
*******************************************************************************/
uint32_t Cy_CSDADC_MeasureVdda(cy_stc_csdadc_context_t * context)
{
    CSD_Type * ptrCsdBaseAdd = context->cfgCopy.base;

    uint32_t interruptState;
    uint32_t newRegValue;

    uint32_t tmpRetVal = CY_CSDADC_MEASUREMENT_FAILED;
    uint32_t tmpResult;

    uint32_t timeVssa2Vref;
    uint32_t timeVdda2Vref;
    uint32_t timeRecover;

    CY_ASSERT_L1(NULL != context);

    if (NULL != context)
    {
        if(CY_CSDADC_SUCCESS == Cy_CSDADC_IsEndConversion(context))
        {
            /* Set the busy bit of the CSDADC status byte */
            context->status |= CY_CSDADC_STATUS_BUSY_MASK;

            /* Mask all CSD HW block interrupts (disable all interrupts) */
            Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR_MASK, CY_CSDADC_CSD_INTR_MASK_CLEAR_MSK);

            /* Clear all pending interrupts of the CSD HW block */
            Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR, CY_CSDADC_CSD_INTR_ALL_MSK);
            (void)Cy_CSD_ReadReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR);

            /* Disconnect channels if connected */
            if (CY_CSDADC_NO_CHANNEL != context->activeCh)
            {
                /* Disconnect existing input channel */
                Cy_CSDADC_ClearChannels(context);
                context->activeCh = CY_CSDADC_NO_CHANNEL;
            }

            /* Configure IDAC */
            Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_IDACB, CY_CSDADC_IDACB_CONFIG | context->cfgCopy.idac);

            /* Start CALIBPH1 */
            /* Start CSDADC conversion */
            tmpResult = Cy_CSDADC_StartAndWait((uint32_t)CY_CSDADC_MEASMODE_VREF, context);
            if (CY_CSDADC_MEASUREMENT_FAILED != tmpResult)
            {
                /* Select the result value */
                timeVssa2Vref = tmpResult & CY_CSDADC_ADC_RES_VALUE_MASK;

                /* Start CALIBPH2 */
                /* Set the mode and acquisition time and start CSDADC conversion */
                tmpResult = Cy_CSDADC_StartAndWait((uint32_t)CY_CSDADC_MEASMODE_VREFBY2, context);
                if (CY_CSDADC_MEASUREMENT_FAILED != tmpResult)
                {
                    /* Select the result value */
                    timeRecover = tmpResult & CY_CSDADC_ADC_RES_VALUE_MASK;

                    /* Disconnect amuxbusB, Connect VDDA to csdbusB */
                    interruptState = Cy_SysLib_EnterCriticalSection();
                    newRegValue = Cy_CSD_ReadReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_BYP_SEL);
                    newRegValue &= (uint32_t)(~CY_CSDADC_SW_BYP_DEFAULT);
                    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_BYP_SEL, newRegValue);
                    Cy_SysLib_ExitCriticalSection(interruptState);
                    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_SHIELD_SEL, CY_CSDADC_SW_SHIELD_VDDA2CSDBUSB);

                    /* Start CALIBPH3 */
                    tmpResult = Cy_CSDADC_StartAndWait((uint32_t)CY_CSDADC_MEASMODE_VIN, context);
                    if (CY_CSDADC_MEASUREMENT_FAILED != tmpResult)
                    {
                        /* Select the result value */
                        timeVdda2Vref = tmpResult & CY_CSDADC_ADC_RES_VALUE_MASK;
                        /* Reconnect amuxbusB, disconnect VDDA */
                        Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_SHIELD_SEL,
                                                                CY_CSDADC_SW_SHIELD_DEFAULT);
                        interruptState = Cy_SysLib_EnterCriticalSection();
                        newRegValue = Cy_CSD_ReadReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_BYP_SEL);
                        newRegValue |= CY_CSDADC_SW_BYP_DEFAULT;
                        Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SW_BYP_SEL, newRegValue);
                        Cy_SysLib_ExitCriticalSection(interruptState);

                        /* Calibrate timeVdda2Vref with Sink/Source mismatch with rounding */
                        timeVdda2Vref = ((((timeVdda2Vref << 1u) * timeRecover) + (timeVssa2Vref >> 1u)) /
                                                                                                    timeVssa2Vref);
                        /* Calculate Vdda and store it in the context structure */
                        tmpRetVal = context->vRefMv + (((context->vRefMv * timeVdda2Vref) + (timeVssa2Vref >> 1u)) /
                                                                                                    timeVssa2Vref);
                        context->vddaMv = (uint16_t)tmpRetVal;
                    }
                }
            }
            /* Set the idle status */
            context->status = (uint16_t)CY_CSDADC_INIT_MASK;
        }
    }
    return (tmpRetVal);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_MeasureAMuxB
****************************************************************************//**
*
* The function measures an AMUX-B voltage and returns the result in millivolts.
*
* This function measures a voltage on AMUX-B without need for explicitly 
* connecting a GPIO input to the CSDADC. This capability can be used to 
* measure an internal voltage connectable to AMUX-B. It is the responsibility 
* of the application program to establish connection between a voltage 
* source and AMUX-B prior to initiating a conversion with this function.
*
* \param context
* The pointer to the CSDADC middleware context structure.
*
* \return
* The function returns the result of an analog MuxBusB voltage measuring
* in millivolts. If the pointer to the CSDADC context is equal to NULL 
* or ADC is not in the idle state, CY_CSDADC_MEASUREMENT_FAILED is returned.
*
*******************************************************************************/
uint32_t Cy_CSDADC_MeasureAMuxB(cy_stc_csdadc_context_t * context)
{
    uint32_t tmpRetVal = CY_CSDADC_MEASUREMENT_FAILED;
    uint32_t polarity;
    cy_en_csdadc_status_t result = CY_CSDADC_SUCCESS;
    CSD_Type * ptrCsdBaseAdd = context->cfgCopy.base;

    CY_ASSERT_L1(NULL != context);

    if (NULL != context)
    {
        if(CY_CSDADC_SUCCESS == Cy_CSDADC_IsEndConversion(context))
        {
            /* Check whether CSDADC is configured */
            if ((uint16_t)CY_CSDADC_INIT_DONE != (context->status & (uint16_t)CY_CSDADC_INIT_MASK))
            {
                result = Cy_CSDADC_WriteConfig(&context->cfgCopy, context);
            }
            if (CY_CSDADC_SUCCESS == result)
            {
                /* Mask all CSD HW block interrupts (disable all interrupts) */
                Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR_MASK, CY_CSDADC_CSD_INTR_MASK_CLEAR_MSK);

                /* Clear all pending interrupts of the CSD HW block */
                Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR, CY_CSDADC_CSD_INTR_ALL_MSK);
                (void)Cy_CSD_ReadReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR);

                /* Disconnect channels if connected */
                if (CY_CSDADC_NO_CHANNEL != context->activeCh)
                {
                    /* Disconnect existing input channel */
                    Cy_CSDADC_ClearChannels(context);
                    context->activeCh = CY_CSDADC_NO_CHANNEL;
                }

                /* Configure IDAC */
                Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_IDACB, CY_CSDADC_IDACB_CONFIG | context->cfgCopy.idac);

                /* Set the busy bit of the CSDADC status byte */
                context->status |= CY_CSDADC_STATUS_BUSY_MASK;

                /* Start CSD sequencer */
                tmpRetVal = Cy_CSDADC_StartAndWait((uint32_t)CY_CSDADC_MEASMODE_VIN, context);
                if (CY_CSDADC_MEASUREMENT_FAILED != tmpRetVal)
                {
                    /* Select the polarity bit */
                    polarity = tmpRetVal & CY_CSDADC_ADC_RES_HSCMPPOL_MASK;
                    /* Select the result value */
                    tmpRetVal &= CY_CSDADC_ADC_RES_VALUE_MASK;
                    /* HSCMP polarity is 0:sink, 1:source */
                    if(0u != polarity) /* Sourcing */
                    {
                        /* Saturate result at tVssa2Vref */
                        tmpRetVal = (tmpRetVal > (uint32_t)context->tVssa2Vref) ? (uint32_t)context->tVssa2Vref : tmpRetVal;
                        /* Scale result to Resolution range with rounding*/
                        tmpRetVal = ((((uint32_t)context->tVssa2Vref - tmpRetVal) * context->codeMax) +
                                     ((uint32_t)context->tFull >> 1u)) / (uint32_t)context->tFull;
                    }
                    else /* Sinking */
                    {
                        if (CY_CSDADC_RANGE_VDDA == context->cfgCopy.range)
                        {
                            /* Scale result with sink/source mismatch with rounding */
                            tmpRetVal = (((uint32_t)((uint32_t)context->tRecover << 1u) * tmpRetVal) +
                                                ((uint32_t)context->tVssa2Vref >> 1u)) / (uint32_t)context->tVssa2Vref;
                            /* Saturate result at t_Vdda2Vref*/
                            tmpRetVal = (tmpRetVal > (uint32_t)context->tVdda2Vref) ? (uint32_t)context->tVdda2Vref :
                                                                                                            tmpRetVal;
                            /* Scale result to Resolution range with rounding */
                            tmpRetVal = ((((uint32_t)context->tVssa2Vref + tmpRetVal) * context->codeMax)  +
                                                         ((uint32_t)context->tFull >> 1u)) / (uint32_t)context->tFull;
                        }
                        else
                        {
                            /* In vref mode, we are not supposed to be sinking. Saturate */
                            tmpRetVal = ((uint32_t)context->tVssa2Vref * context->codeMax) / (uint32_t)context->tFull;
                        }
                    }
                    /* Scale result to mV with rounding and store it */
                    tmpRetVal = ((((uint32_t) context->vMaxMv) * tmpRetVal) +
                                                     ((uint32_t)context->codeMax >> 1u)) / ((uint32_t)context->codeMax);
                    context->vBusBMv = (uint16_t)(tmpRetVal);
                }

                /* Clear all status bits except the initialization bit */
                context->status &= (uint16_t)CY_CSDADC_INIT_MASK;
            }
        }
    }

    return (tmpRetVal);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_SetClkDivider
****************************************************************************//**
*
* Sets the modulator clock and then starts it.
*
* Do not call this function directly by the application program.
* It is used by initialization widget APIs to configure and
* enable the modulator clock.
*
* \param context
* The pointer to the CSDADC middleware context structure.
*
*******************************************************************************/
static void Cy_CSDADC_SetClkDivider(const cy_stc_csdadc_context_t * context)
{
    uint32_t dividerIndex = context->cfgCopy.periDivInd;
    cy_en_divider_types_t dividerType = (cy_en_divider_types_t) context->cfgCopy.periDivTyp;

    (void)Cy_SysClk_PeriphDisableDivider(dividerType, dividerIndex);
    if ((CY_SYSCLK_DIV_8_BIT == dividerType) || (CY_SYSCLK_DIV_16_BIT == dividerType))
    {
        (void)Cy_SysClk_PeriphSetDivider(dividerType, dividerIndex,
                                                    (uint32_t)context->cfgCopy.operClkDivider - 1u);
    }
    else
    {
        (void)Cy_SysClk_PeriphSetFracDivider(dividerType, dividerIndex,
                                                    (uint32_t)context->cfgCopy.operClkDivider - 1u, 0u);
    }
    (void)Cy_SysClk_PeriphEnableDivider(dividerType, dividerIndex);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_ClearChannels
****************************************************************************//**
*
* Resets all the CSDADC channels to disconnected state.
*
* The function goes through all the CSDADC channels and disconnects the pin
* and the analog muxbus B.  Sets the drive mode of the pin as well.
*
*******************************************************************************/
static void Cy_CSDADC_ClearChannels(cy_stc_csdadc_context_t * context)
{
    uint32_t chId;

    for (chId = 0u; chId < context->cfgCopy.numChannels; chId++)
    {
        Cy_CSDADC_SetAdcChannel(chId, CY_CSDADC_CHAN_DISCONNECT, context);
    }
    context->activeCh = CY_CSDADC_NO_CHANNEL;
}


/*******************************************************************************
* Function Name: Cy_CSDADC_DsInitialize
****************************************************************************//**
*
* Initializes the CSDADC data structure.
*
* The function makes the following tasks:
* * copies the configuration structure to the context
* * clears the result arrays
* * sets or calculates and trims Vref
* * calculates Vref gain
* * sets or calculates IDAC
* * calculates snsClkDivider
*
* \param config
* The pointer to the CSDADC middleware configuration structure.
*
* \param context
* The pointer to the CSDADC middleware context structure.
*
*******************************************************************************/
static void Cy_CSDADC_DsInitialize(
                const cy_stc_csdadc_config_t * config, 
                cy_stc_csdadc_context_t * context)
{
    uint32_t nMax;
    uint32_t vMax;
    uint32_t vRefDesired;
    uint32_t iDacGain;
    uint32_t chIndex;
    uint32_t snsClkDiv = CY_CSDADC_MIN_SNSCLK_DIVIDER;
    uint32_t codeMaxIdac;
    uint32_t vGain;

    /* Clear the result structure */
    for (chIndex = 0u; chIndex < CY_CSDADC_MAX_CHAN_NUM; chIndex++)
    {
            context->adcResult[chIndex].code = 0u;
            context->adcResult[chIndex].mVolts = 0u;
    }

    /* Clear the ptrEOCCallback */
    context->ptrEOCCallback = NULL;

    /* Choose VrefDesired depending on configured Vref value */
    if (0 > config->vref)
    {
        if (config->vdda < CY_CSDADC_VDDA_LIMITATION_2200)
        {
            vRefDesired = (uint32_t)CY_CSDADC_VREF_CALIB_BASE_1_2;
        }
        else if (config->vdda > CY_CSDADC_VDDA_LIMITATION_2750)
        {
            vRefDesired = (uint32_t)CY_CSDADC_VREF_CALIB_BASE_2_1;
        }
        else
        {
            vRefDesired = (uint32_t)CY_CSDADC_VREF_CALIB_BASE_1_6;
        }
    }
    else
    {
        vRefDesired = (uint32_t)config->vref;
        /* Check for Vref limitations */
        if (vRefDesired > ((uint32_t)config->vdda - CY_CSDADC_VREF_VDDA_MIN_DIFF))
        {
            vRefDesired = ((uint32_t)config->vdda - CY_CSDADC_VREF_VDDA_MIN_DIFF);
        }
    }

    /* Calculate Vref gain */
    vGain = (((uint32_t)CY_CSDADC_VREF_GAIN_MAX * (uint32_t)CY_CSDADC_VREF_IN_SRSS) - 1u) / (vRefDesired + 1u);
    if (vGain > (uint32_t)(CY_CSDADC_VREF_GAIN_MAX - 1u))
    {
        vGain = (uint32_t)CY_CSDADC_VREF_GAIN_MAX - 1u;
    }
    context->vRefGain = (uint8_t)vGain;

    /* Calculate Vref with rounding and store in the context structure */
    context->vRefMv = (uint16_t)((((uint32_t)CY_CSDADC_VREF_GAIN_MAX * CY_CSDADC_VREF_IN_SRSS) + ((vGain + 1u) >> 1u))
                                                                                                    / (vGain + 1u));

    /* Update nominal Vref to real Vref */
    context->vRefMv = (uint16_t)Cy_CSD_GetVrefTrim((uint32_t)context->vRefMv);

    /* Define Vmax depending on CSDADC range */
    if (CY_CSDADC_RANGE_VREF == config->range)
    {
        vMax = context->vRefMv;
        context->vMaxMv = (uint16_t)vMax;
    }
    else
    /* For 0-Vdda range determine what voltage is more Vdda-Vref or Vref */
    {
        vMax = (((config->vdda - context->vRefMv) > context->vRefMv) ?
                                                ((uint32_t)config->vdda - context->vRefMv) : (uint32_t)context->vRefMv);
        context->vMaxMv = config->vdda;
    }
    /* Max CSDADC code for IDAC calculation depends on resolution and must be multiplied by 2 for accuracy */
    if (CY_CSDADC_RESOLUTION_8BIT == config->resolution)
    {
        codeMaxIdac = CY_CSDADC_RES_8_PLUS_1_MAX_VAL;
    }
    else
    {
        codeMaxIdac = CY_CSDADC_RES_10_PLUS_1_MAX_VAL;
    }

    /* Depends on IDAC gain */
    /* Calculate IDAC gain with an appropriate operation sequence to avoid overflow */
    iDacGain = (config->periClk / codeMaxIdac) / config->operClkDivider;
    iDacGain = (((iDacGain * vMax) / CY_CSDADC_MEGA) * CY_CSDADC_CREF) / CY_CSDADC_IDAC_LSB;
    if (CY_CSDADC_IDAC_MAX < iDacGain)
    {
        iDacGain = CY_CSDADC_IDAC_MAX;
    }
    context->cfgCopy.idac = (uint8_t) iDacGain;

    /* Set init value for tVssa2Vref. It'll be corrected after calibration */
    context->tVssa2Vref = (uint16_t)codeMaxIdac;
    /* Set init value for tRecover. It'll be corrected after calibration */
    context->tRecover = (uint16_t)codeMaxIdac;
    /* Set init value for tVdda2Vref. It'll be corrected after calibration */
    nMax = (uint32_t)context->tVssa2Vref * ((uint32_t)context->cfgCopy.vdda - context->vRefMv);
    context->tVdda2Vref = (uint16_t)(nMax / context->vRefMv);
    /* Set init value for tFull. It'll be corrected after calibration */
    context->tFull = context->tVssa2Vref + context->tVdda2Vref;

    /* Max CSDADC code depends on resolution */
    if (CY_CSDADC_RESOLUTION_8BIT == config->resolution)
    {
        context->codeMax = (uint16_t)CY_CSDADC_RES_8_MAX_VAL;
    }
    else
    {
        context->codeMax = (uint16_t)CY_CSDADC_RES_10_MAX_VAL;
    }

    /* Calculate snsClkDivider as small as possible */
    /* Choose the max time interval */
    nMax = config->acqTime;
    if (config->azTime > nMax)
    {
        nMax = config->azTime;
    }
    nMax = (nMax * (config->periClk / CY_CSDADC_MEGA)) / config->operClkDivider;
    while (((nMax / snsClkDiv) >= CY_CSDADC_MAX_SNSCLK_CYCLES) && (snsClkDiv < CY_CSDADC_MAX_SNSCLK_DIVIDER))
    {
        snsClkDiv++;
    }
    context->acqCycles = (uint8_t)((config->acqTime * (config->periClk / CY_CSDADC_MEGA)) / config->operClkDivider / snsClkDiv);
    context->azCycles = (uint8_t)((config->azTime * (config->periClk / CY_CSDADC_MEGA)) / config->operClkDivider / snsClkDiv);
    context->snsClkDivider = (uint8_t)snsClkDiv;

    /* Set INIT done */
    context->status |= (uint16_t)CY_CSDADC_INIT_DONE;
}


/*******************************************************************************
* Function Name: Cy_CSDADC_SetAdcChannel
****************************************************************************//**
*
* Sets the given channel to the given state.
*
* Connects/disconnects the pin and the analog muxbus B. Sets the drive mode
* of the pin as well.
*
* \param chId  
* The ID of the channel to be set.
*
* \param state 
* The state in which the channel is to be put:
* * (0) CY_CSDADC_CHAN_DISCONNECT
* * (1) CY_CSDADC_CHAN_CONNECT
*
*******************************************************************************/
static void Cy_CSDADC_SetAdcChannel(
                uint32_t chId, 
                uint32_t state, 
                const cy_stc_csdadc_context_t * context)
{
    cy_stc_csdadc_ch_pin_t const ptr2adcIO = context->cfgCopy.ptrPinList[chId];
    uint32_t  interruptState= Cy_SysLib_EnterCriticalSection();
    switch (state)
    {
    case CY_CSDADC_CHAN_CONNECT:
        /* Connect AMuxBusB to the selected port */
        Cy_GPIO_SetHSIOM(ptr2adcIO.ioPcPtr, (uint32_t)ptr2adcIO.pin, HSIOM_SEL_AMUXB);
        /* Update port configuration register (drive mode) to High-Z Analog */
        Cy_GPIO_SetDrivemode(ptr2adcIO.ioPcPtr, (uint32_t)ptr2adcIO.pin, CY_GPIO_DM_ANALOG);
        break;

    /* Disconnection is a safe default state. Fall-through is intentional. */
    case CY_CSDADC_CHAN_DISCONNECT:
    default:
        /* Disconnect AMuxBusB from the selected port */
        Cy_GPIO_SetHSIOM(ptr2adcIO.ioPcPtr, (uint32_t)ptr2adcIO.pin, HSIOM_SEL_GPIO);
        break;
    }
    Cy_SysLib_ExitCriticalSection(interruptState);
}

/*******************************************************************************
* Function Name: Cy_CSDADC_StartFSM
****************************************************************************//**
*
* Starts the CSD state machine with correct parameters to initialize an ADC
* conversion.
*
* \param measureMode 
* The FSM mode:
* * (0) CY_CSDADC_MEASMODE_OFF
* * (1) CY_CSDADC_MEASMODE_VREF
* * (2) CY_CSDADC_MEASMODE_VREFBY2
* * (3) CY_CSDADC_MEASMODE_VIN
*
*******************************************************************************/
static void Cy_CSDADC_StartFSM(
                uint32_t measureMode, 
                cy_stc_csdadc_context_t * context)
{
    uint32_t tmpStartVal = (uint32_t)(measureMode << CY_CSDADC_ADC_CTL_MEAS_POS) | ((uint32_t)context->acqCycles - 1u);

    /* Set the mode and acquisition time */
    Cy_CSD_WriteReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_ADC_CTL, tmpStartVal);

    if(CY_CSDADC_MEASMODE_OFF == measureMode)
    {
        tmpStartVal = CY_CSDADC_FSM_ABORT;
    }
    /* This setting is used for MEASMODE_VREF, MEASMODE_VREFBY2, and MEASMODE_VIN */
    else
    {
        tmpStartVal = CY_CSDADC_FSM_AZ_SKIP_DEFAULT | CY_CSDADC_FSM_START;
    }
    /* Unmask ADC_RES interrupt (enable interrupt) */
    Cy_CSD_WriteReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_INTR_MASK, CY_CSDADC_CSD_INTR_ADC_RES_MSK);
    /* Start CSD sequencer */
    Cy_CSD_WriteReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_SEQ_START, tmpStartVal);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_StartAndWait
****************************************************************************//**
*
* Starts the CSD state machine and waits until the conversion ends.
*
* Starts the CSD state machine with correct parameters to initialize an ADC
* conversion and waits until the conversion ends. To prevent hanging,
* a program watchdog is used.
*
* \param measureMode 
* The FSM mode:
* * (1) CY_CSDADC_MEASMODE_VREF
* * (2) CY_CSDADC_MEASMODE_VREFBY2
* * (3) CY_CSDADC_MEASMODE_VIN
*
* \param context
* The pointer to the CSDADC middleware context structure.
*
* \return
* The function returns the result of the conversion. If an overflow occurs
* or the watchdog triggers, CY_CSDADC_MEASUREMENT_FAILED is returned.
*
*******************************************************************************/
static uint32_t Cy_CSDADC_StartAndWait(
                uint32_t measureMode, 
                cy_stc_csdadc_context_t * context)
{
    CSD_Type * ptrCsdBaseAdd = context->cfgCopy.base;

    uint32_t tmpStartVal = (uint32_t)(measureMode << CY_CSDADC_ADC_CTL_MEAS_POS) | ((uint32_t)context->acqCycles - 1u);
    uint32_t tmpRetVal = CY_CSDADC_MEASUREMENT_FAILED;
    uint32_t tmpResult;

    uint32_t watchdogAdcCounter = CY_CSDADC_CAL_WATCHDOG_CYCLES_NUM;

    /* start CSDADC conversion with desired the mode and acquisition time */
    Cy_CSD_WriteReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_ADC_CTL, tmpStartVal);
    Cy_CSD_WriteReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_SEQ_START,
                                                        CY_CSDADC_FSM_AZ_SKIP_DEFAULT | CY_CSDADC_FSM_START);

    /* Check for watchdog counter */
    while ((0u == (Cy_CSD_ReadReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR) & CY_CSDADC_CSD_INTR_MASK_ADC_RES_MSK))
                                                                                    &&  (0u != watchdogAdcCounter))
    {
        /* Wait until scan complete and decrement Watchdog Counter to prevent unending loop */
        watchdogAdcCounter--;
    }

    /* Clear all pending interrupts of the CSD HW block */
    Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR, CY_CSDADC_CSD_INTR_ALL_MSK);
    (void)Cy_CSD_ReadReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR);

    if (0u != watchdogAdcCounter)
    {
        /* Read ADC result and check for ADC_ABORT or ADC_OVERFLOW flags */
        tmpResult = Cy_CSD_ReadReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_ADC_RES);
        if (0u == (tmpResult & CY_CSDADC_ADC_RES_ABORT_MASK))
        {
            if (0u == (tmpResult & CY_CSDADC_ADC_RES_OVERFLOW_MASK))
            {
                tmpRetVal = tmpResult;
            }
        }
    }
    return (tmpRetVal);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_StartConvert
****************************************************************************//**
*
* Initiates an analog-to-digital conversion.
*
* Initializes the CSD HW block to perform an analog-to-digital conversion 
* on input channels specified by chMask.
* This is a non-blocking function. It initiates conversion only on the first 
* input channel and does not wait for the conversion to complete. 
* The conversion on subsequent channels are initiated by the interrupt service 
* routine of the CSDADC middleware. Therefore, the Cy_CSDADC_IsEndConversion() 
* function must be used to check the status of conversion to ensure 
* the previously initiated conversion is complete prior to initiating 
* other tasks. These include: reading the ADC result, initiation of a new 
* conversion on the same or a different channel, or calibration or 
* stopping a CSDADC conversion.
*
* In Single-shot mode, the CSDADC middleware performs one conversion 
* on all channels specified by the chMask argument and stops. 
* The Cy_CSDADC_IsEndConversion() function returns the CY_CSDADC_SUCCESS status
* after conversion on all channels completes. Otherwise, it returns the
* CY_CSDADC_HW_BUSY state. To get information on the latest conversion, 
* use the Cy_CSDADC_GetConversionStatus() function.
*
* In Continuous mode, this function continuously initiates conversions
* on channels specified by chMask (i.e. once a conversion is completed 
* on all channels specified by chMask, the CSDADC MW initiates a next set
* of conversions). The Cy_CSDADC_IsEndConversion() function 
* always returns the CY_CSDADC_HW_BUSY status. To get information on the latest 
* conversion, use the Cy_CSDADC_GetConversionStatus() functions. 

* \param mode
* The desired mode of conversion:
* * CY_CSDADC_SINGLE_SHOT - Only one conversion cycle of all chosen channels.
* * CY_CSDADC_CONTINUOUS - Continuous mode.
*
* \param chMask
* The bit mask with set bits of specified channels to convert.
*
* \param context
* The pointer to the CSDADC middleware context structure.
*
* \return
* The function returns the status of its operation.
* * CY_CSDADC_SUCCESS         - A conversion is started.
* * CY_CSDADC_HW_BUSY         - A conversion is not started. A previously 
*                               initiated conversion is in progress or 
*                               the CSD HW block is in use by another 
*                               application.
* * CY_CSDADC_BAD_PARAM       - A conversion is not started. An invalid mode 
*                               value or chMask is 0 or the context 
*                               pointer is NULL.
* * CY_CSDADC_NOT_INITIALIZED - CSDADC to be initialized by using the 
*                               Cy_CSDADC_Init() and Cy_CSDADC_Enable() 
*                               functions.
*
* \funcusage
*
* An example of the conversion executing in Single-shot mode:
* \snippet csdadc/snippet/main.c snippet_csdadc_chResult_declaration
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_RunConversionSingleShot
*
* An example of the conversion executing in Continuous mode:
* \snippet csdadc/snippet/main.c snippet_csdadc_chResult_declaration
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_RunConversionContinuous
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_StartConvert(
                cy_en_csdadc_conversion_mode_t mode,
                uint32_t chMask, 
                cy_stc_csdadc_context_t * context)
{
    cy_en_csdadc_status_t result = CY_CSDADC_SUCCESS;
    uint8_t chId = 0u;

    CY_ASSERT_L1(NULL != context);

    if ((NULL == context) || ((mode != CY_CSDADC_SINGLE_SHOT) && (mode != CY_CSDADC_CONTINUOUS)) || (chMask == 0u))
    {
        result = CY_CSDADC_BAD_PARAM;
    }
    else
    {
        /* Check whether the CSDADC is configured */
        if ((uint16_t)CY_CSDADC_INIT_DONE != (context->status & (uint16_t)CY_CSDADC_INIT_MASK))
        {
            result = CY_CSDADC_NOT_INITIALIZED;
        }
        else
        {
            if(CY_CSDADC_SUCCESS != Cy_CSDADC_IsEndConversion(context))
            {
                result = CY_CSDADC_HW_BUSY;
            }
            else
            {
                /* Save chMask to the context structure */
                context->chMask = chMask;
                /* Set the conversion mode bits of the CSDADC status byte */
                context->status &= (uint16_t)~(uint16_t)CY_CSDADC_CONV_MODE_MASK;
                context->status |= (uint16_t)(((uint32_t)mode) << CY_CSDADC_CONV_MODE_BIT_POS);

                /* Choose the first desired channel to convert */
                chId = Cy_CSDADC_GetNextCh(0u, context);
                /* Configure a desired channel if needed */
                if (chId != context->activeCh)
                {
                    if (CY_CSDADC_NO_CHANNEL != context->activeCh)
                    {
                        /* Disconnect existing input channel */
                        Cy_CSDADC_SetAdcChannel((uint32_t)context->activeCh, CY_CSDADC_CHAN_DISCONNECT, context);
                    }
                    /* Connect desired input channel */
                    Cy_CSDADC_SetAdcChannel((uint32_t)chId, CY_CSDADC_CHAN_CONNECT, context);
                    context->activeCh = chId;
                }

                /* Set the cycle counter to zero and the number of the first channel to convert to the conversion counter */
                context->counter = (uint32_t)(((uint32_t)chId) << CY_CSDADC_COUNTER_CHAN_POS);
                /* Clear stop bits */
                context->status &= (uint16_t)~CY_CSDADC_STOP_BITS_MASK;
                /* Set the busy bit of the CSDADC status byte */
                context->status |= CY_CSDADC_STATUS_BUSY_MASK;
                /* Set CSDADC FSM status */
                context->status &= (uint16_t)~(uint16_t)CY_CSDADC_STATUS_FSM_MASK;
                context->status |= (uint16_t)CY_CSDADC_STATUS_CONVERTING;
                /* Start conversion */
                Cy_CSDADC_StartFSM(CY_CSDADC_MEASMODE_VIN, context);
            }
        }
    }

    return (result);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_StopConvert
****************************************************************************//**
*
* The function stops  conversions in the continuous mode.
*
* This function can be used to stop CSDADC conversions in continuous mode. 
* The ADC can be stopped instantly by ignoring current and all future 
* conversions in the queue. Or it can be stopped after the current conversion 
* on an input channel is completed but ignore future conversions in the queue. 
* Or it can be stopped after the current conversion cycle (i.e. one set of 
* conversion on all enabled inputs) is completed and ignore conversion 
* cycles in the queue.
* 
* The ADC status should be checked using the Cy_CSDADC_IsEndConversion() 
* function. A new conversion or calibration should be started only 
* if CSDADC is not BUSY.
*
* \param stopMode
* The desired mode of the stop operation. It can be:
* * CY_CSDADC_IMMED_STOP        - The CSDADC conversion will be stopped 
*                                 immediately. A last channel conversion 
*                                 may produce an invalid result.
* * CY_CSDADC_CURRENT_CHAN_STOP - The CSDADC conversion will be stopped after
*                                 the current channel conversion to be completed.
* * CY_CSDADC_ENABLED_CHAN_STOP - The CSDADC conversion will be stopped after
*                                 all the enabled channels conversions to be 
*                                 completed.
*
* \param context
* The pointer to the CSDADC middleware context structure.
*
* \return
* The function returns the next statuses:
* * CY_CSDADC_SUCCESS   - Stop operation successful. The CSDADC conversion is 
                          stopped in the case of the immediate stop mode or 
                          the stop conversion bit in the CSDADC status byte is  
                          set to stop conversions after the current channel or 
                          all the enabled channels are completed.
* * CY_CSDADC_BAD_PARAM - An input parameter is bad.
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_StopConvert(
                cy_en_csdadc_stop_mode_t stopMode, 
                cy_stc_csdadc_context_t * context)
{
    cy_en_csdadc_status_t result = CY_CSDADC_SUCCESS;
    uint32_t watchdogAdcCounter;

    CY_ASSERT_L1(NULL != context);

    if ((NULL == context) || ((CY_CSDADC_IMMED_STOP != stopMode) &&
                            (CY_CSDADC_CURRENT_CHAN_STOP != stopMode) &&
                            (CY_CSDADC_ENABLED_CHAN_STOP != stopMode)))
    {
        result = CY_CSDADC_BAD_PARAM;
    }
    else
    {
        if (CY_CSDADC_IMMED_STOP == stopMode)
        {
            /* Mask all CSD HW block interrupts (disable all interrupts) */
            Cy_CSD_WriteReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_INTR_MASK, CY_CSDADC_CSD_INTR_MASK_CLEAR_MSK);

            /* Stop the CSD HW block sequencer */
            Cy_CSD_WriteReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_SEQ_START, CY_CSDADC_FSM_ABORT);

            /* Initialize Watchdog Counter with a time interval that is enough for the ADC operation to complete */
            watchdogAdcCounter = CY_CSDADC_CAL_WATCHDOG_CYCLES_NUM;
            while ((CY_CSD_SUCCESS != Cy_CSD_GetConversionStatus(context->cfgCopy.base, context->cfgCopy.csdCxtPtr))
                                                                                     &&  (0u != watchdogAdcCounter))
            {
                /* Wait until scan complete and decrement Watchdog Counter to prevent unending loop */
                watchdogAdcCounter--;
            }

            /* Clear all pending interrupts of the CSD HW block */
            Cy_CSD_WriteReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_INTR, CY_CSDADC_CSD_INTR_ALL_MSK);
            (void)Cy_CSD_ReadReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_INTR);

            if (0u == watchdogAdcCounter)
            {
                result = CY_CSDADC_TIMEOUT;
            }
            /* Clear all status bits except the initialization bit */
            context->status &= (uint16_t)CY_CSDADC_INIT_MASK;

        }
        else if (CY_CSDADC_CURRENT_CHAN_STOP == stopMode)
        {
            context->status &= (uint16_t)~(CY_CSDADC_STOP_BITS_MASK);
            context->status |= (uint16_t)(((uint32_t)CY_CSDADC_CURRENT_CHAN_STOP) << CY_CSDADC_STOP_BITS_POS);
        }
        else
        {
            context->status &= (uint16_t)~(CY_CSDADC_STOP_BITS_MASK);
            context->status |= (uint16_t)(((uint32_t)CY_CSDADC_ENABLED_CHAN_STOP) << CY_CSDADC_STOP_BITS_POS);
        }
    }
    return result;
}


/*******************************************************************************
* Function Name: Cy_CSDADC_IsEndConversion
****************************************************************************//**
*
* The function returns the status of the CSDADC's operation.
*
* This function is used to ensure the CSDADC is in the idle state prior 
* to initiating a conversion, calibration, or configuration change. 
* Initiating any ADC operation while the CSDADC is in the busy state may 
* produce unexpected results from the ADC.
*
* \param context
* The pointer to the CSDADC context.
*
* \return
* The function returns the status of the ADC operation.
* * CY_CSDADC_SUCCESS  - The ADC is not busy, so
*                        a new conversion can be initiated.
* * CY_CSDADC_HW_BUSY  - The previously initiated conversion is in progress.
* * CY_CSDADC_OVERFLOW - The most recent conversion caused an overflow. The root 
*                        cause of the overflow may be the previous calibration 
*                        values being invalid or VDDA and VDDA configuration 
*                        setting mismatch. Perform a re-calibration or set 
*                        the appropriate VDDA value to avoid this error 
*                        condition.
*
* \funcusage
*
* An example of the Cy_CSDADC_IsEndConversion() function usage:
* \snippet csdadc/snippet/main.c snippet_csdadc_chResult_declaration
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_RunConversionSingleShot
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_IsEndConversion(
                const cy_stc_csdadc_context_t * context)
{
    cy_en_csdadc_status_t result = CY_CSDADC_SUCCESS;

    CY_ASSERT_L1(NULL != context);

    if (NULL == context)
    {
        result = CY_CSDADC_BAD_PARAM;
    }
    else
    {
        if (0u != (Cy_CSD_ReadReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_ADC_RES) & CY_CSDADC_ADC_RES_OVERFLOW_MASK))
        {
            result = CY_CSDADC_OVERFLOW;
        }

        else if (0u != (CY_CSDADC_STATUS_BUSY_MASK & context->status))
        {
           result = CY_CSDADC_HW_BUSY;
        }
        else
        {
            /* Do nothing result = CY_CSDADC_SUCCESS */
        }
    }
    return result;
}


/*******************************************************************************
* Function Name: Cy_CSDADC_GetConversionStatus
****************************************************************************//**
*
* The function returns a current CSDADC conversion status.
*
* In Continuous mode, this function returns a combination of the current channel 
* number and current cycle number. This function can be used to identify whether 
* a cycle of conversion completed or identify the latest input where 
* the conversion completed so a result can be read.
* In Single-shot mode, only the latest input where a conversion completed is returned. 
* A conversion cycle number is incremented by the CSDADC after each cycle 
* of conversion completes. A channel number is assigned to each input channel.
* A new start-conversion request resets the conversion cycle number to zero and 
* the channel number to the first enabled channel in the chMask parameter.
*
* \param context
* The pointer to the CSDADC context.
*
* \return
* The function returns a combination of the conversion number and channel number.
* * Bit[0-26]  The current cycle number in Continuous mode. In the single 
*              shot mode, it is equal to 0u.
* * Bit[27-31] A current input channel number inside the current cycle.
* * If the context parameter is equal to NULL, then the function 
*   returns \ref CY_CSDADC_COUNTER_BAD_PARAM.
*
* \funcusage
*
* An example of the Cy_CSDADC_GetConversionStatus() function usage:
* \snippet csdadc/snippet/main.c snippet_csdadc_chResult_declaration
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_RunConversionContinuous
*
*******************************************************************************/
uint32_t Cy_CSDADC_GetConversionStatus(const cy_stc_csdadc_context_t * context)
{
    uint32_t result;

    CY_ASSERT_L1(NULL != context);

    if (NULL != context)
    {
        result = context->counter;
    }
    else
    {
        result = CY_CSDADC_COUNTER_BAD_PARAM;
    }
    return (result);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_Calibrate
****************************************************************************//**
*
* Performs calibration of the CSDADC.
*
* Executes calibration for the CSDADC to identify optimal CSD HW block 
* configuration to produce accurate results. The configuration parameters 
* are dependent on VDDA, VREF, IDAC, and PERI_CLK tolerances, and hence 
* run calibrations periodically (for example every 10 seconds) 
* to compensate for variation in the above mentioned parameters.
*
* \param context
* The pointer to the CSDADC context structure.
*
* \return
* The function returns the status of its operation.
* * CY_CSDADC_SUCCESS          - The CSD HW block is successfully calibrated for 
*                                the ADC use.
* * CY_CSDADC_BAD_PARAM        - The context pointer is NULL. A calibration 
*                                has not been executed.
* * CY_CSDADC_HW_BUSY          - The CSD HW block is already in use by a previously 
*                                initialized conversion or other function. 
*                                A calibration has not been executed.
* * CY_CSDADC_CALIBRATION_FAIL - The operation watchdog is triggered. 
*                                The calibration was not performed.
*
*******************************************************************************/
cy_en_csdadc_status_t Cy_CSDADC_Calibrate(cy_stc_csdadc_context_t * context)
{
    uint32_t watchdogAdcCounter;

    cy_en_csdadc_status_t result = CY_CSDADC_SUCCESS;

    CY_ASSERT_L1(NULL != context);

    if (NULL == context)
    {
        result = CY_CSDADC_BAD_PARAM;
    }
    else
    {
        if (CY_CSD_SUCCESS != Cy_CSD_GetConversionStatus(context->cfgCopy.base, context->cfgCopy.csdCxtPtr))
        {
            result = CY_CSDADC_HW_BUSY;
        }
        else
        {
            /* Disconnect channels if connected */
            if (CY_CSDADC_NO_CHANNEL != context->activeCh)
            {
                /* Disconnect existing input channel */
                Cy_CSDADC_ClearChannels(context);
                context->activeCh = CY_CSDADC_NO_CHANNEL;
            }

            Cy_CSD_WriteReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_IDACB, CY_CSDADC_IDACB_CONFIG | context->cfgCopy.idac);

            /* Set the busy bit of the CSDADC status byte */
            context->status |= (uint16_t)CY_CSDADC_STATUS_BUSY_MASK;
            /* Set CADADC FSM status */
            context->status &= (uint16_t)~(uint16_t)CY_CSDADC_STATUS_FSM_MASK;
            context->status |= (uint16_t)CY_CSDADC_STATUS_CALIBPH1;
            /* Un-mask ADC_RES interrupt (enable interrupt) */
            Cy_CSDADC_StartFSM(CY_CSDADC_MEASMODE_VREF, context);

            /* Initialize Watchdog Counter with a time interval that is enough for ADC calibration to complete */
            watchdogAdcCounter = CY_CSDADC_CAL_WATCHDOG_CYCLES_NUM;
            while (((context->status & CY_CSDADC_STATUS_FSM_MASK) != 0u) &&  (0u != watchdogAdcCounter))
            {
                /* Wait until scan complete and decrement Watchdog Counter to prevent unending loop */
                watchdogAdcCounter--;
            }
            if (0u == watchdogAdcCounter)
            {
                result = CY_CSDADC_CALIBRATION_FAIL;
            }
        }
    }
    return (result);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_GetNextCh
****************************************************************************//**
*
* Get the next channel enabled in the channel mask starting from the current.
*
* The function checks whether the current channel is enabled in the chMask.
* If the current channel is enabled in chMask, the function returns
* the current number. If not, the function seeks the next enabled channel.
* If the current channel and all the next are disabled in chMask, the function
* returns the channel's number.
*
* \param currChId
* The ID of the channel to start the enabled checking.
*
* \param context
* The pointer to the CSDADC middleware context structure.
*
* \return
* The function returns the next enabled channel number starting from
* the current one.
*
*******************************************************************************/
static uint8_t Cy_CSDADC_GetNextCh(
                uint8_t currChId, 
                const cy_stc_csdadc_context_t * context)
{
    uint32_t chPos = 1uL << currChId;

    /* Choose the first set channel to convert */
    while ((currChId < context->cfgCopy.numChannels) && (0u == (chPos & context->chMask)))
    {
        chPos <<= 1u;
        currChId++;
    }

    return (currChId);
}


/*******************************************************************************
* Function Name: Cy_CSDADC_InterruptHandler
****************************************************************************//**
*
* Implements the interrupt service routine for the CSDADC middleware.
* 
* The CSD HW block generates an interrupt at the end of every conversion or 
* a calculation phase. The CSDADC middleware uses this interrupt to implement 
* a non-blocking conversion method, in which only the first conversion is 
* initiated by the application program and subsequent conversions are 
* initiated in the interrupt service routine as soon as the current one 
* is completed. The above stated interrupt service routine is implemented 
* as a part of the CSDADC middleware. 
* 
* The CSDADC middleware does not initialize or modify the priority 
* of interrupts. For the middleware operation, the application program 
* must configure the CSD interrupt and assign the interrupt vector to 
* the Cy_CSDACD_InterruptHandler() function.
* If the CSD HW block is shared by more than one middleware, 
* the CSD interrupt vector is initialized to the interrupt handler 
* function of the middleware that is active in the application program.
*
* \param base
* The pointer to the base register address of the CSD HW block. A macro for 
* the pointer can be found in cycfg_peripherals.h file defined as 
* \<Csd_Personality_Name\>_HW. If no name is specified, the default name
* is used csd_\<Block_Number\>_csd_\<Block_Number\>_HW.
*
* \param CSDADC_Context
* The pointer to the CSDADC context structure.
*
* \funcusage
* An example of the ISR initialization:
*
* The CSDADC_ISR_cfg variable is declared by the application
* program according to the examples below:<br>
* For CM0+ core:
* \snippet csdadc/snippet/main.c snippet_m0p_adc_interrupt_source_declaration
*
* For CM4 core:
* \snippet csdadc/snippet/main.c snippet_m4_adc_interrupt_source_declaration
* 
* The CSDADC interrupt handler should be declared by the application program
* according to the example below:
* \snippet csdadc/snippet/main.c snippet_CSDADC_Interrupt
*
* Then, the application program configures and enables the CSD block interrupt
* between calls of the Cy_CapSense_Init() and Cy_CapSense_Enable() functions:
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_Initialization
*
* An example of sharing the CSD HW block by the CapSense and CSDADC middleware.<br>
* Declares the CapSense_ISR_cfg variable:
* \snippet csdadc/snippet/main.c snippet_m4_capsense_interrupt_source_declaration
* 
* Declares the CSDADC_ISR_cfg variable:
* \snippet csdadc/snippet/main.c snippet_m4_adc_interrupt_source_declaration
* 
* Defines the CapSense interrupt handler:
* \snippet csdadc/snippet/main.c snippet_CapSense_Interrupt
* 
* Defines the CSDADC interrupt handler:
* \snippet csdadc/snippet/main.c snippet_CSDADC_Interrupt
* 
* The part of the main.c FW flow:
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_TimeMultiplex  
*
*******************************************************************************/
void Cy_CSDADC_InterruptHandler(const CSD_Type * base, void * CSDADC_Context)
{
    cy_stc_csdadc_context_t * csdadcCxt = (cy_stc_csdadc_context_t *) CSDADC_Context;

    uint32_t interruptState;
    uint32_t newRegValue;

    uint32_t tmpResult;
    uint32_t polarity;

    uint16_t timeVssa2Vref = csdadcCxt->tVssa2Vref;
    uint16_t timeFull = csdadcCxt->tFull;
    uint16_t voltageMaxMv = csdadcCxt->vMaxMv;
    uint16_t timeVdda2Vref = csdadcCxt->tVdda2Vref;
    uint16_t timeRecover = csdadcCxt->tRecover;

    uint8_t adcFsmStatus;
    uint8_t tmpChId;

    (void)base;
    
    /* Mask all CSD HW block interrupts (disable all interrupts) */
    Cy_CSD_WriteReg(csdadcCxt->cfgCopy.base, CY_CSD_REG_OFFSET_INTR_MASK, CY_CSDADC_CSD_INTR_MASK_CLEAR_MSK);

    /* Clear all pending interrupts of CSD HW block */
    Cy_CSD_WriteReg(csdadcCxt->cfgCopy.base, CY_CSD_REG_OFFSET_INTR, CY_CSDADC_CSD_INTR_ALL_MSK);
    (void)Cy_CSD_ReadReg(csdadcCxt->cfgCopy.base, CY_CSD_REG_OFFSET_INTR);

    /* Read ADC result and check for ADC_ABORT or ADC_OVERFLOW flags */
    tmpResult = Cy_CSD_ReadReg(csdadcCxt->cfgCopy.base, CY_CSD_REG_OFFSET_ADC_RES);
    if (0u == (tmpResult & CY_CSDADC_ADC_RES_ABORT_MASK))
    {
        if (0u == (tmpResult & CY_CSDADC_ADC_RES_OVERFLOW_MASK))
        {
            /* Read ADC status, define polarity, value, and ChId */
            adcFsmStatus = (uint8_t)(csdadcCxt->status & CY_CSDADC_STATUS_FSM_MASK);
            /* Select the polarity bit */
            polarity = tmpResult & CY_CSDADC_ADC_RES_HSCMPPOL_MASK;
            /* Select the result value */
            tmpResult &= CY_CSDADC_ADC_RES_VALUE_MASK;
            tmpChId = csdadcCxt->activeCh;

            /* ADC could have been converting or calibrating; handle each differently. */
            switch (adcFsmStatus)
            {
            case CY_CSDADC_STATUS_CONVERTING:
                /*
                * After converting, will calculate an ADC result in mV depending on
                * sourcing or sinking mode. Checks for saturation in all modes.
                */
                /* HSCMP polarity is 0:sink, 1:source */
                if(0u != polarity) /* Sourcing */
                {
                    /* Saturate result at timeVssa2Vref */
                    tmpResult = (tmpResult > (uint32_t)timeVssa2Vref) ? (uint32_t)timeVssa2Vref : tmpResult;
                    /* Scale result to Resolution range with rounding*/
                    tmpResult = ((((uint32_t)timeVssa2Vref - tmpResult) * csdadcCxt->codeMax) +
                                                                     ((uint32_t)timeFull >> 1u)) / (uint32_t)timeFull;
                }
                else /* Sinking */
                {
                    if (CY_CSDADC_RANGE_VDDA == csdadcCxt->cfgCopy.range)
                    {
                        /* Scale result with sink/source mismatch with rounding */
                        tmpResult = (((uint32_t)((uint32_t)timeRecover << 1u) * tmpResult) +
                                                            ((uint32_t)timeVssa2Vref >> 1u)) / (uint32_t)timeVssa2Vref;
                        /* Saturate result at t_Vdda2Vref*/
                        tmpResult = (tmpResult > (uint32_t)timeVdda2Vref) ? (uint32_t)timeVdda2Vref : tmpResult;
                        /* Scale result to Resolution range with rounding */
                        tmpResult = ((((uint32_t)timeVssa2Vref + tmpResult) * csdadcCxt->codeMax)  +
                                     ((uint32_t)timeFull >> 1u)) / (uint32_t)timeFull;
                    }
                    else
                    {
                        /* In vref mode, we are not supposed to be sinking. Saturate */
                        tmpResult = ((uint32_t)timeVssa2Vref * csdadcCxt->codeMax) / (uint32_t)timeFull;
                    }
                }
                if (tmpChId < csdadcCxt->cfgCopy.numChannels)
                {
                    /* Store ADC result code */
                    csdadcCxt->adcResult[tmpChId].code = (uint16_t)(tmpResult);
                    /* Scale result to mV with rounding and store it */
                    tmpResult = (((uint32_t)voltageMaxMv * tmpResult) + ((uint32_t)csdadcCxt->codeMax >> 1u)) /
                                                                                        (uint32_t)csdadcCxt->codeMax;
                    csdadcCxt->adcResult[tmpChId].mVolts = (uint16_t)(tmpResult);

                    /* Check for current channel stop */
                    if ((uint16_t)CY_CSDADC_CURRENT_CHAN_STOP ==
                            ((csdadcCxt->status & CY_CSDADC_STOP_BITS_MASK) >> CY_CSDADC_STOP_BITS_POS))
                    {
                        /* Clear all status bits except the initialization bit */
                        csdadcCxt->status &= (uint16_t)CY_CSDADC_INIT_MASK;
                    }
                    else
                    {
                        /* Get next channel ID and check whether it is chosen in chMask */
                        tmpChId++;
                        tmpChId = Cy_CSDADC_GetNextCh(tmpChId, csdadcCxt);
                        /* Check whether it is the last channel */
                        if ((tmpChId >= csdadcCxt->cfgCopy.numChannels))
                        {
                            /* Check for single shot mode or enabled channel stop */
                            if (((uint16_t)CY_CSDADC_SINGLE_SHOT ==
                                    ((csdadcCxt->status & CY_CSDADC_CONV_MODE_MASK) >> CY_CSDADC_CONV_MODE_BIT_POS)) ||
                                ((uint16_t)CY_CSDADC_ENABLED_CHAN_STOP ==
                                        ((csdadcCxt->status & CY_CSDADC_STOP_BITS_MASK) >> CY_CSDADC_STOP_BITS_POS)))
                            {
                                /* Clear all status bits except the initialization bit */
                                csdadcCxt->status &= CY_CSDADC_INIT_MASK;
                            }
                            else
                            {
                                /* Call EOC callback if defined */
                                if(NULL != csdadcCxt->ptrEOCCallback)
                                {
                                    csdadcCxt->ptrEOCCallback((cy_stc_csdadc_context_t *)csdadcCxt);
                                }
                                /* For continuous mode, start from the first channel again */
                                tmpChId = Cy_CSDADC_GetNextCh(0u, csdadcCxt);
                                /* Disconnect the current input channel */
                                Cy_CSDADC_SetAdcChannel((uint32_t)csdadcCxt->activeCh, CY_CSDADC_CHAN_DISCONNECT, csdadcCxt);
                                /* Connect desired input channel */
                                Cy_CSDADC_SetAdcChannel((uint32_t)tmpChId, CY_CSDADC_CHAN_CONNECT, csdadcCxt);
                                csdadcCxt->activeCh = tmpChId;
                                /* Update the conversion counter */
                                csdadcCxt->counter &= (uint32_t)~CY_CSDADC_COUNTER_CHAN_MASK;
                                csdadcCxt->counter |= (uint32_t)(((uint32_t)tmpChId) << CY_CSDADC_COUNTER_CHAN_POS);
                                csdadcCxt->counter++;
                                /* Don't allow overflow of the cycle counter */
                                if ((csdadcCxt->counter & CY_CSDADC_COUNTER_CYCLE_MASK) == CY_CSDADC_COUNTER_CYCLE_MASK)
                                {
                                    csdadcCxt->counter &= (uint32_t)~CY_CSDADC_COUNTER_CYCLE_MASK;
                                }
                                /* Start conversion */
                                Cy_CSDADC_StartFSM(CY_CSDADC_MEASMODE_VIN, csdadcCxt);
                            }
                        }
                        else
                        {
                            /* Disconnect the current input channel */
                            Cy_CSDADC_SetAdcChannel((uint32_t)csdadcCxt->activeCh, CY_CSDADC_CHAN_DISCONNECT, csdadcCxt);
                            /* Connect the next input channel set in chMask*/
                            Cy_CSDADC_SetAdcChannel((uint32_t)tmpChId, CY_CSDADC_CHAN_CONNECT, csdadcCxt);
                            csdadcCxt->activeCh = tmpChId;
                            /* Update the conversion counter */
                            csdadcCxt->counter &= (uint32_t)~CY_CSDADC_COUNTER_CHAN_MASK;
                            csdadcCxt->counter |= (uint32_t)(((uint32_t)tmpChId) << CY_CSDADC_COUNTER_CHAN_POS);
                            /* Start conversion */
                            Cy_CSDADC_StartFSM(CY_CSDADC_MEASMODE_VIN, csdadcCxt);
                        }
                    }
                }
                else
                {
                    tmpResult = (((uint32_t)voltageMaxMv * tmpResult) + ((uint32_t)csdadcCxt->codeMax >> 1u)) /
                                                                                        (uint32_t)csdadcCxt->codeMax;
                    if (CY_CSDADC_VDDA_CHANNEL_MASK == (tmpChId & CY_CSDADC_VDDA_CHANNEL_MASK))
                    {
                        csdadcCxt->vddaMv = (uint16_t)(tmpResult);
                    }
                    else
                    {
                        csdadcCxt->vBusBMv = (uint16_t)(tmpResult);
                    }
                    /* Clear all status bits except the initialization bit */
                    csdadcCxt->status &= (uint16_t)CY_CSDADC_INIT_MASK;
                }
                break;

            case CY_CSDADC_STATUS_CALIBPH1:
                /*
                * After the calibration, phase 1 will define a time to charge Cref1 and Cref2 from Vssa to Vref in
                * clock cycles. In the full-range mode, the next calibration starts in phase 2. In the Vref mode, this checks for a target
                * and recalibrates IDAC if necessary.
                */
                csdadcCxt->tVssa2Vref = (uint16_t)tmpResult;

                if (CY_CSDADC_RANGE_VDDA == csdadcCxt->cfgCopy.range)
                {
                    /* Full range mode */
                    csdadcCxt->status &= (uint16_t)~(uint16_t)CY_CSDADC_STATUS_FSM_MASK;
                    csdadcCxt->status |= (uint16_t)CY_CSDADC_STATUS_CALIBPH2;
                    Cy_CSDADC_StartFSM(CY_CSDADC_MEASMODE_VREFBY2, csdadcCxt);
                }
                else
                {
                    /* Vref range mode */
                    csdadcCxt->vMaxMv = csdadcCxt->vRefMv;
                    csdadcCxt->tFull = csdadcCxt->tVssa2Vref;
                    /* In vref mode not need further calibration */
                    /* Set idle status */
                    csdadcCxt->status &= (uint16_t)~(uint16_t)CY_CSDADC_STATUS_BUSY_MASK;
                    csdadcCxt->status &= (uint16_t)~(uint16_t)CY_CSDADC_STATUS_FSM_MASK;
                }

                break;

            case CY_CSDADC_STATUS_CALIBPH2:
                /*
                * After the calibration, phase 2 defines a time to charge Cref1 and Cref2 to Vref by a sourcing them after
                * a discharge by a sinking from Vref during tVssa2Vref/2 time. This recharge time is called tRecover
                * and is proportional to an Idac_sourcing/Idac_sinking mismatch. Then this charges capacitors to Vdda and starts
                * the next calibration phase 3.
                */
                csdadcCxt->tRecover = (uint16_t)tmpResult;

                /* Disconnect amuxbusB, Connect VDDA to csdbusB */
                interruptState = Cy_SysLib_EnterCriticalSection();
                newRegValue = Cy_CSD_ReadReg(csdadcCxt->cfgCopy.base, CY_CSD_REG_OFFSET_SW_BYP_SEL);
                newRegValue &= (uint32_t)(~CY_CSDADC_SW_BYP_DEFAULT);
                Cy_CSD_WriteReg(csdadcCxt->cfgCopy.base, CY_CSD_REG_OFFSET_SW_BYP_SEL, newRegValue);
                Cy_SysLib_ExitCriticalSection(interruptState);
                Cy_CSD_WriteReg(csdadcCxt->cfgCopy.base, CY_CSD_REG_OFFSET_SW_SHIELD_SEL, CY_CSDADC_SW_SHIELD_VDDA2CSDBUSB);

                csdadcCxt->status &= (uint16_t)~(uint16_t)CY_CSDADC_STATUS_FSM_MASK;
                csdadcCxt->status |= (uint16_t)CY_CSDADC_STATUS_CALIBPH3;

                Cy_CSDADC_StartFSM(CY_CSDADC_MEASMODE_VIN, csdadcCxt);
                break;

            case CY_CSDADC_STATUS_CALIBPH3:
                /*
                * After the calibration, phase 3 will define a time to discharge Cref1 and Cref2 from Vdda to Vref by
                * a sinking. This time must be corrected to an Idac_sourcing/Idac_sinking mismatch defined in the phase 2.
                * Calculates t_full, checks it for a target and recalibrates IDAC if necessary. Then this calculates Vdda.
                */
                /* Reconnect amuxbusB, disconnect VDDA */
                Cy_CSD_WriteReg(csdadcCxt->cfgCopy.base, CY_CSD_REG_OFFSET_SW_SHIELD_SEL, CY_CSDADC_SW_SHIELD_DEFAULT);
                interruptState = Cy_SysLib_EnterCriticalSection();
                newRegValue = Cy_CSD_ReadReg(csdadcCxt->cfgCopy.base, CY_CSD_REG_OFFSET_SW_BYP_SEL);
                newRegValue |= CY_CSDADC_SW_BYP_DEFAULT;
                Cy_CSD_WriteReg(csdadcCxt->cfgCopy.base, CY_CSD_REG_OFFSET_SW_BYP_SEL, newRegValue);
                Cy_SysLib_ExitCriticalSection(interruptState);

                timeVdda2Vref = (uint16_t)tmpResult;
                /* Calibrate timeVdda2Vref with Sink/Source mismatch with rounding */
                timeVdda2Vref = (uint16_t)(((((uint32_t)timeVdda2Vref << 1u) * timeRecover) + ((uint32_t)timeVssa2Vref >> 1u)) / timeVssa2Vref);
                /* Store tVdda2Vref in the CSDADC context structure */
                csdadcCxt->tVdda2Vref = timeVdda2Vref;
                /* Store tFull in the CSDADC context structure */
                csdadcCxt->tFull = (csdadcCxt->tVssa2Vref + timeVdda2Vref);
                /* Store vMaxMv in the CSDADC context structure */
                csdadcCxt->vMaxMv = csdadcCxt->vRefMv + (((csdadcCxt->vRefMv * timeVdda2Vref) + (timeVssa2Vref >> 1u)) / timeVssa2Vref);
                /* Set the idle status */
                csdadcCxt->status &= (uint16_t)~(uint16_t)CY_CSDADC_STATUS_BUSY_MASK;
                csdadcCxt->status &= (uint16_t)~(uint16_t)CY_CSDADC_STATUS_FSM_MASK;

                break;

            default:
                /* If interrupt is called without a defined ADC state, do nothing. */
                break;
            }
        }
        else
        {
            csdadcCxt->status |= CY_CSDADC_OVERFLOW_MASK;
        }
    }
    else
    {
        csdadcCxt->status &= (uint16_t)~(uint16_t)CY_CSDADC_STATUS_BUSY_MASK;
    }
}

#endif /* CY_IP_MXCSDV2 */


/* [] END OF FILE */
