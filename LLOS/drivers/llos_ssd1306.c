/*
 * 作者: LittleLeaf All rights reserved
 * 版本: V1.0.0T
 * 修订日期: 2024 09 12
 * 修订日志:
 * N/A
 */
#include <llos_ssd1306.h>
#include <llos_ssd1306_font.h>

static ll_I8080_WriteByteCB_t ll_I8080_WriteByteCB;
static ll_I8080_DMAWriteCB_t ll_I8080_DMACB;

static uint16_t width, height;
static enum ll_SSD1306_screenType_t ll_type;
static enum ll_SSD1306_screen_xOffset_t ll_xOffset;

void LLOS_SSD1306_Init(ll_I8080_WriteByteCB_t I8080_WriteByteCB, ll_I8080_DMAWriteCB_t I8080_DMAWriteCB, ll_SSD1306_screenConfig_t *screenConfig)
{
	ll_I8080_WriteByteCB = I8080_WriteByteCB;
	ll_I8080_DMACB = I8080_DMAWriteCB;
	
	if(ll_I8080_WriteByteCB == NULL || screenConfig == NULL)return;

	ll_type = screenConfig->type;
	ll_xOffset = screenConfig->xOffset;

	ll_I8080_WriteByteCB(0xAE, ll_SSD1306_cmd_Cmd);	/* 关闭OLED -- turn off oled panel */

	if(screenConfig->isMirrot)	/* 设置段重映射 -- Set SEG / Column Mapping     0xA0左右反置（复位值） 0xA1正常（重映射值） */
		ll_I8080_WriteByteCB(0xA0, ll_SSD1306_cmd_Cmd);
	else
		ll_I8080_WriteByteCB(0xA1, ll_SSD1306_cmd_Cmd);

	if(screenConfig->isInvert)	/* 设置行输出扫描方向 -- Set COM / Row Scan Direction   0xc0上下反置（复位值） 0xC8正常（重映射值） */
		ll_I8080_WriteByteCB(0xC0, ll_SSD1306_cmd_Cmd);
	else
		ll_I8080_WriteByteCB(0xC8, ll_SSD1306_cmd_Cmd);

	if(screenConfig->isInvertPhase)	/* 设置显示方式(正常/反显) -- set normal display (0xA6 / 0xA7) */
		ll_I8080_WriteByteCB(0xA7, ll_SSD1306_cmd_Cmd);
	else
		ll_I8080_WriteByteCB(0xA6, ll_SSD1306_cmd_Cmd);

	ll_I8080_WriteByteCB(0x81, ll_SSD1306_cmd_Cmd);	/* 设置对比度 -- set contrast control register (0x00~0x100) */
	ll_I8080_WriteByteCB(screenConfig->brightness, ll_SSD1306_cmd_Cmd);    /* \ Set SEG Output Current Brightness */

	ll_I8080_WriteByteCB(0xD5, ll_SSD1306_cmd_Cmd);	/* 设置显示时钟分频因子/振荡器频率 -- set display clock divide ratio/oscillator frequency */
	ll_I8080_WriteByteCB(0x80, ll_SSD1306_cmd_Cmd);	/* \ set divide ratio, Set Clock as 100 Frames/Sec */

	ll_I8080_WriteByteCB(0xD9, ll_SSD1306_cmd_Cmd);	/* 设置预充电期间的持续时间 -- set pre-charge period */
	ll_I8080_WriteByteCB(0xF1, ll_SSD1306_cmd_Cmd);	/* \ Set Pre-Charge as 15 Clocks & Discharge as 1 Clock */

	ll_I8080_WriteByteCB(0xDB, ll_SSD1306_cmd_Cmd);	/* 调整VCOMH调节器的输出 -- set vcomh (0x00 / 0x20 / 0x30) */
	ll_I8080_WriteByteCB(0x20, ll_SSD1306_cmd_Cmd);	/* \ Set VCOM Deselect Level */

	if(screenConfig->type == ll_SSD1306_screenType_128x64)
	{
		width = 128;
		height = 64;

		ll_I8080_WriteByteCB(0xA8, ll_SSD1306_cmd_Cmd);	/* 设置多路传输比率 -- set multiplex ratio (16 to 63) */
		ll_I8080_WriteByteCB(0x3F, ll_SSD1306_cmd_Cmd);	/* \ 1 / 64 duty */

		ll_I8080_WriteByteCB(0xDA, ll_SSD1306_cmd_Cmd);	/* 设置列引脚硬件配置 -- set com pins hardware configuration */
		ll_I8080_WriteByteCB(0x12, ll_SSD1306_cmd_Cmd);	/* \ Sequential COM pin configuration，Enable COM Left/Right remap */
	}
	else if(screenConfig->type == ll_SSD1306_screenType_128x32)
	{
		width = 128;
		height = 32;

		ll_I8080_WriteByteCB(0xA8,ll_SSD1306_cmd_Cmd);	/* 设置多路传输比率 -- set multiplex ratio (16 to 63) */
		ll_I8080_WriteByteCB(0x1F,ll_SSD1306_cmd_Cmd);	/* \ 1 / 32 duty */

		ll_I8080_WriteByteCB(0xDA,ll_SSD1306_cmd_Cmd);	/* 设置列引脚硬件配置 -- set com pins hardware configuration */
		ll_I8080_WriteByteCB(0x02,ll_SSD1306_cmd_Cmd);	/* \ Sequential COM pin configuration，Disable COM Left/Right remap */
	}

	ll_I8080_WriteByteCB(0x40, ll_SSD1306_cmd_Cmd);	/* 设置设置屏幕（GDDRAM）起始行 -- Set Display Start Line (0x40~0x7F) */

	ll_I8080_WriteByteCB(0xD3, ll_SSD1306_cmd_Cmd);	/* 设置显示偏移 -- set display offset (0x00~0x3F) */
	ll_I8080_WriteByteCB(0x00, ll_SSD1306_cmd_Cmd);	/* \ not offset */

	ll_I8080_WriteByteCB(0x8D, ll_SSD1306_cmd_Cmd);	/* 电荷泵设置 -- set Charge Pump enable / disable (0x14 / 0x10) */
	ll_I8080_WriteByteCB(0x14, ll_SSD1306_cmd_Cmd);	/* \ Enable charge pump during display on */

	ll_I8080_WriteByteCB(0xA4, ll_SSD1306_cmd_Cmd);	/* 全局显示开启(黑屏/亮屏) -- Entire Display On (0xA4 / 0xA5) */

	LLOS_SSD1306_Fill(0x00);				/* 清屏 */
	ll_I8080_WriteByteCB(0xAF, ll_SSD1306_cmd_Cmd);	/* 打开显示 */
}

