#include <Wire.h>
#include "data.h"

//Touch
#define CLAMP(val, lo, hi) (val < lo ? lo : (val > hi ? hi : val))
#define touch_addr0 0x08 //第一个（左边）芯片的I2C地址
#define touch_addr1 0x09

struct touchval touchval0;
struct touchval touchval1;
uint8_t touchdata[32];

const uint8_t sensorPinMap0[16] = {0,1,2,3,4,5,6,7,9,8,11,10,13,12,15,14}; //第一个触摸芯片引脚到实际区域的映射数组
const uint8_t sensorPinMap1[16] = {16,17,18,19,20,21,22,23,25,24,27,26,29,28,31,30}; //第二个触摸芯片引脚到实际区域的映射数组

void touchInit()
{
  Wire1.setSDA(18);
  Wire1.setSCL(19);
  Wire1.setClock(1000000);
  Wire1.begin();
}

void updateSensorValue()
{
  uint8_t buffer[64];
  //获取第一个PSoC的数据
  Wire1.requestFrom(touch_addr0, 64);
  uint8_t c = 0;
  while (Wire1.available()) {
    buffer[c] = Wire1.read();
    c ++;
  }
  memcpy(&touchval0, buffer, 64);
  //获取第二个PSoC的数据
  Wire1.requestFrom(touch_addr1, 64);
  c = 0;
  while (Wire1.available()) {
    buffer[c] = Wire1.read();
    c ++;
  }
  memcpy(&touchval1, buffer, 64);
}

void updateTouchData()
{
  int temp;
  for (int i=0; i<16; i++)
  {
    temp = touchval0.raw[i] - touchval0.baseline[i];
    temp = temp / 3 - 15; //这部分的具体处理要随实际情况而定
    touchdata[sensorPinMap0[i]] = CLAMP(temp, 0, 255);
  }
  for (int i=0; i<16; i++)
  {
    temp = touchval1.raw[i] - touchval1.baseline[i];
    temp = temp / 3 - 15;
    touchdata[sensorPinMap1[i]] = CLAMP(temp, 0, 255);
  }
}

uint8_t getTouchData(uint8_t sensorNum)
{
  return touchdata[sensorNum];
}

//Air
#define IR_TX0 0
#define IR_TX1 1
#define IR_TX2 2
#define IR_TX3 3
#define IR_TX4 4
#define IR_TX5 5

#define IR_RX_ADD0 20
#define IR_RX_ADD1 21
#define IR_RX_ADD2 22

#define IR_ADC 26

#define SAMPLES_NUM 200 //校准阈值时采样次数

float pressSensitivity=0.3; //按下灵敏度设定
float releaseSensitivity=0.2; //松开灵敏度设定，比pressSensitivity稍低，防止在阈值附近时结果输出的抖动
uint8_t airVal = 0;
int readings[6] = {0}; //传感器读数
int pressThreshold[6] = {0}; //按下阈值
int releaseThreshold[6] = {0}; //松开阈值
int maxOnReadings[6] = {0}; //LED亮起时ADC读数的最大值
int minOffReadings[6] = {0}; //LED灭时ADC读数的最小值（借鉴自Lingnithm项目）
int mapg[6] = {1,0,3,2,5,4}; //传感器物理位置到yubideck报告比特位置的映射

void airInit()
{
  pinMode(IR_TX0,OUTPUT); //初始化TX端输出引脚
  pinMode(IR_TX1,OUTPUT);
  pinMode(IR_TX2,OUTPUT);
  pinMode(IR_TX3,OUTPUT);
  pinMode(IR_TX4,OUTPUT);
  pinMode(IR_TX5,OUTPUT);

  pinMode(IR_RX_ADD0,OUTPUT); //初始化RX端地址输出引脚
  pinMode(IR_RX_ADD1,OUTPUT);
  pinMode(IR_RX_ADD2,OUTPUT);

  pinMode(IR_ADC,INPUT); //初始化RX端ADC输入引脚
  analogReadResolution(12); //将ADC分辨率设为12bits
}

void airThresholdInit()
{
  gpio_clr_mask(7<<20); // 7即0b111，左移到以gpio20为起始，使用clr_mask函数将20，21，22都设置为低电平
  //LED关闭时采集环境数值
  for (int i = 0; i < SAMPLES_NUM; i++)
  {
    for (int j = 0; j < 6; j++)
    {
      gpio_set_mask(j<<20); // 将对应引脚置高来在ADD引脚上输出地址值
      delayMicroseconds(20); // 保险起见的延迟（
      readings[j] = analogRead(IR_ADC);
      if (readings[j] < minOffReadings[j] || minOffReadings[j] == 0)
      {
        minOffReadings[j] = readings[j];
      }
      gpio_clr_mask(j<<20); // 将对应引脚置低电平
    }
  }

  //LED on
  for (int i = 0; i < SAMPLES_NUM; i++)
  {
    for (int j = 0; j < 6; j++)
    {
      digitalWrite(j,HIGH);
      gpio_set_mask(j<<20);
      delayMicroseconds(400); //用的发射管比较差，要400微秒才能保证当前LED完全开启和上一个完全关断（所以将Air更新放到core1去了，防止阻塞主进程），可根据实际情况调节
      readings[j] = analogRead(IR_ADC);
      if (readings[j] > maxOnReadings[j])
      {
        maxOnReadings[j] = readings[j];
      }
      digitalWrite(j,LOW);
      gpio_clr_mask(j<<20);
    }
  }

  //Set up threshold value
  float temp;
  for (int i = 0; i < 6; i++)
  {
    //Press threshold
    temp = (minOffReadings[i] - maxOnReadings[i]) * pressSensitivity + maxOnReadings[i];
    pressThreshold[i] = temp;
    temp = (minOffReadings[i] - maxOnReadings[i]) * releaseSensitivity + maxOnReadings[i];
    releaseThreshold[i] = temp;
  }
}

void updateAirVal()
{
  gpio_clr_mask(7<<20);
  for (int i = 0; i < 6; i++)
  {
    digitalWrite(i, HIGH);
    gpio_set_mask(i<<20);
    delayMicroseconds(400);
    readings[i] = analogRead(IR_ADC);
    if (readings[i] >= pressThreshold[i])
    {
      airVal |= 1<<mapg[i];
    }
    if (readings[i] <= releaseThreshold[i])
    {
      airVal &= ~(1<<mapg[i]);
    }
    digitalWrite(i, LOW);
    gpio_clr_mask(i<<20);
  }
}

uint8_t getAirVal()
{
  return airVal;
}

//Button
#define service 11
#define coin 12
#define test 13

uint8_t buttonVal = 0;

void buttonInit()
{
  pinMode(service,INPUT_PULLUP);
  pinMode(coin,INPUT_PULLUP);
  pinMode(test,INPUT_PULLUP);
}

void updateButton()
{
  buttonVal = 0;
  if (digitalRead(service) == LOW)
  {
    buttonVal += 0x02;
  }
  if (digitalRead(coin) == LOW)
  {
    buttonVal += 0x04;
  }
  if (digitalRead(test) == LOW)
  {
    buttonVal += 0x01;
  }
}

uint8_t getButtonVal()
{
  return buttonVal;
}