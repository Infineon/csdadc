/***************************************************************************//**
* \file cy_csdadc.h
* \version 1.0.1
*
* \brief
* This file provides function prototypes and constants specific to the CSDADC
* middleware.
*
********************************************************************************
* \copyright
* Copyright 2018-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
/**
* \mainpage Cypress CSDADC Middleware Library
*
* The CSDADC provides an API that enables ADC functionality of the
* CSD HW block. It can be useful for a devices that do not include 
* another ADC option. 
*
********************************************************************************
* \section section_capsense_general General Description
********************************************************************************
*
* The CSD HW block enables multiple sensing capabilities on PSoC devices 
* including self-cap and mutual-cap capacitive touch sensing solutions, 
* a 10-bit ADC, IDAC, and Comparator. The CSD driver is a low-level 
* peripheral driver, a wrapper to manage access to the CSD HW block. 
* Each middleware access to the CSD HW block is through the CSD Driver. 
* 
* The CSD HW block can support only one function at a time. However, all  
* supported functionality (like CapSense, CSDADC, etc.) can be 
* time-multiplexed in a design. I.e. you can save the existing state 
* of the CapSense middleware, restore the state of the CSDADC middleware,  
* perform ADC measurements, and then switch back to the CapSense functionality.
* For more details and code examples, refer to the description of the 
* Cy_CSDADC_Save() and Cy_CSDADC_Restore() functions.
* 
* \image html capsense_solution.png "CapSense Solution" width=800px
* \image latex capsense_solution.png
* 
* This section describes only CSDADC middleware. Refer to the corresponding 
* sections for documentation of other middleware supported by the CSD HW block.
* The CSDADC library is designed to be used with the CSD driver.
* The application program does not need to interact with the CSD driver 
* and/or other drivers such as GPIO or SysClk directly. All of that is 
* configured and managed by middleware.
* 
* Include cy_csdadc.h to get access to all functions and other declarations 
* in this library.
*
* The CSDADC API is described in the following sections:
* * \ref group_csdadc_macros
* * \ref group_csdadc_data_structures
* * \ref group_csdadc_enums
* * \ref group_csdadc_functions
*
* <b>Features:</b>
* * ADC with 8- and 10-bit resolution
* * Two input measurement ranges: GND to VREF and GND to VDDA
* * Two operation modes: Continuous conversion and single shot conversion
*
* \warning
* I2C transactions during ADC conversions could lead to measurement result 
* distortions. Perform ADC conversions and I2C communications in 
* time-sharing mode.
*
* \section group_csdadc_configuration Configuration Considerations
*
* The CSDADC operates on the top of the CSD driver. The CSD driver has
* some prerequisites for proper operation. Refer to the "CSD (CapSense
* Sigma Delta)" section of the PDL API Reference Manual.
* In the ModusToolbox IDE, the Device Configurator CSD personality should 
* be used for CSDADC MW initial configuration.
* If the user checks the "Enable CSDADC" checkbox, all required 
* configuration parameters appear. The user can set a number
* of input channels, a resolution, a measurement range, and all 
* other parameters needed for CSDADC middleware configuration. 
* All the parameters in CSD personality have pop-ups with detailed descriptions.
* If the user does not use ModusToolbox IDE, the user could create 
* a CSDADC configuration structure manually by using this API Ref Guide 
* and the configuration structure prototype define in the cy_csdadc.h file. 
*
* <b>Building CSDADC </b>
*
* The CSDADC middleware should be enabled for CM4 core of the PSoC 6 
* by Middleware Selector in the ModusToolbox.
* The CSDADC middleware used for CM0+ core is not supported in the 
* ModusToolbox, but the user can create CM0+ configuration 
* with CSDADC middleware manually.
*
* \note
* If building the project with CSDADC outside the ModusToolbox environment, 
* the path to the CSDADC middleware should be manually specified 
* in the project settings or in the Makefile.
*
* <b>Initializing CSDADC </b>
*
* To initialize a CSDADC, the CSDADC context structure should
* be declared by the user. An example of the CSDADC context structure
* declaration is below:
*
*         cy_stc_csdadc_context_t cy_csdadc_context;
*
* Note that the name "cy_csdadc_context" is shown only for a reference.
* Any other name can be used instead. 
*
* The CSDADC configuration structure is generated by the Device
* Configurator CSD personality and should then be passed to the Cy_CSDADC_Init()
* as well as the context structure.
*
* Apart of the context structure allocation, the user should create and
* register an interrupt function for the CSDADC proper operation.
* This interrupt function will be called for every interrupt generated
* by the CSD HW block. For instance, it should be as described below:
*
*         static void CSDADC_Interrupt(void)
*         {
*             Cy_CSDADC_InterruptHandler(&cy_csdadc_context);
*         }
*
* This interrupt function and an allocated interrupt configuration structure
* should be declared as below:
*
*         static void CSDADC_Interrupt(void);
*         const cy_stc_sysint_t CSDADC_ISR_cfg =
*         {
*             .intrSrc = csd_interrupt_IRQn,
*             .intrPriority = 7u,
*         };
*
* In the main function, the interrupt function should be registered as below:
*
*         Cy_SysInt_Init(&CSDADC_ISR_cfg, &CSDADC_Interrupt);
*         NVIC_ClearPendingIRQ(CSDADC_ISR_cfg.intrSrc);
*         NVIC_EnableIRQ(CSDADC_ISR_cfg.intrSrc);
*
*
* \section group_csdadc_more_information More Information
* For more information, refer to the following documents:
*
* * <a href="http://www.cypress.com/trm218176"><b>Technical Reference Manual
* (TRM)</b></a>
*
* * <a href="http://www.cypress.com/ds218787"><b>PSoC 63 with BLE Datasheet
* Programmable System-on-Chip datasheet</b></a>
*
* * <a href="http://www.cypress.com/an210781"><b>AN210781 Getting Started
* with PSoC 6 MCU with Bluetooth Low Energy (BLE) Connectivity</b></a>
*
* * <a href="../../capsense_api_reference_manual.html"><b>CapSense MW API 
* Reference</b></a>
*
* * <a href="../../csdidac_api_reference_manual.html"><b>CSDIDAC MW API 
* Reference</b></a>
*
* * <a href="../../pdl_api_reference_manual/html/group__group__csd.html"><b>CSD 
* Driver API Reference</b></a>
*
* \section group_csdadc_MISRA MISRA-C Compliance
*
* The Cy_CSDADC library has the following specific deviations:
*
* <table class="doxtable">
*   <tr>
*     <th>MISRA Rule</th>
*     <th>Rule Class (Required/Advisory)</th>
*     <th>Rule Description</th>
*     <th>Description of Deviation(s)</th>
*   </tr>
*   <tr>
*     <td>11.4</td>
*     <td>A</td>
*     <td>A conversion should not be performed between a pointer to object 
*         and an integer type.</td>
*     <td>Such a conversion is performed with CSDADC context in two cases: the
*         interrupt handler and DeepSleepCallback function.
*         Both cases are verified on correct operation.</td>
*   </tr>
*   <tr>
*     <td>1.2</td>
*     <td rowspan=2> R</td>
*     <td rowspan=2> Constant: Dereference of NULL pointer.</td>
*     <td rowspan=2> These violations are reported as a result of using 
*         offset macros of the CSD Driver with corresponding documented 
*         violation 20.6. Refer to the CSD Driver API Ref Guide.</td>
*   </tr>
*   <tr>
*     <td>20.3</td>
*   </tr>
* </table>
*
* \section group_csdadc_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td rowspan="2">1.0.1</td>
*     <td>Documentation updates</td>
*     <td>Improve user's experience</td>
*   </tr>
*   <tr>
*     <td>Added limitation to the module.mk file</td>
*     <td>Support only devices with the CSD HW block</td>
*   </tr>
*   <tr>
*     <td>1.0</td>
*     <td>The initial version.</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_csdadc_macros Macros
* \brief
* This section describes the CSDADC Macros. These Macros can be used for 
* checking maximum channel number, for defining CSDADC conversion mode and 
* for checking CSDADC conversion counters. A detailed information about
* macros see in every macro description. 
*
* \defgroup group_csdadc_enums Enumerated types
* \brief
* Describes the enumeration types defined by the CSDADC. These enumerations 
* can be used for checking CSDADC functions' return statuses,
* for defining a CSDADC resolution and an input voltage range and for 
* defining start and stop conversion modes.  A detailed information about
* enumerations seen in every enumeration description. 
*
* \defgroup group_csdadc_data_structures Data Structures
* \brief
* Describes the data structures defined by the CSDADC. The CSDADC MW 
* use structures for input channel pins, conversion results, 
* MW configuration and context. The pin structure is included into 
* the configuration structure and both of them can be defined by the 
* user with the CSD personality in Device Configurator or manually if  
* the user don't use ModusToolbox.
* The result structure is included into the context structure and 
* contains voltages and ADC codes for all 32 input channel of the more 
* recent conversions. Besides the result structure, the context structure 
* contains a copy of the configuration structure, current CSDADC MW state 
* data and calibration data. The context structure should be
* allocated by the user and passed to all CSDADC MW functions. 
* CSDADC MW structure sizes are shown in the table below:
* 
* <table class="doxtable">
*   <tr><th>Structure</th><th>Size in bytes (w/o padding)</th></tr>
*   <tr>
*     <td>cy_stc_csdadc_ch_pin_t</td>
*     <td>9</td>
*   </tr>
*   <tr>
*     <td>cy_stc_csdadc_config_t</td>
*     <td>157</td>
*   </tr>
*   <tr>
*     <td>cy_stc_csdadc_result_t</td>
*     <td>4</td>
*   </tr>
*   <tr>
*     <td>cy_stc_csdadc_context_t</td>
*     <td>322</td>
*   </tr>
* </table>
*
* \defgroup group_csdadc_functions Functions
* \brief
* This section describes the CSDADC Function Prototypes.
*
* \defgroup group_csdadc_callback Callback
* \brief
* This section describes the CSDADC Callback Function.
* 
*/

