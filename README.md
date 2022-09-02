# CYPRESS™ CSDADC Middleware Library

### Overview
The CSDADC middleware is the CYPRESS™ ADC solution, which uses the CSD HW block for measurement. It is useful for a devices that do not include another ADC option. The CSDADC provides the following measurement capabilities:
* Voltage monitoring on multiple external channels.
* Voltage monitoring on AMUX-B.
* Device supply voltage (VDDA) monitoring without need of explicitly connecting VDDA to a GPIO input of ADC. This capability can be used to measure battery voltage and/or change VDDA dependent parameters of the ADC during run-time.

The listed capabilities make the CSDADC useful for a variety of applications including home appliances, automotive, IoT, and industrial applications. The CSDADC middleware can use the same CSD HW block with the other CSD-based middleware (CAPSENSE™, CSDIDAC, etc.) in the time-multiplexed manner.

### Features
* ADC with 8- and 10-bit resolution
* Two input measurement ranges: GND to VREF and GND to VDDA
* Two operation modes: Continuous conversion and single shot conversion

### Quick Start

The CSDADC can be configured by the ModusToolbox™ CSD personality. Refer to the [API Reference Guide Configuration Considerations](https://infineon.github.io/csdadc/csdadc_api_reference_manual/html/index.html#group_csdadc_configuration).


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
