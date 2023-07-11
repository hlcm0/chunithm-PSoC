#include "sensor.h"
#include "Adafruit_TinyUSB.h"
#include "report.h"
#include "Adafruit_NeoPixel.h"

Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 1, true);

void setup() {
  // put your setup code here, to run once:
  touchInit(); //初始化与触摸芯片的I2C通讯
  buttonInit(); //初始化功能键引脚
  TinyUSBDevice.setSerialDescriptor("HLCM"); //设定USB设备序列号
  TinyUSBDevice.setID(0x1973,0x2001); //设定USB设备vid，pid
  TinyUSBDevice.setProductDescriptor("psocnithm"); //设定USB设备产品名
  TinyUSBDevice.setManufacturerDescriptor("HLCM.co"); //设定USB设备制造商名
  usb_hid.setPollInterval(1); //设置最小1ms的回报间隔
  usb_hid.setReportCallback(get_report_callback, set_report_callback); //当电脑向手台发送数据时会调用set_report_callback进行处理
  usb_hid.begin();
  while (!TinyUSBDevice.mounted()) delay(1);  //如果没插入则等待
  while (!usb_hid.ready()) delay(1);
}

struct inputdata data_tx;
struct usb_output_data_1 data_rx_1;
struct usb_output_data_2 data_rx_2;

void loop() {
  // put your main code here, to run repeatedly: 
  updateSensorValue(); //从触摸芯片获取值
  updateTouchData(); //将值处理后放入TouchData数组
  updateButton(); //从按键获取值
  data_tx.IRValue = getAirVal(); //获取AirVal
  data_tx.Buttons = getButtonVal(); //获取按钮值
  for (int i = 0; i < 32; i++)
  {
    data_tx.TouchValue[i] = getTouchData(i); //获取触摸数值
  }
  data_tx.CardStatue = 0;
  for (int i = 0; i < 10; i ++)
  {
    data_tx.CardID[i] = 0; //读卡，未实现
  }

  usb_hid.sendReport(0, &data_tx, sizeof(data_tx)); //发送报告
}

uint16_t get_report_callback (uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // not used in this example
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;
  return 0;
}

void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  // This example doesn't use multiple report and report ID
  (void) report_id;
  (void) report_type;

  //buffer即为从电脑收到的数据，首先判断第1byte的数值，来选择解析为data_rx_1还是data_rx_2 (see definition in report.h)
  if (buffer[0] == 0)
  {
    memcpy(&data_rx_1, buffer, bufsize);
  }
  else
  {
    memcpy(&data_rx_2, buffer, bufsize);
  }
}



#define RGBpin 7
Adafruit_NeoPixel panelRGB(33, RGBpin, NEO_GRB + NEO_KHZ800);
void setup1()
{
  airInit(); //对Air相关端口
  airThresholdInit(); //初始化Air阈值
  panelRGB.begin();
  panelRGB.setBrightness(127);
}

uint16_t starthue = 0;
uint16_t hue = 0;
void loop1() {
  // put your main code here, to run repeatedly:

  for (int i = 0; i < 16; i++) {
    if ((data_tx.TouchValue[i*2 + 1] > 40) || (data_tx.TouchValue[i*2] > 40))
    {
      panelRGB.setPixelColor(i*2 + 1, 255, 255, 255);
    }
    else if ((data_tx.TouchValue[i*2 + 1] < 35) && (data_tx.TouchValue[i*2] < 35))
    {
      panelRGB.setPixelColor(i*2 + 1, 0);
    }
    hue += 400;
  }

  panelRGB.show();
  updateAirVal(); //更新AirVal
}