/******************************************************************************/
/** \addtogroup group_csdadc_callback
* \{
*
* A callback allows a user to execute custom code called from the CSDADC
* middleware when the current cycle of enabled channel conversion finishes
* in the continuous mode.
*
* To assign a user's function to this callback presented in the middleware,
* do the following:
*
* * Assign a pointer to the variable of the \ref cy_stc_csdadc_context_t 
*   type as below:
*
*   <tt>cy_stc_csdadc_context_t * ptrCxt = \&My_CSDADC_context;</tt>
*
* * Write the callback function implementation (in any user file) using 
*   the following function prototype:
*
*   <tt>void CallbackFunction((void *) ptrCxt);</tt>
*
* * Assign the function to the CSDADC context structure by using the function
*   Cy_CSDADC_RegisterCallback;
*
* * Use the Cy_CSDADC_UnRegisterCallback() API to un-register the callback
*   function that was previously assigned.
*
* \} 
*/

#if !defined(CY_CSDADC_H)
#define CY_CSDADC_H

#include "cy_device_headers.h"
#include "cy_csd.h"


/* The C binding of definitions if building with the C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CY_IP_MXCSDV2
    #error "The CapSense middleware is not supported on this device"
#endif

/**
* \addtogroup group_csdadc_macros
* \{
*/
/** Middleware major version */
#define CY_CSDADC_MDW_VERSION_MAJOR             (1)

