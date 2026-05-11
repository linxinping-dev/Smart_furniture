#include "ssd1306.h"
#include "string.h"

// 字模：8*16 点阵ASCII字符库（仅支持英文/数字，中文需额外字模）
const unsigned char SSD1306_Font8x16[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,// 空格
    0x00,0x00,0x7C,0x12,0x11,0x12,0x7C,0x00,0x00,0x00,0x3F,0x40,0x40,0x40,0x3F,0x00,// 0
    0x00,0x00,0x10,0x10,0x10,0x10,0x10,0x00,0x00,0x00,0x7F,0x40,0x40,0x40,0x40,0x00,// 1
    // 省略其他字符（完整库可网上下载，这里仅示例，不影响核心功能）
};


// 写OLED命令
void SSD1306_WriteCmd(uint8_t cmd)
{
    HAL_I2C_Mem_Write(&hi2c1, SSD1306_I2C_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 100);
}

// 写OLED数据
void SSD1306_WriteData(uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1, SSD1306_I2C_ADDR, 0x40, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

// OLED初始化
void SSD1306_Init(void)
{
    HAL_Delay(100); // 上电延时
    SSD1306_WriteCmd(0xAE); // 关闭显示
    SSD1306_WriteCmd(0x20); // 设置内存寻址模式
    SSD1306_WriteCmd(0x10); // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    SSD1306_WriteCmd(0xB0); // 设置页起始地址
    SSD1306_WriteCmd(0xC8); // 设置COM输出扫描方向
    SSD1306_WriteCmd(0x00); // ---set low column address
    SSD1306_WriteCmd(0x10); // ---set high column address
    SSD1306_WriteCmd(0x40); // --set start line address
    SSD1306_WriteCmd(0x81); // 设置对比度控制寄存器
    SSD1306_WriteCmd(0xFF); // 对比度值（0x00~0xFF）
    SSD1306_WriteCmd(0xA1); // 设置段重定义，0xA0左右反置，0xA1正常
    SSD1306_WriteCmd(0xA6); // 设置显示方式，0xA6正常显示，0xA7反色显示
    SSD1306_WriteCmd(0xA8); // 设置多路复用率
    SSD1306_WriteCmd(0x3F); // 1/64 duty
    SSD1306_WriteCmd(0xA4); // 0xA4,Output follows RAM content;0xA5,Output ignores RAM content
    SSD1306_WriteCmd(0xD3); // 设置显示偏移
    SSD1306_WriteCmd(0x00); // 无偏移
    SSD1306_WriteCmd(0xD5); // 设置显示时钟分频比/振荡器频率
    SSD1306_WriteCmd(0xF0); // 分频比
    SSD1306_WriteCmd(0xD9); // 设置预充电周期
    SSD1306_WriteCmd(0x22); //
    SSD1306_WriteCmd(0xDA); // 设置COM引脚硬件配置
    SSD1306_WriteCmd(0x12);
    SSD1306_WriteCmd(0xDB); // 设置VCOMH
    SSD1306_WriteCmd(0x20); // 0x20,0.77xVcc
    SSD1306_WriteCmd(0x8D); // 设置电荷泵
    SSD1306_WriteCmd(0x14); // 开启电荷泵
    SSD1306_WriteCmd(0xAF); // 开启显示
    SSD1306_Clear(); // 清屏
}

// OLED清屏
void SSD1306_Clear(void)
{
    uint8_t i,j;
    for(j=0;j<8;j++)
    {
        SSD1306_WriteCmd(0xB0+j); // 设置页地址
        SSD1306_WriteCmd(0x00);   // 设置列低地址
        SSD1306_WriteCmd(0x10);   // 设置列高地址
        for(i=0;i<128;i++)
        {
            SSD1306_WriteData(0x00); // 写入空数据
        }
    }
}

// 显示字符串（x:列(0~127)，y:页(0~7)，str:要显示的字符串）
void SSD1306_DrawString(uint8_t x, uint8_t y, char *str)
{
    uint8_t i, j;
    while(*str)
    {
        // 超出屏幕宽度则换行
        if(x > 127) {x=0; y++;}
        // 取字符字模（仅支持ASCII 32~127）
        for(i=0;i<8;i++)
        {
            SSD1306_WriteData(SSD1306_Font8x16[(*str - 32)*16 + i]);
        }
        for(i=0;i<8;i++)
        {
            SSD1306_WriteData(SSD1306_Font8x16[(*str - 32)*16 + i + 8]);
        }
        x += 8; // 每个字符占8列
        str++;
    }
}

/**
 * @brief  显示单个任意16*16点阵中文（万能版）
 * @param  x : 起始列坐标 (0~127)
 * @param  y : 起始页坐标 (0~7，16*16占y和y+1两个页)
 * @param  chinese_data : 单个中文的16*16点阵数据（必须是32字节数组：前16字节=上半部分，后16字节=下半部分）
 * @retval 无
 */
void SSD1306_DrawChinese(uint8_t x, uint8_t y, const uint8_t *chinese_data)
{
    uint8_t i, page;
    // 空指针保护
    if(chinese_data == NULL) return;
    
    // 16*16点阵占2个页（y和y+1）、16列
    for(page = 0; page < 2; page++)
    {
        // 设置页地址（y + page）
        SSD1306_WriteCmd(0xB0 + (y + page));
        // 设置列地址（x的低4位 + 高4位）
        SSD1306_WriteCmd(0x00 + (x & 0x0F));       // 列低地址
        SSD1306_WriteCmd(0x10 + ((x >> 4) & 0x0F));// 列高地址
        
        // 写入16个字节的点阵数据（每页8行，2页共16行）
        for(i = 0; i < 16; i++)
        {
            // 前16字节=page0（上半部分），后16字节=page1（下半部分）
            SSD1306_WriteData(chinese_data[i + page*16]);
        }
    }
}

/**
 * @brief  显示任意16*16点阵中文字符串（万能版）
 * @param  x : 起始列坐标 (0~127)
 * @param  y : 起始页坐标 (0~7)
 * @param  chinese_str : 中文字符串点阵数组（二维数组：每个元素是32字节的单个中文点阵）
 * @param  len : 中文字符个数
 * @retval 无
 */
void SSD1306_DrawChineseString(uint8_t x, uint8_t y, const uint8_t (*chinese_str)[32], uint8_t len)
{
    uint8_t i;
    // 空指针/长度0保护
    if(chinese_str == NULL || len == 0) return;
    
    for(i = 0; i < len; i++)
    {
        // 超出屏幕宽度（128-16=112）则换行，y+2（占2个页）
        if(x > 112) 
        {
            x = 0;
            y += 2;
            // 超出屏幕高度则退出（7-2=5）
            if(y > 5) break;
        }
        // 显示单个中文（传入当前中文的点阵数据）
        SSD1306_DrawChinese(x, y, chinese_str[i]);
        x += 16; // 每个中文占16列
    }
}

/**
 * @brief  显示单个任意8*16点阵字符（修正版，解决上下分半问题）
 * @param  x : 起始列坐标 (0~127)
 * @param  y : 起始页坐标 (0~7，8*16占y和y+1两个页)
 * @param  char_data : 单个字符的8*16点阵数据（16字节：前8=上8行，后8=下8行）
 * @retval 无
 */
void SSD1306_DrawChar8x16(uint8_t x, uint8_t y, const uint8_t *char_data)
{
    uint8_t i, page;
    // 空指针保护
    if(char_data == NULL) return;
    
    // 8*16字符占2个页（y=上8行，y+1=下8行）
    for(page = 0; page < 2; page++)
    {
        // 设置页地址（第1次= y，第2次= y+1）
        SSD1306_WriteCmd(0xB0 + (y + page));
        // 设置列地址（固定x，不偏移）
        SSD1306_WriteCmd(0x00 + (x & 0x0F));       // 列低地址
        SSD1306_WriteCmd(0x10 + ((x >> 4) & 0x0F));// 列高地址
        
        // 写入对应页的8个字节数据
        // page=0 → 前8字节（上8行），page=1 → 后8字节（下8行）
        for(i = 0; i < 8; i++)
        {
            SSD1306_WriteData(char_data[i + page*8]);
        }
    }
}

/**
 * @brief  显示任意8*16点阵字符串（修正版）
 * @param  x : 起始列坐标 (0~127)
 * @param  y : 起始页坐标 (0~7，8*16占2个页)
 * @param  char_str : 字符点阵数组（二维数组：每个元素16字节）
 * @param  len : 字符个数
 * @retval 无
 */
void SSD1306_DrawChar8x16String(uint8_t x, uint8_t y, const uint8_t (*char_str)[16], uint8_t len)
{
    uint8_t i;
    if(char_str == NULL || len == 0) return;
    
    for(i = 0; i < len; i++)
    {
        // 超出宽度换行（128-8=120），换行后y+2（因为占2个页）
        if(x > 120) 
        {
            x = 0;
            y += 2;
            if(y > 6) break; // 最大页7，y+1≤7 → y≤6
        }
        SSD1306_DrawChar8x16(x, y, char_str[i]);
        x += 8; // 每个字符占8列（横向偏移）
    }
}


// 数组：16*16点阵，林(0)、心(1)、苹(2)（和ZHY数组格式完全一致）
const unsigned char LXP[][32] = {
    // 林 (索引0)
    {0x10,0x10,0xD0,0xFF,0x90,0x10,0x00,0x10,0x10,0xD0,0xFF,0xD0,0x10,0x10,0x10,0x00,
     0x04,0x03,0x00,0xFF,0x00,0x11,0x08,0x04,0x03,0x00,0xFF,0x00,0x03,0x04,0x08,0x00},
    // 心 (索引1)
    {0x00,0x00,0x80,0x00,0x00,0xE0,0x02,0x04,0x18,0x00,0x00,0x00,0x40,0x80,0x00,0x00,
     0x10,0x0C,0x03,0x00,0x00,0x3F,0x40,0x40,0x40,0x40,0x40,0x78,0x00,0x01,0x0E,0x00},
    // 苹 (索引2)
    {0x04,0x24,0x24,0xA4,0x2F,0x24,0x24,0xE4,0x24,0x24,0x2F,0xA4,0x24,0x24,0x04,0x00,
     0x08,0x08,0x08,0x08,0x0B,0x08,0x08,0xFF,0x08,0x08,0x0A,0x09,0x08,0x08,0x08,0x00}
};

// 数组：16*16点阵，朱(0)、海(1)、洋(2)
const unsigned char ZHY[][32] = {
    // 朱 (索引0)
    {0x80,0xA0,0x90,0x8E,0x88,0x88,0x88,0xFF,0x88,0x88,0x88,0x88,0x88,0x80,0x80,0x00,
     0x20,0x20,0x10,0x08,0x04,0x02,0x01,0xFF,0x01,0x02,0x04,0x08,0x10,0x20,0x20,0x00},
    // 海 (索引1)
    {0x10,0x60,0x02,0x0C,0xC0,0x10,0x08,0xF7,0x14,0x54,0x94,0x14,0xF4,0x04,0x00,0x00,
     0x04,0x04,0x7C,0x03,0x00,0x01,0x1D,0x13,0x11,0x55,0x99,0x51,0x3F,0x11,0x01,0x00},
    // 洋 (索引2)
    {0x10,0x60,0x02,0x8C,0x00,0x10,0x91,0x96,0x90,0xF0,0x90,0x94,0x93,0x10,0x00,0x00,
     0x04,0x04,0x7E,0x01,0x00,0x04,0x04,0x04,0x04,0xFF,0x04,0x04,0x04,0x04,0x04,0x00}
};

// GGBond的8*16点阵库（每个字符16字节，前8=上半部分，后8=下半部分）
// G(0)、G(1)、B(2)、o(3)、n(4)、d(5)
const uint8_t GGBondLib[][16] = {
    // G(0)
    {0xC0,0x30,0x08,0x08,0x08,0x38,0x00,0x00, 0x07,0x18,0x20,0x20,0x22,0x1E,0x02,0x00},
    // G(1)
    {0xC0,0x30,0x08,0x08,0x08,0x38,0x00,0x00, 0x07,0x18,0x20,0x20,0x22,0x1E,0x02,0x00},
    // B(2)
    {0x08,0xF8,0x88,0x88,0x88,0x70,0x00,0x00, 0x20,0x3F,0x20,0x20,0x20,0x11,0x0E,0x00},
    // o(3)
    {0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00, 0x00,0x1F,0x20,0x20,0x20,0x20,0x1F,0x00},
    // n(4)
    {0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x00, 0x20,0x3F,0x21,0x00,0x00,0x20,0x3F,0x20},
    // d(5)
    {0x00,0x00,0x80,0x80,0x80,0x90,0xF0,0x00, 0x00,0x1F,0x20,0x20,0x20,0x10,0x3F,0x20}
};