void LLOS_SSD1306_ScreenEN(ll_newState_t newState)
{
	if(ll_I8080_WriteByteCB == NULL)return;

	ll_I8080_WriteByteCB(0x8D, ll_SSD1306_cmd_Cmd);  	/* 升压使能 */
	if(newState)
	{
		ll_I8080_WriteByteCB(0x14, ll_SSD1306_cmd_Cmd);  /* 启用升压使能 */
		ll_I8080_WriteByteCB(0xAF, ll_SSD1306_cmd_Cmd);  /* 打开显示 */
	}
	else
	{
		ll_I8080_WriteByteCB(0x10, ll_SSD1306_cmd_Cmd);  /* 禁用升压使能 */
		ll_I8080_WriteByteCB(0xAE, ll_SSD1306_cmd_Cmd);  /* 关闭显示 */
	}
}

void LLOS_SSD1306_Fill(uint16_t color)
{
	uint16_t i, j;

	if(ll_I8080_WriteByteCB == NULL)return;

    for(i = 0; i < height >> 3; i++)
    {
        ll_I8080_WriteByteCB(0xB0 + i, ll_SSD1306_cmd_Cmd);
        ll_I8080_WriteByteCB(0x00 + ll_xOffset, ll_SSD1306_cmd_Cmd);
        ll_I8080_WriteByteCB(0x10, ll_SSD1306_cmd_Cmd);

        for(j = 0; j < width; j++)
            ll_I8080_WriteByteCB(color, ll_SSD1306_cmd_Data);
    }
}