/** Middleware minor version */
#define CY_CSDADC_MDW_VERSION_MINOR             (0)

/** CSDADC PDL ID */
#define CY_CSDADC_ID                            (CY_PDL_DRV_ID(0x43u))

/** The CSDADC max channels number */
#define CY_CSDADC_MAX_CHAN_NUM                  (32u)
/** The CSDADC max code value for 8 bit resolution */
#define CY_CSDADC_RES_8_MAX_VAL                     ((uint32_t)(1uL << 8u) - 1u)
/** The CSDADC max code value for 10 bit resolution */
#define CY_CSDADC_RES_10_MAX_VAL                    ((uint32_t)(1uL << 10u) - 1u)
/** The parameter for the single shot mode of the CSDADC start conversion function */
#define CY_CSDADC_SINGLE_SHOT_MODE              (0u)
/** The parameter for the continuous mode of the CSDADC start conversion function */
#define CY_CSDADC_CONTINUOUS_MODE               (1u)

/* Conversion counter defines */
/** The channel counter mask for the CSDADC operation counter */
#define CY_CSDADC_COUNTER_CHAN_MASK             (0xF8000000uL)
/** The cycle counter mask for the CSDADC operation counter */
#define CY_CSDADC_COUNTER_CYCLE_MASK            (0x07FFFFFFuL)
/** The channel counter position in the CSDADC operation counter */
#define CY_CSDADC_COUNTER_CHAN_POS              (27u)
/** The conversion status function return for the NULL context pointer */
#define CY_CSDADC_COUNTER_BAD_PARAM             (0xFFFFFFFFuL)
/** The measurement function return for a fail case */
#define CY_CSDADC_MEASUREMENT_FAILED            (0xFFFFFFFFuL)

