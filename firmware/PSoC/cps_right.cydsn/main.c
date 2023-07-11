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

#define start 1
#define running 0

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
            while (left_ready_in_Read() != 1) //等待另一个PSoC准备好开始下一次扫描
            {
                CyDelayUs(1);
            }
            CapSense_ScanEnabledWidgets();
            right_start_out_Write(start); //向right_start信号线写入1表明扫描开始，使另一个PSoC也开始扫描
            CyDelayUs(50); //延迟50微秒确保另一个PSoC收到信号
            right_start_out_Write(running); //向right_start信号线写入0表明正在扫描，请勿进行下一次扫描
        }
    }
}

/* [] END OF FILE */
