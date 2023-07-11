# chunithm-PSoC
一个可参考的chunithm手台使用PSoC芯片的例子

## 大体结构
* 使用RP2040开发板作为主控芯片
* 两块PSoC4（cy8c4245axi-483）作为触摸芯片，通过I2C与RP2040通讯
* 由于RP2040的ADC外设数量不足，使用74hc4051多路模拟复用器芯片来轮询六个红外接收
* 使用yubideck的io进行通讯

## 特别感谢
* 凌鸽在触摸干扰调试和Air调试上提供的帮助（Air校准部分参考自LINGNITHM项目）
* yubideck开放的io协议

## 关于PSoC
* 需要的软件：PSoC Creator
* PSoC Creator的使用可以观看youtube上的PSoC101相关教程，也可以来三连支持一下我在bilibili上传的渣翻的[PSoC101教程](https://www.bilibili.com/video/BV1js4y1y7VQ/)（
* PSoC4的烧录也许可以使用普通的SWD烧录器（如jlink，daplink等），但我并没摸索出来。目前已知可行的办法：</br>
1.使用kitprog（可从CY8CKIT-147上掰下来）</br>
2.使用miniprog（将近一千块钱，好贵）</br>
3.使用arduino作为其SWD烧录器（我没试过，不过应该可以），详见项目[PSoC4_HSSP_Arduino](https://github.com/k4zuk/PSoC4_HSSP_Arduino)</br>

## 备注
* pid，vid，设备名和制造商名可以在代码中修改了，而不需要修改boards.txt
* 受预算和个人精力限制，本项目的cad以及支架设计极为落后，所以仅供软硬件方案参考，可参考更完善成熟的方案，如LINGNITHM、GSK等
* 可以将I2C通讯更改为UART通讯来达成更高的传输速度
* 板子上加了一个EEPROM，连接到了RP2040的I2C0外设，可用于保存用户配置（未实现），因为RP2040没有自带EEPROM
* 暂未编写灯光反馈功能
* 如果想适配原生串口，需要修改TinyUSB库的部分代码来删除其中的DTR信号检查
* 两个芯片之间通过left_ready（左芯片准备好开始下一次扫描），right_start（右芯片开始下一次扫描）两个信号线来保证两个芯片的同步，从而防止扫描的干扰。可以利用上两个PSoC上剩余空闲的引脚来实现其他功能（如PSoC的复位，数据传输等等）
* 单次扫描周期目前实测为23ms，可能需要修改PSoC代码的部分配置来达成更高的扫描速度，详见[CapSense Design Guide](https://www.infineon.com/dgdl/Infineon-AN85951_PSoC_4_CapSense_Design_Guide-ApplicationNotes-v27_00-CN.pdf?fileId=8ac78c8c7cdc391c017d07235d2d4679)