/** \} group_csdadc_macros */

/***************************************
* Enumerated Types and Parameters
***************************************/
/**
* \addtogroup group_csdadc_enums
* \{
*/

/** 
* CSDADC return enumeration type. \ref CY_CSDADC_ID is a unique FW registration number.
* CY_PDL_STATUS_ERROR see
* <a href="../../pdl_api_reference_manual/html/group__group__syslib__macros__status__codes.html" 
* title="SysLib Status Codes Macros"
* >SysLib Status Codes Macros</a>. */
typedef enum
{
    CY_CSDADC_SUCCESS           = (0u),   
                                            /**< The function executed successfully */
    CY_CSDADC_BAD_PARAM         = (CY_CSDADC_ID + (uint32_t)CY_PDL_STATUS_ERROR + 1u),
                                            /**< 
                                             * Any of the input parameter is invalid. The user should check if all
                                             * the input parameters are valid.
                                             */
    CY_CSDADC_HW_LOCKED         = (CY_CSDADC_ID + (uint32_t)CY_PDL_STATUS_ERROR + 2u),
                                            /**< 
                                             * The CSD HW block is acquired and locked by another middleware 
                                             * or an application. CSDADC must wait for the CSD HW block to be released
                                             * to acquire the CSD HW block for use. 
                                             */
    CY_CSDADC_HW_BUSY           = (CY_CSDADC_ID + (uint32_t)CY_PDL_STATUS_ERROR + 3u),
                                            /**< 
                                             * The previous CSDADC operation not completed. The user should call
                                             * Cy_CSDADC_IsEndConversion() function and wait until current
                                             * operation complete.
                                             */
    CY_CSDADC_OVERFLOW          = (CY_CSDADC_ID + (uint32_t)CY_PDL_STATUS_ERROR + 4u),
                                            /**< 
                                             * CSDADC counter overflow. This error could occur if CSDADC 
                                             * is not calibrated. The user should check VREF, PERI_CLK, interrupts and 
                                             * perform a new CSDADC calibration.
                                             */
    CY_CSDADC_CALIBRATION_FAIL  = (CY_CSDADC_ID + (uint32_t)CY_PDL_STATUS_ERROR + 5u),
                                            /**< 
                                             * CSDADC calibration failed. CSDADC can't be calibrated well. The user
                                             * should check VREF, PERI_CLK, interrupts.
                                             */
    CY_CSDADC_WRITE_CONFIG_FAIL = (CY_CSDADC_ID + (uint32_t)CY_PDL_STATUS_ERROR + 6u),
                                            /**< CSDADC configuration writing failed */
    CY_CSDADC_NOT_INITIALIZED   = (CY_CSDADC_ID + (uint32_t)CY_PDL_STATUS_ERROR + 7u),
                                            /**< 
                                             * CSDADC middleware is not initialized. This error could occur if
                                             * the user tries to start conversion without a proper CSDADC initialization.
                                             */
    CY_CSDADC_TIMEOUT           = (CY_CSDADC_ID + (uint32_t)CY_PDL_STATUS_ERROR + 8u),
                                            /**< 
                                             * CSDADC operation timeout. This error could occur if
                                             * a CSDADC conversion can not be stopped by Cy_CSDADC_StopConvert()
                                             * function in the immediate mode. The user should check interrupts.
                                             */
} cy_en_csdadc_status_t;

