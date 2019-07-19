/***************************************************************************//**
* \file cy_csdadc.h
* \version 2.0
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
* The CSDADC middleware is the Cypress ADC solution that uses the CSD HW block.
* The CSD HW block is mainly used to implement the touch sense applications and
* proximity sensors (refer to the
* <a href="https://cypresssemiconductorco.github.io/capsense/capsense_api_reference_manual/html/index.html">
* <b>CapSense Middleware API Reference Guide</b></a>), but can also
* be used to implement the ADC, which is especially useful for the devices that
* do not include another hardware option to implement the ADC.
* CSDADC provides the following measurement capabilities:
* * Voltage monitoring on multiple external channels.  Any GPIO that can be
*   connected to AMUX-B (refer to the particular device datasheet for information)
*   can be an input to the CSDADC under software control. 
* * Voltage monitoring on AMUX-B.
* * Device supply voltage (VDDA) monitoring without the need of explicitly connecting
*   VDDA to a GPIO input of the ADC. This capability can be used to measure battery
*   voltages and/or change VDDA-dependent parameters of the ADC during run-time.
*
* The listed capabilities are making the CSDADC useful for a variety of
* applications, including home appliances, automotive, IoT, and industrial
* applications. The CSDADC middleware can use the same CSD HW block with other
* CSD-based middleware (CapSense, CSDIDAC, etc) in time-multiplexed manner.
*
* <b>Features:</b>
* * ADC with 8- and 10-bit resolution
* * Two input measurement ranges: GND to VREF and GND to VDDA
* * Two operation modes: Continuous conversion and Single-shot conversion
*
********************************************************************************
* \section section_csdadc_general General Description
********************************************************************************
*
* Include cy_csdadc.h to get access to all functions and other declarations in
* this library.
* The \ref group_csdadc_quick_start is offered in this API Reference Guide.
*
* Refer to the \ref section_csdadc_toolchain for the compatibility
* information.
*
* Refer to the \ref group_csdadc_changelog for the differences between the
* Middleware versions.
*
* The \ref group_csdadc_changelog also describes the impact of the changes to
* your code.
*
* The CSD HW block enables multiple sensing capabilities on PSoC devices 
* including the self-cap and mutual-cap capacitive touch sensing solutions, 
* a 10-bit ADC, IDAC, and Comparator. The CSD driver is a low-level 
* peripheral driver, a wrapper to manage access to the CSD HW block. 
* Any middleware access to the CSD HW block happens through the CSD Driver. 
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
* This section describes only the CSDADC middleware. Refer to the corresponding 
* sections for documentation of other middleware supported by the CSD HW block.
* The CSDADC library is designed to use with the CSD driver.
* The application program does not need to interact with the CSD driver 
* and/or other drivers such as GPIO or SysClk directly. All of that is 
* configured and managed by the middleware.
*
* The CSDADC API is described in the following sections:
* * \ref group_csdadc_macros
* * \ref group_csdadc_data_structures
* * \ref group_csdadc_enums
* * \ref group_csdadc_functions
*
* \warning
* I2C transactions during ADC conversions may lead to measurement result 
* distortions. Perform ADC conversions and I2C communications in 
* Time-sharing mode.
*
********************************************************************************
* \section group_csdadc_quick_start Quick Start Guide
********************************************************************************
*
* Cypress CSDADC middleware can be used in various Development Environments
* such as ModusToolbox, MBED, etc. Refer to the \ref section_csdadc_toolchain.
* The quickest way to get started is using the Code Examples.
* Cypress Semiconductor continuously extends its portfolio of the code examples
* at the <a href="http://www.cypress.com"><b>Cypress Semiconductor website</b></a>
* and at the <a href="https://github.com/cypresssemiconductorco">
* <b>Cypress Semiconductor GitHub</b></a>.
*
* This quick start guide assumes that the environment is configured to use the
* PSoC 6 Peripheral Driver Library(psoc6pdl) for development and the
* PSoC 6 Peripheral Driver Library(psoc6pdl) is included in the project.
*
* The following steps are required to set up the CSDADC and to run the
* measurement:
* 1. Set up the CSDADC configuration manually or by using the Device Configurator
*    as described in the \ref group_csdadc_configuration section.
* \note
* Put the CSDADC name to the Alias field of the CSD resource if the
* Device Configurator is used.
* 
* 2. Include cy_csdadc.h to get access to all CSDADC API and cy_pdl.h to get
*    access to API of peripheral drivers according to the example below:
* \snippet csdadc/snippet/main.c snippet_required_includes
* 3. If you use the MBED OS, include the cycfg.h file to get access to the
*    System Configuration:
* \snippet csdadc/snippet/main.c snippet_mbed_required_includes
* 4. Declare the 'cy_csdadc_context' variable as per example below:
* \snippet csdadc/snippet/main.c snippet_csdadc_context_declaration
* 5. Declare and initialize the CSDADC_ISR_cfg variable as per example below:
* \snippet csdadc/snippet/main.c snippet_m4_adc_interrupt_source_declaration
* 6. Define the CSDADC interrupt handler according to the example below:
* \snippet csdadc/snippet/main.c snippet_CSDADC_Interrupt
* 7. Update the main() routine with the following code:
* \snippet csdadc/snippet/main.c snippet_csdadc_chResult_declaration
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_RunConversion
*
********************************************************************************
* \section group_csdadc_configuration Configuration Considerations
********************************************************************************
*
* The CSDADC middleware operates on the top of the CSD Driver included in the
* PSoC 6 Peripheral Driver Library (psoc6pdl). Refer to the "CSD(CapSense
* Sigma Delta)" section of the PSoC 6 Peripheral Driver Library (psoc6pdl) API
* Reference Manual.
* This Configuration Considerations section guides how to set up the
* CSDADC middleware for the operation with the following parameters:
* 1. Device VDDA: 3.3V.
* 2. Device Peri Clock frequency: 48MHz.
* 3. Desired Number of Input Channels: 2 (Ch-1 is assigned to P6[2],
     Ch-2 is assigned to P6[3]).
* 4. Desired Resolution: 10 bit.
* 5. Desired Measurement Range: GND to VDDA.
* 
* There are two methods for the CSDADC Middleware configuration:
* 1. \ref group_csdadc_mtb_configuring
* 2. \ref group_csdadc_manual_configuring
*
* Generation of the initialization code using the
* <a href="https://www.cypress.com/ModusToolboxDeviceConfig">
* <b>ModusToolbox Device Configurator Tool </b></a> which is part of the
* <a href="https://www.cypress.com/products/modustoolbox-software-environment">
* <b>ModusToolbox</b></a>, greatly simplifies the PSoC configuration.
* The <a href="https://www.cypress.com/ModusToolboxDeviceConfig"><b>ModusToolbox
* Device Configurator Tool </b></a> provides the user interface to set up and
* automatically generate the initialization code (including analog routing) and
* configuration structures.
* 
* Manual implementation of the initialization code (including analog routing)
* and configuration structures is recommended for expert Users only. This will
* include the code for the following settings which in case of the
* Device Configurator usage are generated automatically based upon the settings
* entered in its UI:
*  * Assigning the Peripheral Clock Divider.
*  * Configuring the HSIOM_AMUX_SPLIT_CTL switches to route signal from input
*    pins configured as the CSDADC channels to the CSD HW block.
*  * Declaration and initialization of the CSDADC configuration structure.
*  * Declaration and initialization of the CSD HW driver context structure.
*  * Definition of the of the CSD HW block base address.
*
********************************************************************************
* \subsection group_csdadc_mtb_configuring Use ModusToolbox Device Configurator Tool to generate initialization code
********************************************************************************
*
* The following steps are required to generate the initialization code using the
* <a href="https://www.cypress.com/ModusToolboxDeviceConfig">
* <b>ModusToolbox Device Configurator Tool </b></a>:
* 1. Launch the ModusToolbox Middleware Selector and enable the CSD ADC
*    middleware. This step is required only if the ModusToolbox IDE is used.
*    Otherwise, ensure the CSDADC Middleware is included in your project.
* 2. Launch the ModusToolbox Device Configurator Tool.
* 3. Switch to the System tab. Configure the CLK_PERI frequency to achieve 48MHz
*    (you may need to change the FLL or PLL frequency) and set the VDDA voltage
*    to 3.3V in Power/MCU Personality.
* 4. Switch to the Peripherals tab (#1 on figure below). Enable the CSD personality
*    under System (#2 on figure below) and enter Alias (#3 on figure below).
*    We use CSDADC in \ref group_csdadc_quick_start.
* 5. Go to the Parameters Pane and configure the CSD Personality:
*  * Assign the peripheral clock divider by using the Clock
*    combo box(#4 on figure below). Any free divider can be used.
*  * Set the Enable CSDADC check box (#5 on figure below).
*  * Configure the CSDADC with the desired parameters per
*    \ref group_csdadc_configuration (#5 on figure below).
*  * Assign the CSDADC Channels to pins per \ref group_csdadc_configuration
*    (#6 on figure below).
* 6. Perform File->Save to generate initialization code.
*
* \image html csdadc_config.png "CSDADC configuration" width=1175px
* \image latex csdadc_config.png
*
* Now, all required CSDADC initialization code and configuration prerequisites
* will be generated:
* * The Peripheral Clock Divider assignment and analog routing are parts of
*   the init_cycfg_all() routine. Place the call of the init_cycfg_all() function
*   before using any CSDADC API functions to ensure initialization of all
*   external resources required for the CSDADC operation. 
*   Refer to the main() routine code snippet in
*   \ref group_csdadc_quick_start
* * The CSDADC configuration structure declaration in the
*   cycfg_peripherals.h file and its initialization in the
*   cycfg_peripherals.c file. The variable name is
*   \<Alias_Name\>_csdadc_config.
* * The CSD HW driver context structure declaration in the
*   cycfg_peripherals.h file and its initialization in the
*   cycfg_peripherals.c file. The variable name is
*   cy_csd_\<CSD_Block_Index\>_context.
* * The CSD HW block base address definition is in the
*   cycfg_peripherals.h file.
* * The definition name is \<Alias_Name\>_HW.
*
* The generated code will be available under the GeneratedSource folder.
*
* Refer to \ref group_csdadc_quick_start section for the application layer code
* required to set up CSDADC and run the measurement.
*
********************************************************************************
* \subsection group_csdadc_manual_configuring Implement the initialization code manually
********************************************************************************
*
* The steps required to implement the initialization code manually:
* 1. Launch the ModusToolbox Middleware Selector and enable the
*    CSD ADC middleware. This step is required only if the ModusToolbox IDE
*    is used.
*    Otherwise, ensure the CSDADC Middleware is included in your project.
* 2. Define the CSD HW block base address. See the code example below:
* \snippet csdadc/snippet/main.c snippet_csd_hw_definition  
* 3. Declare the CSD HW driver context structure and initialize the
*    lockKey field with the CY_CSD_NONE_KEY value. See the code example below:
* \snippet csdadc/snippet/main.c snippet_csd_context_declaration
* 4. Declare the CSDADC configuration structure and initialize it according
*    to the desired parameters. See the code example below:
* \snippet csdadc/snippet/main.c snippet_csd_div_index_definition
* \snippet csdadc/snippet/main.c snippet_csdadc_config_declaration
* 5. Assign the Peripheral Clock Divider to the CSD HW block.
*    See the code example below and refer to the main() routine code snippet in
*    \ref group_csdadc_quick_start
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_Clock_Assignment
* 6. Set the configuration of the HSIOM_AMUX_SPLIT_CTL switches to route signal
*    from input pins configured as the CSDADC channels to the
*    CSD HW block. The AMUX_SPLIT_CTL[4] switches are closed to connect
*    port P6 with the CSD HW block. Refer to the
*    <a href="http://www.cypress.com/trm218176"><b>Technical Reference Manual
*    (TRM)</b></a> for more information regarding the analog interconnection.
*    See the code example below and refer to the main() routine code snippet in
*    \ref group_csdadc_quick_start
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_Amux_Configuration
*
* Refer to \ref group_csdadc_quick_start section for the application layer code
* required to set up CSDADC and run the measurement.
*
********************************************************************************
* \section group_csdadc_use_cases Use Cases
********************************************************************************
*
* This section provides descriptions and links to additional documentation for
* some specific CSDADC use cases.
*
********************************************************************************
* \subsection group_csdadc_low_power_design Low Power Design
********************************************************************************
*
* The CSD HW block and CSDADC middleware can operate in CPU active and CPU sleep 
* power modes. It is also 
* possible to switch between low power and ultra low power system modes.
* The CSD HW block interrupt can wake-up the CPU from sleep mode.
* In System Deep Sleep and Hibernate power modes, the CSD HW block is 
* powered off and CSDADC conversions are not performed.
* When the device wakes up from CPU / System Deep Sleep, the CSD HW block resumes operation 
* without the need for re-initialization and the CSDADC conversions 
* can be continued with a configuration that was set before CPU / System Deep Sleep 
* transition. When the device wakes up from System Hibernate
* power mode, the CSD HW block does not retain configuration and CSDADC requires
* re-initialization.
* If the user performs some communications to transmit CSDADC results via I2C, 
* UART etc., transitions to CPU sleep, Deep Sleep or Hibernate modes are performed
* with recommendations both for CSDADC and communications' drivers/middleware.
*
* \note
* 1. CPU can seamlessly enter and exit CPU sleep mode while the CSD HW block is busy.
*    However, do not put the CSD HW block into block low power mode during the
*    conversion as it may lead to unexpected behavior.
*
* 2. Entering CPU Deep Sleep mode does not mean the device enters 
*    System Deep Sleep. For more detail about switching to System Deep Sleep,
*    refer to the device TRM.
*
* 3. The analog start-up time for the CSD HW block is 25 us. Initiate
*    any kind of conversion only after 25 us from System Deep Sleep / Hibernate exit.
*   
* Refer to the Cy_CSDADC_DeepSleepCallback() function description and to the
* SysPm (System Power Management) driver documentation for the low power design
* considerations.
*
* <b>Sleep mode</b><br>
* The CSD HW block can operate in the CPU sleep mode. The user can start CSDADC
* and move a CPU into sleep mode. After every conversion, the CPU is 
* woken-up by the CSD interrupt, the results are read, and the CPU goes to
* sleep again to reduce a power consumption. After the whole conversion cycle
* completes, the user can read results, proccess them, and start a new cycle. Then, 
* the user configures the CSDADC middleware as described in 
* \ref group_csdadc_configuration, and updates the main() routine with 
* the following code: 
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_Sleep
*
* <b>Deep Sleep mode</b><br>
* To use the CSDADC middleware in the CPU / System Deep Sleep mode, the user configures
* a wake-up source (e.g. a pin, WDT, LPC or another entities, that are active 
* in CPU / System Deep Sleep mode), configures the CSDADC middleware as described in 
* \ref group_csdadc_configuration, configures CSDADC and other drivers' and 
* middleware's (if presented) Deep Sleep Callback structures, registers 
* callbacks, and updates the main() routine with the following code: 
* \snippet csdadc/snippet/main.c snippet_CSDADC_DeepSleep_structures
* \snippet csdadc/snippet/main.c snippet_Cy_CSDADC_DeepSleep
*
********************************************************************************
* \subsection group_csdadc_calibration ADC calibration
********************************************************************************
*
* Refer to the Cy_CSDADC_Calibrate() function description for the CSDADC
* calibration considerations. Cy_CSDADC_Enable() performs first-time
* calibration at the start of CSDADC operation. Periodical re-calibrations
* are required to keep the measurement results accurate.
*
********************************************************************************
* \subsection group_csdadc_time_multiplexing Time-multiplexing operation of CSDADC and CapSense
********************************************************************************
*
* Refer to the Cy_CSDADC_Save() and Cy_CSDADC_Restore() functions descriptions
* to implementat the time-multiplexing operation of CSDADC and CapSense by
* using a common CSD HW block.
*
********************************************************************************
* \section section_csdadc_toolchain Supported Software and Tools
********************************************************************************
*
* This version of the CSDADC Middleware was validated for the compatibility 
* with the following Software and Tools:
* 
* <table class="doxtable">
*   <tr>
*     <th>Software and Tools</th>
*     <th>Version</th>
*   </tr>
*   <tr>
*     <td>ModusToolbox Software Environment</td>
*     <td>2.0</td>
*   </tr>
*   <tr>
*     <td>- ModusToolbox Device Configurator</td>
*     <td>2.0</td>
*   </tr>
*   <tr>
*     <td>- ModusToolbox CSD Personality in Device Configurator</td>
*     <td>2.0</td>
*   </tr>
*   <tr>
*     <td>PSoC6 Peripheral Driver Library (PDL)</td>
*     <td>1.2.0</td>
*   </tr>
*   <tr>
*     <td>GCC Compiler</td>
*     <td>7.2.1</td>
*   </tr>
*   <tr>
*     <td>IAR Compiler</td>
*     <td>8.32</td>
*   </tr>
*   <tr>
*     <td>Arm Compiler 6</td>
*     <td>6.11</td>
*   </tr>
*   <tr>
*     <td>MBED OS</td>
*     <td>5.13.1</td>
*   </tr>
*   <tr>
*     <td>FreeRTOS</td>
*     <td>10.0.1</td>
*   </tr>
* </table>
*
********************************************************************************
* \section section_csdadc_update Update to Newer Versions
********************************************************************************
* Consult \ref group_csdadc_changelog to learn about the design impact of the
* newer version. Set up your environment in accordance with
* \ref section_csdadc_toolchain. You might need to re-generate the configuration
* structures for either the device initialization code or the middleware
* initialization code.
*
* Ensure:
* * The specified version of the ModusToolbox Device Configurator and
*   the CSD personality are used to re-generate the device configuration.
* * The toolchains are set up properly for your environment per the settings
*   outlined in the Supported Software and Tools.
* * The project is re-built once the the toolchains are configured and the
*   configuration is completed.
*
********************************************************************************
* \section group_csdadc_MISRA MISRA-C Compliance
********************************************************************************
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
*     <td>Such a conversion is performed with the CSDADC context in two cases:
*         the interrupt handler and DeepSleepCallback function.
*         Both cases are verified on the correct operation.</td>
*   </tr>
*   <tr>
*     <td>1.2</td>
*     <td rowspan=2> R</td>
*     <td rowspan=2> Constant: Dereference of NULL pointer.</td>
*     <td rowspan=2> These violations are reported to result from using 
*         offset macros of the CSD Driver with corresponding documented 
*         violation 20.6. Refer to the CSD Driver API Ref Guide.</td>
*   </tr>
*   <tr>
*     <td>20.3</td>
*   </tr>
* </table>
*
********************************************************************************
* \section section_csdadc_errata Errata
********************************************************************************
*
* This section lists the known problems with the CSDADC middleware:
* 
* <table class="doxtable">
*   <tr><th>Cypress ID</th><th>Known Issue</th><th>Workaround</th></tr>
*   <tr>
*     <td>319100</td>
*     <td>
*         The GPIO simultaneous operation with unrestricted strength and
*         frequency creates noise that can affect the CSDADC operation.
*     </td>
*     <td>
*         Refer to the errata section of the device datasheet for details.<br>
*         <a href="http://www.cypress.com/ds218787"><b>PSoC 63 with BLE 
*         Datasheet Programmable System-on-Chip</b></a>
*     </td>
*   </tr>
* </table>
* 
********************************************************************************
* \section group_csdadc_changelog Changelog
********************************************************************************
*
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td rowspan="8">2.0</td>
*     <td>Made public the CY_CSDADC_NO_CHANNEL macro</td>
*     <td>Defect fixing</td>
*   </tr>
*   <tr>
*     <td>Changed the Cy_CSDADC_InterruptHandler() function prototype. 
*         Added function argument: const CSD_Type * base </td>
*     <td>User experience improvement</td>
*   </tr>
*   <tr>
*     <td>Changed the Cy_CSDADC_StartConvert() function prototype. 
*         Changed the mode argument type from uint32_t to
*         cy_en_csdadc_conversion_mode_t</td>
*     <td>User experience improvement</td>
*   </tr>
*   <tr>
*     <td>Renamed function Cy_CSDADC_ConversionStatus() to 
*         Cy_CSDADC_GetConversionStatus()</td>
*     <td>User experience improvement</td>
*   </tr>
*   <tr>
*     <td>The CSDADC MW sources are enclosed with the conditional compilation to 
*         ensure a successful compilation for non-CSDADC-capable devices</td>
*     <td>A compilation for non-CSDADC-capable devices</td>
*   </tr>
*   <tr>
*     <td>After conversion, the ADC channel is still connected to the
*         CSD HW block, and disconnected only prior to new channel connection</td>
*     <td>Defect fixing</td>
*   </tr>
*   <tr>
*     <td>Changed the cy_stc_csdadc_config_t structure: the ptrPin field is
*         replaced with the ptrPinList field. It is a pointer to the array of
*         the size determined by the numChannels field of this structure versus
*         the ptrPin field that was an array of pointers with fixed
*         \ref CY_CSDADC_MAX_CHAN_NUM size.
*     <td>User experience improvement</td>
*   </tr>
*   <tr>
*     <td>Added the cpuClk field to the cy_stc_csdadc_config_t structure.
*         Changed the software watchdog counter calculation in the Cy_CSDADC_Restore() 
*         function where cpuClk is used instead of periClk.
*     <td>Defect fixing</td>
*   </tr>
*   <tr>
*     <td rowspan="2">1.0.1</td>
*     <td>Improvements to documentation</td>
*     <td>User experience improvement</td>
*   </tr>
*   <tr>
*     <td>Forbidden usage of CSDADC for non-CSDADC-capable devices in module.mk 
*         file</td>
*     <td>A Compilation for non-CSDADC-capable devices</td>
*   </tr>
*   <tr>
*     <td>1.0</td>
*     <td>The initial version</td>
*     <td></td>
*   </tr>
* </table>
*
********************************************************************************
* \section group_csdadc_more_information More Information
********************************************************************************
*
* For more information, refer to the following documents:
*
* * <a href="https://www.cypress.com/products/modustoolbox-software-environment">
*   <b>ModusToolbox Software Environment, Quick Start Guide, Documentation,
*   and Videos</b></a>
*
* * <a href="https://github.com/cypresssemiconductorco"><b>CSDADC Middleware
*   Code Example for MBED OS</b></a>
*
* * <a href="https://github.com/cypresssemiconductorco"><b>CSDADC Middleware
*   Code Examples at GITHUB</b></a>
*
* * <a href="https://www.cypress.com/ModusToolboxDeviceConfig"><b>ModusToolbox
*   Device Configurator Tool Guide</b></a>
*
* * <a href="https://cypresssemiconductorco.github.io/capsense/capsense_api_reference_manual/html/index.html">
*   <b>CapSense Middleware API Reference Guide</b></a>
*
* * <a href="https://cypresssemiconductorco.github.io/csdidac/csdidac_api_reference_manual/html/index.html">
*   <b>CSDIDAC Middleware API Reference Guide</b></a>
*
* * <a href="https://github.com/cypresssemiconductorco.github.io/psoc6pdl/pdl_api_reference_manual/html/index.html">
*   <b>PDL API Reference</b></a>
*
* * <a href="https://www.cypress.com/documentation/technical-reference-manuals/psoc-6-mcu-psoc-63-ble-architecture-technical-reference">
*   <b>PSoC 6 Technical Reference Manual</b></a>
*
* * <a href="http://www.cypress.com/ds218787">
*   <b>PSoC 63 with BLE Datasheet Programmable System-on-Chip datasheet</b></a>
*
* * <a href="http://www.cypress.com"><b>Cypress Semiconductor</b></a>
*
* \note
* The links to another software component’s documentation (middleware and PDL) 
* point to GitHub to the latest available version of the software. 
* To get documentation of the specified version, download from GitHub and unzip 
* the component archive. The documentation is available in the <i>docs</i> folder.
*
* \defgroup group_csdadc_macros Macros
* \brief
* This section describes the CSDADC Macros. These Macros can be used to 
* check the maximum channel number for defining CSDADC conversion mode and 
* for checking CSDADC conversion counters. Detailed information about the
* macros is available in each macro description. 
*
* \defgroup group_csdadc_enums Enumerated types
* \brief
* Describes the enumeration types defined by the CSDADC. These enumerations 
* can be used for checking the CSDADC functions return statuses
* for defining a CSDADC resolution and an input voltage range and for 
* defining Start and Stop conversion modes.  Detailed information about
* the enumerations is available in each enumeration description. 
*
* \defgroup group_csdadc_data_structures Data Structures
* \brief
* Describes the data structures defined by the CSDADC. The CSDADC MW 
* use structures for the input channel pins, conversion results, 
* MW configuration and context. The pin structure is included into 
* the configuration structure and both can be defined by the 
* user with the CSD personality in Device Configurator or manually if  
* the user does not use ModusToolbox.
* The result structure is included into the context structure and 
* contains voltages and ADC codes for all 32 input channels of more 
* recent conversions. Besides the result structure, the context structure 
* contains a copy of the configuration structure, the current CSDADC MW state 
* data and calibration data. The context structure is allocated by the user 
* and passed to all CSDADC MW functions. 
* The CSDADC MW structure sizes are shown in the table below:
* 
* <table class="doxtable">
*   <tr><th>Structure</th><th>Size in bytes (w/o padding)</th></tr>
*   <tr>
*     <td>cy_stc_csdadc_ch_pin_t</td>
*     <td>5</td>
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
*   Cy_CSDADC_RegisterCallback();
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

#if defined(CY_IP_MXCSDV2)

/* The C binding of definitions if building with the C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* \addtogroup group_csdadc_macros
* \{
*/
/** Middleware major version */
#define CY_CSDADC_MW_VERSION_MAJOR              (2)

