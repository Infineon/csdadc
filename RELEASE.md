# CYPRESS™ CSDADC Middleware Library 2.10

### What's Included?

For more information, refer to [README.md](./README.md) and the [API Reference Guide](https://infineon.github.io/csdadc/csdadc_api_reference_manual/html/index.html).
The revision history of the CSDADC Middleware is also available on the [API Reference Guide Changelog](https://infineon.github.io/csdadc/csdadc_api_reference_manual/html/index.html#group_csdadc_changelog).
New in this release:
* Added the support of PSoC™ 4 CAPSENSE™ Forth Generation devices

### Known Issues
| Problem | Workaround |
| ------- | ---------- |
| GPIO simultaneous operation with unrestricted strength and frequency creates noise that can affect CSDADC operation | Refer to the errata section of the device datasheet for details |

### Defect Fixes
* Improved input parameters check in the Cy_CSDADC_StartConvert() function

### Supported Software and Tools
This version of the CSDADC Middleware was validated for compatibility with the following software and tools:

| Software and Tools                                                   | Version |
| :---                                                                 | :----:  |
| ModusToolbox™ Software Environment                                   | 2.4.0   |
| - ModusToolbox™ Device Configurator                                  | 3.10    |
| - ModusToolbox™ CSD Personality  for PSoC™ 6 in Device Configurator  | 2.0     |
| - ModusToolbox™ CSD Personality  for PSoC™ 4 in Device Configurator  | 1.1     |
| PSoC™6 Peripheral Driver Library (PDL)                               | 2.4.0   |
| PSoC™4 Peripheral Driver Library (PDL)                               | 1.6.0   |
| GCC Compiler                                                         | 10.3.1  |
| IAR Compiler                                                         | 8.42.1  |
| ARM Compiler 6                                                       | 6.13    |
| MBED OS                                                              | 5.13.1  |
| FreeRTOS                                                             | 10.0.1  |

### More information
For more information, refer to:
* CSDADC overview:
  * [CSDADC Middleware RELEASE.md](./RELEASE.md)
  * [CSDADC Middleware API Reference Guide](https://infineon.github.io/csdadc/csdadc_api_reference_manual/html/index.html)
  * [CAPSENSE™ Middleware API Reference Guide](https://infineon.github.io/capsense/capsense_api_reference_manual/html/index.html)
  * [CSDIDAC Middleware API Reference Guide](https://infineon.github.io/csdidac/csdidac_api_reference_manual/html/index.html)
* ModusToolbox™ overview:
  * [ModusToolbox™ Software Environment, Quick Start Guide, Documentation, and Videos](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software/)
  * [ModusToolbox™ Device Configurator Tool Guide](https://documentation.infineon.com/html/modustoolbox-software/en/latest/tool-guide/ModusToolbox_Device_Configurator_Guide.html)
* Code Examples:
  * [CSDADC Middleware Code Example for MBED OS](https://github.com/Infineon/mbed-os-example-csdadc)
  * [CSDADC Middleware Code Examples for PSoC™ 6](https://github.com/Infineon/mtb-example-psoc6-csdadc)
* General information:
  * [PSoC™ Technical Reference Manual](https://www.infineon.com/cms/en/search.html#!term=PSoC%20Architecture%20Technical%20Reference%20Manual&view=downloads)
  * [PSoC™ 63 with BLE Datasheet Programmable System-on-Chip datasheet](http://www.cypress.com/ds218787)
  * [CAT1 PDL API Reference](https://infineon.github.io/mtb-pdl-cat1/pdl_api_reference_manual/html/index.html)
  * [CAT2 PDL API Reference](https://infineon.github.io/mtb-pdl-cat2/pdl_api_reference_manual/html/index.html)
  * [PSoC™ 4000S Family: PSoC™ 4 Architecture Technical Reference Manual (TRM)](https://www.infineon.com/dgdl/Infineon-PSoC_4000S_Family_PSoC_4_Architecture_Technical_Reference_Manual_(TRM)-AdditionalTechnicalInformation-v04_00-EN.pdf?fileId=8ac78c8c7d0d8da4017d0f915c737eb7)
  * [PSoC™ 4100S and PSoC™ 4100S Plus: PSoC™ 4 Architecture Technical Reference Manual (TRM)](https://www.infineon.com/dgdl/Infineon-PSoC_4100S_and_PSoC_4100S_Plus_PSoC_4_Architecture_TRM-AdditionalTechnicalInformation-v12_00-EN.pdf?fileId=8ac78c8c7d0d8da4017d0f9433460188)
  * [Infineon Technologies GitHub](https://github.com/Infineon)
  * [Infineon Technologies](http://www.infineon.com)

---
CYPRESS™ Semiconductor Corporation, 2019-2022.