/** CSDADC input voltage ranges enumeration type */
typedef enum
{ 
    CY_CSDADC_RANGE_VREF          = 0u,      /**< 
                                              * The GND to VREF input voltage range.
                                              * The user can choose this range because of its better linearity. 
                                              * The VREF can be set by middleware automatically depending on VDDA or
                                              * can be set by the user manually.
                                              */
    CY_CSDADC_RANGE_VDDA          = 1u,      /**< 
                                              * The GND to VDDA input voltage range.
                                              * This range has a worse linearity, but has a wider input voltage band. 
                                              */
}cy_en_csdadc_range_t;

/** CSDADC resolution enumeration type */
typedef enum
{
    CY_CSDADC_RESOLUTION_8BIT     = 0u,      /**< 
                                              * The 8 bit resolution. This mode has a worse accuracy but provides
                                              * a minimum conversion time.
                                              */
    CY_CSDADC_RESOLUTION_10BIT    = 1u,      /**< 
                                              * The 10 bit resolution. This mode has a better accuracy than an 8bit,
                                              * but has approximately a 4 times bigger conversion time.
                                              */
}cy_en_csdadc_resolution_t;

/** 
* CSDADC conversion mode enumeration type. This enum type should be used to 
* specify conversion mode of CSDADC when initiating a new conversion 
* using the Cy_CSDADC_StartConvert() function. With a single shot mode, 
* the CSDADC will convert every input channel voltages that are specified 
* by channel mask parameter of this function. With a conditions
* mode, conversion will repeated for all specified channels until the user 
* stop them by using the Cy_CSDADC_StopConvert() function. To read 
* conversion data in the continuous mode without getting over-written, 
* the user should call the Cy_CSDADC_ConversionStatus() function. */
typedef enum
{ 
    CY_CSDADC_SINGLE_SHOT         = 0u,      /**< The single shot mode */
    CY_CSDADC_CONTINUOUS          = 1u,      /**< The continuous mode */
}cy_en_csdadc_conversion_mode_t;

/** 
* CSDADC stop mode enumeration type. This enum type should be used to 
* specify stop conversion mode of CSDADC when breaking conversion(s) 
* using the Cy_CSDADC_StopConvert() function. It can be used only in 
* continuous conversion mode. 
*/
typedef enum
{
    CY_CSDADC_IMMED_STOP          = 0u,      /**< 
                                              * The immediate stop mode. In this mode CSDADC conversion is disrupted
                                              * immediately and an interrupt will be cleared. To prevent a possible
                                              * CSD HW block hanging, a program watchdog starts. If this watchdog will
                                              * be triggered, the Cy_CSDADC_StopConvert() function returns
                                              * CY_CSDADC_TIMEOUT.
                                              */ 
    CY_CSDADC_CURRENT_CHAN_STOP   = 1u,      /**< 
                                              * After the current channel conversion ending stop mode. In this mode 
                                              * CSDADC conversion will be stopped after a current channel conversion. 
                                              * The user should check the status by using the 
                                              * Cy_CSDADC_IsEndConversion()function.
                                              */
    CY_CSDADC_ENABLED_CHAN_STOP   = 2u,      /**< 
                                              * After the enabled channels conversion cycle ending stop mode. In this mode 
                                              * CSDADC conversion will be stopped after all enabled channel conversion. 
                                              * The user should check the status by using the 
                                              * Cy_CSDADC_IsEndConversion()function.
                                              */
}cy_en_csdadc_stop_mode_t;

