// RawHID might never work with multireports, because of OS problems
// therefore we have to make it a single report with no ID. No other HID device will be supported then.
#undef RAWHID_USAGE_PAGE
#define RAWHID_USAGE_PAGE	0xFFC0 // recommended: 0xFF00 to 0xFFFF

#undef RAWHID_USAGE
#define RAWHID_USAGE		0x0C00 // recommended: 0x0100 to 0xFFFF


uint8_t const desc_hid_report[] =
{
  	/*    RAW HID */
    0x06, lowByte(RAWHID_USAGE_PAGE), highByte(RAWHID_USAGE_PAGE),      /* 30 */
    0x0A, lowByte(RAWHID_USAGE), highByte(RAWHID_USAGE),

    0xA1, 0x01,                  /* Collection 0x01 */
    // RawHID is not multireport compatible.
    // On Linux it might work with some modifications,
    // however you are not happy to use it like that.
    //0x85, HID_REPORTID_RAWHID,			 /* REPORT_ID */
    0x75, 0x08,                  /* report size = 8 bits */
    0x15, 0x00,                  /* logical minimum = 0 */
    0x26, 0xFF, 0x00,            /* logical maximum = 255 */

    0x95, 45,        /* report count TX */
    0x09, 0x01,                  /* usage */
    0x81, 0x02,                  /* Input (array) */

    0x95, 61,        /* report count RX */
    0x09, 0x02,                  /* usage */
    0x91, 0x02,                  /* Output (array) */
    0xC0                         /* end collection */ 
};

struct inputdata {
  unsigned char IRValue;         //红外传感器信号，低6bits对应6个传感器
  unsigned char Buttons;         //控制器3个功能按键，低3bits对应3个按键
  unsigned char TouchValue[32];  //32个触摸数值
  unsigned char CardStatue;      //读卡器卡片状态，0：无卡，1：aime，2：Felica
  unsigned char CardID[10];      //卡片ID，aime：10字节数据，Felica：8字节数据 + 2字节留空
};

struct rgb {
  unsigned char R;
  unsigned char G;
  unsigned char B;
};

struct usb_output_data_1 {
  unsigned char Index;       //该字节要求，为0 (即, 接收到的数据如果第1byte为0, 那么就按照usb_output_data_1的结构来解析)
  struct rgb TouchArea[20];  //对应触摸区域前20个灯
};

struct usb_output_data_2 {
  unsigned char Index;       //该字节要求，为1 (即, 接收到的数据如果第1byte为1, 那么就按照usb_output_data_2的结构来解析)
  struct rgb TouchArea[11];  //对应触摸区域后11个灯
  struct rgb LeftAir;        //左侧板灯光
  struct rgb Rightair;       //右侧板灯光
  struct rgb CardReader;     //读卡器灯光
  unsigned char Empyt[18];   //填0以填充UBS规定长度 (因为hid报告描述符里规定下行数据就是61字节所以用0填充到规定长度)
};