/** Middleware minor version */
#define CY_CSDADC_MW_VERSION_MINOR              (0)

/** CSDADC PDL ID */
#define CY_CSDADC_ID                            (CY_PDL_DRV_ID(0x43u))

/** The CSDADC max channels number */
#define CY_CSDADC_MAX_CHAN_NUM                  (32u)
/** The parameter for no active CSDADC channel indication */
#define CY_CSDADC_NO_CHANNEL                    (0xFFu)

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


/* 
* These defines are obsolete and kept for backward compatibility only. 
* They will be removed in the future versions.
*/
#define CY_CSDADC_MDW_VERSION_MAJOR             (CY_CSDADC_MW_VERSION_MAJOR)
#define CY_CSDADC_MDW_VERSION_MINOR             (CY_CSDADC_MW_VERSION_MINOR)


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
* the user should call the Cy_CSDADC_GetConversionStatus() function. */
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

/**
* CSDADC pin structure. This structure contains information about Port/Pin
* assignment of a particular CSDADC channel. The CSDADC channel can be assigned to
* any GPIO that supports the static connection to the AMUX-B (refer to the
* particular device datasheet for information).
*/
typedef struct {
    GPIO_PRT_Type * ioPcPtr;                /**< Pointer to channel IO PC register */
    uint8_t pin;                            /**< Channel IO pin */
} cy_stc_csdadc_ch_pin_t;