void LLOS_SSD1306_SetPos(uint16_t x, uint16_t y)
{
	if(ll_I8080_WriteByteCB == NULL)return;

	ll_I8080_WriteByteCB(0xB0 + y, ll_SSD1306_cmd_Cmd);
	ll_I8080_WriteByteCB((x + ll_xOffset & 0x0F), ll_SSD1306_cmd_Cmd);
	ll_I8080_WriteByteCB(((x + ll_xOffset & 0xF0) >> 4) | 0x10, ll_SSD1306_cmd_Cmd);
}

void LLOS_SSD1306_DrawPic(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *pic)
{
	uint16_t i, j, k = 0;

	if(ll_I8080_WriteByteCB == NULL)return;
	if(x + w > width || y + h > height)return;

	if(ll_I8080_DMACB == NULL)
	{
		for(j = y; j < y + h; j++)
		{
			i = x;
			LLOS_SSD1306_SetPos(i, j);
			for( ; i < x + w; i++)
				ll_I8080_WriteByteCB(pic[k++], ll_SSD1306_cmd_Data);
		}
	}
	else
	{
		for (i = 0; i < 8; i++)
		{
			LLOS_SSD1306_SetPos(0, i);
			ll_I8080_DMACB(pic + 128 * i, 128);
		}
	}
}

void LLOS_SSD1306_GetSize(uint16_t *w, uint16_t *h)
{
	*w = width;
	*h = height;
}

static void LLOS_SSD1306_ShowChar(uint16_t x, uint16_t y, const char chr, enum ll_SSD1306_sizeFont_t sizeFont)
{
	uint8_t i;
	char ch;
	
	ch = chr;
	ch -= ' ';
	
	if(sizeFont == ll_SSD1306_sizeFont_6x8)
	{
		LLOS_SSD1306_SetPos(x, y);
		for(i = 0; i < 6; i++)
			ll_I8080_WriteByteCB(ll_SSD1306_font_ASCII6x8[ch][i], ll_SSD1306_cmd_Data);
	}
	else
    {
        LLOS_SSD1306_SetPos(x, y); /* 填充第一页 */
        for(i = 0; i < 8; i++)
            ll_I8080_WriteByteCB(ll_SSD1306_font_ASCII8x16[ch][i], ll_SSD1306_cmd_Data); /* 写入数据 */
        LLOS_SSD1306_SetPos(x, y + 1); /* 填充第二页 */
        for(i = 0; i < 8; i++)
            ll_I8080_WriteByteCB(ll_SSD1306_font_ASCII8x16[ch][i + 8], ll_SSD1306_cmd_Data);
    }
}
static void LLOS_SSD1306_ShowStr(uint16_t x, uint16_t y, const char *str, enum ll_SSD1306_sizeFont_t sizeFont)
{
	while(*str != '\0')
	{
		if(*str == '\r' && *(str + 1) == '\n')
		{
			if(sizeFont == ll_SSD1306_sizeFont_6x8)++y; /* 换行 */
			else if(sizeFont == ll_SSD1306_sizeFont_8x16)y += 2; /* 换行 */
			x = 0;
			str += 2;
		}
		if((x > (width - 6) && sizeFont == ll_SSD1306_sizeFont_6x8)||(x > (width - 8) && sizeFont == ll_SSD1306_sizeFont_8x16)) /* 自动换行 */
		{
			x = 0;
			if(sizeFont == ll_SSD1306_sizeFont_6x8)++y; /* 换行 */
			else if(sizeFont == ll_SSD1306_sizeFont_8x16)y += 2; /* 换行 */
		}
		
		LLOS_SSD1306_ShowChar(x, y, *str, sizeFont);
		
		if(sizeFont == ll_SSD1306_sizeFont_6x8)x += 6;
		else if(sizeFont == ll_SSD1306_sizeFont_8x16)x += 8;
			
		str++; /* 字符串指针移动 */
	}
}