/** \} group_csdadc_enums */


/***************************************
* Data Structure definitions
***************************************/
/**
* \addtogroup group_csdadc_data_structures
* \{
*/

/** CSDADC pin structure */
typedef struct {
    GPIO_PRT_Type * ioPcPtr;                /**< Pointer to channel IO PC register */
    uint8_t pin;                            /**< Channel IO pin */
} cy_stc_csdadc_ch_pin_t;

/** CSDADC configuration structure */
typedef struct
{
    const cy_stc_csdadc_ch_pin_t * ptrPin[CY_CSDADC_MAX_CHAN_NUM];
                                            /**< Array of pointers to the channel IO structures */
    CSD_Type * base;                        /**< Pointer to the CSD HW Block */
    cy_stc_csd_context_t * csdCxtPtr;       /**< Pointer to the CSD context driver */
    uint32_t periClk;                       /**< Peri Clock in Hz */
    int16_t vref;                           /**< Voltage Reference in mV */
    uint16_t vdda;                          /**< Analog Power Voltage in mV */
    uint16_t calibrInterval;                /**< Interval for auto-calibration. In this version not supported */
    cy_en_csdadc_range_t range;             /**< Mode of ADC operation */
    cy_en_csdadc_resolution_t resolution;   /**< Resolution */
    cy_en_divider_types_t periDivTyp;       /**< Peri Clock divider type */
    uint8_t numChannels;                    /**< Number of ADC channels */
    uint8_t idac;                           /**< IDAC code */
    uint8_t operClkDivider;                 /**< Divider of Peri Clock frequency for the CSDADC operation clock */
    uint8_t azTime;                         /**< CSDADC auto-zero time in us */
    uint8_t acqTime;                        /**< CSDADC acquisition time in us */
    uint8_t csdInitTime;                    /**< CSD HW Block Initialization time in us */
    uint8_t idacCalibrationEn;              /**< Enables run-time IDAC calibration. In this version not supported */
    uint8_t periDivInd;                     /**< Peri Clock divider index */
} cy_stc_csdadc_config_t;

/** CSDADC result structure */
typedef struct {
    uint16_t code;                          /**< Channel conversion result as ADC code */
    uint16_t mVolts;                        /**< Channel conversion result as input voltage in mV */
} cy_stc_csdadc_result_t;

/**
* Provides the typedef for the callback function that is intended to be called
* when the "End Of Conversion" cycle callback event occurs.
*/
typedef void (*cy_csdadc_callback_t)(void * ptrCxt);

