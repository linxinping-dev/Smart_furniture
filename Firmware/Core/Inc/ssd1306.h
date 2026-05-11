#ifndef __SSD1306_H
#define __SSD1306_H

#include "main.h"
#include "i2c.h"

// OLED I2C地址（0.96寸OLED一般是0x78，若显示异常可改为0x7A）
#define SSD1306_I2C_ADDR    0x78
// OLED分辨率
#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      64

// 16*16中文点阵：林(0)、心(1)、苹(2)
extern const unsigned char LXP[][32];

// 16*16中文点阵：朱(0)、海(1)、洋(2)
extern const unsigned char ZHY[][32];

// 8*16字符点阵：GGBond（G(0)、G(1)、B(2)、o(3)、n(4)、d(5)）
extern const uint8_t GGBondLib[][16];

// 函数声明
void SSD1306_Init(void);                  // OLED初始化
void SSD1306_Clear(void);                 // 清屏
void SSD1306_DrawString(uint8_t x, uint8_t y, char *str); // 显示字符串
void SSD1306_DrawChinese(uint8_t x, uint8_t y, const uint8_t *chinese_data);
void SSD1306_DrawChineseString(uint8_t x, uint8_t y, const uint8_t (*chinese_str)[32], uint8_t len);
void SSD1306_WriteCmd(uint8_t cmd);       // 写命令
void SSD1306_WriteData(uint8_t data);     // 写数据

// 新增8*16字符显示函数声明
void SSD1306_DrawChar8x16(uint8_t x, uint8_t y, const uint8_t *char_data);
void SSD1306_DrawChar8x16String(uint8_t x, uint8_t y, const uint8_t (*char_str)[16], uint8_t len);

#endif