void LLOS_SSD1306_ShowNumFormat(uint16_t x, uint16_t y, uint32_t num, const char *str, enum ll_SSD1306_sizeFont_t sizeFont)
{
    char s[100];
    sprintf(s, str, num);//整形转十六进制（字符串格式化）
    LLOS_SSD1306_ShowStr(x, y, s, sizeFont);
}
void LLOS_SSD1306_ShowString(uint16_t x, uint16_t y, const char *str)
{
#ifdef ll_SSD1306_font_CN1616
	uint8_t chinese_num = sizeof(ll_SSD1306_font_CN16x16) / sizeof(ll_SSD1306_font_CN_t); /* 计算字库字符数 */
	uint8_t index,i;
#endif
	while(*str != '\0')
	{
		if(*str == '\r' && *(str + 1) == '\n')
		{
			y += 2; /* 换行 */
			x = 0;
			str += 2 ;
		}
#ifdef ll_SSD1306_font_CN1616
		if((*str) > 127) /* 如果是中文 */
		{
			if(x > width - 16) /* 自动换行 */
			{
				x = 0;
				y += 2; /* 换行 */
			}
			for(index = 0; index < chinese_num; index++) /* 循环查找 */
			{
				if(ll_SSD1306_font_CN16x16[index].CN_index[0] == *str && ll_SSD1306_font_CN16x16[index].CN_index[1] == *(str + 1) && ll_SSD1306_font_CN16x16[index].CN_index[2] == *(str + 2))
				{			
					LLOS_SSD1306_SetPos(x, y);
					/* 从字库中查找字模填充第一页 */		
					for(i = 0; i < 16; i++)
						ll_I8080_WriteByteCB(ll_SSD1306_font_CN16x16[index].CN_library[i], ll_SSD1306_cmd_Data);
					
					LLOS_SSD1306_SetPos(x, y + 1); /* 一个汉字占两页，坐标跳转下一页 */
					
					/* 从字库中查找字模填充第二页 */		
					for(i = 0; i < 16; i++)
						ll_I8080_WriteByteCB(ll_SSD1306_font_CN16x16[index].CN_library[i + 16], ll_SSD1306_cmd_Data);
					
					x += 16; /* x指针往后移16位，为显示下一个汉字做准备 */
					if(x > width - 16) /* 自动换行 */
					{
						x = 0;
						y += 2;
					}	
				}
			}
			str += 3;	
			index = 0;
		}
#endif
		else 
		{
			if(x > width - 8) /* 自动换行 */
			{
				x = 0;
				y += 2; /* 换行 */
			}
			LLOS_SSD1306_ShowChar(x, y, *str, ll_SSD1306_sizeFont_8x16);
			x += 8;
			str++;
		}
	}
}
uint8_t g_aLcdBuf[128][64 >> 3];
void LLOS_SSD1306_DrawDot(uint16_t x, uint16_t y)
{
    uint8_t PageNumber = y >> 3;
    LLOS_SSD1306_SetPos(x, PageNumber);
    g_aLcdBuf[x][PageNumber] |= 1 << (y & 7);

    ll_I8080_WriteByteCB(g_aLcdBuf[x][PageNumber], ll_SSD1306_cmd_Data);
}
void LLOS_SSD1306_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint16_t Xpoint, Ypoint;
    int16_t dx, dy;
    int16_t XAddway,YAddway;
    int16_t Esp;
    int8_t dottedLen;
    Xpoint = x1;
    Ypoint = y1;
	
    dx = (int16_t)x2 - (int16_t)x1 >= 0 ? x2 - x1 : x1 - x2;
    dy = (int16_t)y2 - (int16_t)y1 <= 0 ? y2 - y1 : y1 - y2;

    XAddway = x1 < x2 ? 1 : -1;
    YAddway = y1 < y2 ? 1 : -1;

    Esp = dx + dy;
    dottedLen = 0;

    for(;;)
    {
        dottedLen++;
        LLOS_SSD1306_DrawDot(Xpoint, Ypoint);
        if((Esp << 1) >= dy)
        {
            if(Xpoint == x2)break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if((Esp << 1) <= dx)
        {
            if(Ypoint == y2)break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

void LLOS_SSD1306_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, ll_newState_t isFill)
{
    uint16_t i;
	
    if(isFill)
    {
		for(i = y; i < y + h; i++)LLOS_SSD1306_DrawLine(x, i, x + w, i);
    }
    else
    {
        LLOS_SSD1306_DrawLine(x, y, x + w, y);
        LLOS_SSD1306_DrawLine(x, y, x, y + h);
        LLOS_SSD1306_DrawLine(x + w, y + h, x + w, y);
        LLOS_SSD1306_DrawLine(x + w, y + h, x, y + h);
    }
}
void LLOS_SSD1306_DrawRoundedRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, ll_newState_t isFill)
{
	uint16_t i;
	if(r > w || r > h)return;

    if(isFill)
    {
    	i = y;
    	for( ; i < y + r; i++)
    	{
    		LLOS_SSD1306_DrawLine(x + (r - (i - y)), i, x + w - 1 - (r - (i - y)), i);
		}
    	for( ; i < y + h - r; i++)
    	{
    		LLOS_SSD1306_DrawLine(x, i, x + w - 1, i);
		}
    	for( ; i < y + h; i++)
    	{
    		LLOS_SSD1306_DrawLine(x + r - (y + h - i), i, x + w - 1 - r + (y + h - i), i);
		}
    }
    else
    {
        LLOS_SSD1306_DrawLine(x + r, y, x + w - 1 - r, y);
        LLOS_SSD1306_DrawLine(x + w - 1, y + r, x + w - 1, y + h - 1 - r);
        LLOS_SSD1306_DrawLine(x + w - 1 - r, y + h - 1, x + r, y + h - 1);
        LLOS_SSD1306_DrawLine(x, y + h - 1 - r, x, y + r);

        LLOS_SSD1306_DrawLine(x + r, y, x, y + r);
        LLOS_SSD1306_DrawLine(x + w - 1 - r, y, x + w - 1, y + r);
        LLOS_SSD1306_DrawLine(x + w - 1, y + h - 1 - r, x + w - 1 - r, y + h - 1);
        LLOS_SSD1306_DrawLine(x, y + h - 1 - r, x + r, y + h - 1);
	}
}
void LLOS_SSD1306_DrawCircle(uint16_t x, uint16_t y, uint16_t r, ll_newState_t isFill)
{
	int16_t sCurrentX, sCurrentY;
	int16_t sError;

     sCurrentX = 0;
     sCurrentY = r;
     sError = 3 - (r << 1);

     while(sCurrentX <= sCurrentY)
     {
         int16_t sCountY = 0;
         if(isFill)
         {
             for(sCountY = sCurrentX; sCountY <= sCurrentY; sCountY++)
             {
                 LLOS_SSD1306_DrawDot(x + sCurrentX, y + sCountY);
                 LLOS_SSD1306_DrawDot(x - sCurrentX, y + sCountY);
                 LLOS_SSD1306_DrawDot(x - sCountY,   y + sCurrentX);
                 LLOS_SSD1306_DrawDot(x - sCountY,   y - sCurrentX);
                 LLOS_SSD1306_DrawDot(x - sCurrentX, y - sCountY);
                 LLOS_SSD1306_DrawDot(x + sCurrentX, y - sCountY);
                 LLOS_SSD1306_DrawDot(x + sCountY,   y - sCurrentX);
                 LLOS_SSD1306_DrawDot(x + sCountY,   y + sCurrentX);
             }
		}
		else
		{
			LLOS_SSD1306_DrawDot(x + sCurrentX, y + sCurrentY);
			LLOS_SSD1306_DrawDot(x - sCurrentX, y + sCurrentY);
			LLOS_SSD1306_DrawDot(x - sCurrentY, y + sCurrentX);
			LLOS_SSD1306_DrawDot(x - sCurrentY, y - sCurrentX);
			LLOS_SSD1306_DrawDot(x - sCurrentX, y - sCurrentY);
			LLOS_SSD1306_DrawDot(x + sCurrentX, y - sCurrentY);
			LLOS_SSD1306_DrawDot(x + sCurrentY, y - sCurrentX);
			LLOS_SSD1306_DrawDot(x + sCurrentY, y + sCurrentX);
		}
		sCurrentX++;
		if(sError < 0)sError += (4 * sCurrentX + 6);
		else
		{
			sError += (10 + 4 * (sCurrentX - sCurrentY));
			sCurrentY--;
		}
     }
}
/* 扫描线活动表 */
struct edge_t
{
	uint16_t startY;
    float startX, endX;
    float slopeInv; /* 斜率的倒数 */
};
/* 交换两个整数值 */
static void SwapInt(uint16_t *a, uint16_t *b)
{
	uint16_t temp = *a;
    *a = *b;
    *b = temp;
}
/* 初始化边的信息 */
static void InitEdge(struct edge_t *e, uint16_t yStart, float xStart, float xEnd, float slopeInverse)
{
    e->startY = yStart;
    e->startX = xStart;
    e->endX = xEnd;
    e->slopeInv = slopeInverse;
}

void LLOS_SSD1306_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, ll_newState_t isFill)
{
	if(isFill)
	{
		/* 扫描线算法 */
	    /* 对三个点按y坐标进行排序 */
	    if(y1 > y2)
	    {
	    	SwapInt(&y1, &y2);
	    	SwapInt(&x1, &x2);
	    }
	    if(y1 > y3)
	    {
	    	SwapInt(&y1, &y3);
	    	SwapInt(&x1, &x3);
	    }
	    if(y2 > y3)
	    {
	    	SwapInt(&y2, &y3);
	    	SwapInt(&x2, &x3);
	    }

	    /* 初始化三条边的信息 */
	    struct edge_t edges[3];
	    InitEdge(&edges[0], y1, x1, x2, (float)(x2 - x1) / (y2 - y1));
	    InitEdge(&edges[1], y2, x2, x3, (float)(x3 - x2) / (y3 - y2));
	    InitEdge(&edges[2], y1, x1, x3, (float)(x3 - x1) / (y3 - y1));

	    /* 逐行扫描，填充三角形 */
	    for(uint16_t y = y1; y <= y3; y++)
	    {
	        /* 找到当前行对应的边 */
	    	int16_t leftX = -1, rightX = -1;
	        for(uint16_t i = 0; i < 3; i++)
	        {
	            if(y >= edges[i].startY)
	            {
	                if(leftX == -1 || edges[i].startX < edges[leftX].startX)leftX = i;
	                if(rightX == -1 || edges[i].startX > edges[rightX].startX)rightX = i;
	            }
	        }

	        /* 画水平线填充三角形 */
	        if (leftX != -1 && rightX != -1)
	        	LLOS_SSD1306_DrawLine((uint16_t)edges[leftX].startX, y, (uint16_t)edges[rightX].startX, y);

	        /* 更新边的起始x坐标 */
	        edges[leftX].startX += edges[leftX].slopeInv;
	        edges[rightX].startX += edges[rightX].slopeInv;
	    }
	}
	else
	{
		LLOS_SSD1306_DrawLine(x1, y1, x2, y2);
		LLOS_SSD1306_DrawLine(x1, y1, x3, y3);
		LLOS_SSD1306_DrawLine(x2, y2, x3, y3);
	}
}