/** 
* CSDADC context structure that contains the internal driver data for 
* the CSDADC MW. The context structure should be
* allocated by the user and passed to all CSDADC MW functions. 
*/
typedef struct{
    cy_stc_csdadc_config_t cfgCopy;         /**< Configuration structure copy */
    cy_stc_csdadc_result_t adcResult[CY_CSDADC_MAX_CHAN_NUM];
                                            /**< CSDADC result array */
    cy_csdadc_callback_t ptrEOCCallback;
                                            /**< Pointer to a user's End Of Conversion callback function. Refer to \ref group_csdadc_callback section */
    uint32_t chMask;                        /**< Active mask of channels to convert */
    uint32_t counter;                       /**< Counter for CSDADC operations:
                                             * * bit [0:26] - current enabled channels cycle number:
                                             *     * In the continuous mode, sets to 0 u with the conversion start and increments with every enabled channel cycle
                                             *     * In the single shot mode, is equal to 0 u
                                             * * bits [27:31] - current channel number inside the current cycle */
    volatile uint16_t status;               /**< Current CSDADC status:
                                             * * bit [0] - if set to 1, then CSDADC initialization is done
                                             * * bit [1] - 0 single shot mode, 1 continuous mode
                                             * * bit [2] - 0 CSDADC idle state, 1 CSDADC busy state
                                             * * bit [3] - 0 ADC_RESULT did not overflow, 1 ADC_RESULT overflow
                                             * * bit [4:7] - FSM status:
                                             *     * 0 - CY_CSDADC_STATUS_FSM_IDLE
                                             *     * 1 - CY_CSDADC_STATUS_CALIBPH1
                                             *     * 2 - CY_CSDADC_STATUS_CALIBPH2
                                             *     * 3 - CY_CSDADC_STATUS_CALIBPH3
                                             *     * 4 - CY_CSDADC_STATUS_CONVERTING
                                             * * bit [9] - stop conversion mode
                                             *     * 0 - stop after current channel conversion
                                             *     * 1 - stop after all enabled channels in chMask */
    uint16_t codeMax;                       /**< Max CSDADC code value */
    uint16_t vMaxMv;                        /**< Max CSDADC input voltage in mV */
    uint16_t tFull;                         /**< Calibration data */
    uint16_t tVssa2Vref;                    /**< Calibration data */
    uint16_t tVdda2Vref;                    /**< Calibration data */
    uint16_t tRecover;                      /**< Calibration data */
    uint16_t vddaMv;                        /**< Measured Vdda voltage in mV */
    uint16_t vBusBMv;                       /**< Measured voltage of the analog muxbusB in mV */
    uint16_t vRefMv;                        /**< Vref value in mV */
    uint8_t vRefGain;                       /**< Vref gain */
    uint8_t activeCh;                       /**< ID of the channel is being converted:
                                             * * bit [0:4] - ID of current measured channel;
                                             * * CY_CSDADC_NO_CHANNEL - no active channel.
                                             */
    uint8_t snsClkDivider;                  /**< Divider of sense clock */
    uint8_t acqCycles;                      /**< Acquisition time in Sns cycles */
    uint8_t azCycles;                       /**< Auto-zero time in in Sns cycles */
}cy_stc_csdadc_context_t;

/** \} group_csdadc_data_structures */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
* \addtogroup group_csdadc_functions
* \{
*/
cy_en_csdadc_status_t Cy_CSDADC_Init(
                const cy_stc_csdadc_config_t * config,
                cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_Enable(
                cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_DeInit(
                cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_WriteConfig(
                const cy_stc_csdadc_config_t * config,
                cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_RegisterCallback(
                cy_csdadc_callback_t callbackFunction,
                cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_UnRegisterCallback(
                cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_StartConvert(
                uint32_t mode, 
                uint32_t chMask,
                cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_StopConvert(
                cy_en_csdadc_stop_mode_t stopMode,
                cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_IsEndConversion(
                const cy_stc_csdadc_context_t * context);
uint32_t Cy_CSDADC_ConversionStatus(
                const cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_Calibrate(
                cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_Wakeup(
                const cy_stc_csdadc_context_t * context);
cy_en_syspm_status_t Cy_CSDADC_DeepSleepCallback(
                cy_stc_syspm_callback_params_t * callbackParams,
                cy_en_syspm_callback_mode_t mode);
cy_en_csdadc_status_t Cy_CSDADC_Save(cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_Restore(cy_stc_csdadc_context_t * context);
uint32_t Cy_CSDADC_GetResult(
                uint32_t chId,
                const cy_stc_csdadc_context_t * context);
uint32_t Cy_CSDADC_GetResultVoltage(
                uint32_t chId,
                const cy_stc_csdadc_context_t * context);
uint32_t Cy_CSDADC_MeasureVdda(cy_stc_csdadc_context_t * context);
uint32_t Cy_CSDADC_MeasureAMuxB(cy_stc_csdadc_context_t * context);
void Cy_CSDADC_InterruptHandler(void * CSDADC_Context);

/** \} group_csdadc_functions */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CY_CSDADC_H */


/* [] END OF FILE */
