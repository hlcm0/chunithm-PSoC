/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "stdio.h"
#include "stdlib.h"

#define ready 1
#define busy 0

int main(void)
{
    struct touchval
    {
        uint16 baseline[16];
        uint16 raw[16];                                         
    } touchval;
    uint8_t i2cbuf[64];

    CyGlobalIntEnable; /* Enable global interrupts. */

    CapSense_Start();
    CapSense_InitializeAllBaselines();
    CapSense_ScanEnabledWidgets();
    
    I2C_Start();
    I2C_EzI2CSetBuffer1( 64, 1, i2cbuf );

    for(;;)
    {

        if( ! CapSense_IsBusy() )
        {
            for (int i=0;i<16;i++)
            {
                touchval.baseline[i] = CapSense_GetBaselineData(i);
                touchval.raw[i] = CapSense_ReadSensorRaw(i);
            }
            memcpy(i2cbuf, &touchval, 64);
            CapSense_UpdateEnabledBaselines();
            left_ready_out_Write(ready);
            while (right_start_in_Read() != 1)
            {
                CyDelayUs(1);
            }
            CapSense_ScanEnabledWidgets();
            left_ready_out_Write(busy);
        }
    }
}

/* [] END OF FILE */