/** CSDADC configuration structure */
typedef struct
{
    const cy_stc_csdadc_ch_pin_t * ptrPinList;
                                            /**< Array of pointers to the channel IO structures */
    CSD_Type * base;                        /**< Pointer to the CSD HW Block */
    cy_stc_csd_context_t * csdCxtPtr;       /**< Pointer to the CSD context driver */
    uint32_t cpuClk;                        /**< CPU Clock in Hz */
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
                cy_en_csdadc_conversion_mode_t mode,
                uint32_t chMask,
                cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_StopConvert(
                cy_en_csdadc_stop_mode_t stopMode,
                cy_stc_csdadc_context_t * context);
cy_en_csdadc_status_t Cy_CSDADC_IsEndConversion(
                const cy_stc_csdadc_context_t * context);
uint32_t Cy_CSDADC_GetConversionStatus(
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
void Cy_CSDADC_InterruptHandler(const CSD_Type * base, void * CSDADC_Context);

/** \} group_csdadc_functions */

/*******************************************************************************
* Function Name: Cy_CSDADC_ConversionStatus
****************************************************************************//**
*
* This function is obsolete and kept for backward compatibility only. 
* The Cy_CSDADC_GetConversionStatus() function should be used instead.
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
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_CSDADC_ConversionStatus(const cy_stc_csdadc_context_t * context)
{
    return(Cy_CSDADC_GetConversionStatus(context));
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CY_IP_MXCSDV2 */

#endif /* CY_CSDADC_H */


/* [] END OF FILE */
