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

    CapSense_Start(); //启动CapSense
    CapSense_InitializeAllBaselines(); //初始化Baseline
    CapSense_ScanEnabledWidgets(); //开始扫描所有区块
    
    I2C_Start(); //启动I2C
    I2C_EzI2CSetBuffer1( 64, 1, i2cbuf ); //设置I2C buffer

    for(;;)
    {

        if( ! CapSense_IsBusy() ) //等待扫描完成
        {
            for (int i=0;i<16;i++)
            {
                touchval.baseline[i] = CapSense_GetBaselineData(i);
                touchval.raw[i] = CapSense_ReadSensorRaw(i);
            }
            memcpy(i2cbuf, &touchval, 64); //将触摸数据转入I2C buffer
            CapSense_UpdateEnabledBaselines(); //根据上一次扫描数据更新Baseline
            left_ready_out_Write(ready); //向left_ready信号线写入1来表明已准备好开始下一次扫描
            while (right_start_in_Read() != 1) //等待另一个PSoC发出的right_start信号
            {
                CyDelayUs(1);
            }
            CapSense_ScanEnabledWidgets(); //开始扫描所有区块
            left_ready_out_Write(busy); //向left_ready写入0表明正在繁忙，不能开始下一次扫描
        }
    }
}

/* [] END OF FILE */
