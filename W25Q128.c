#include <stdio.h>
#include <stdint.h>
#include <types.h>

#define W25QXX_CS PBout //片选信号
#define W25Q128WriteEnable 0x06
#define W25Q128ReadData 0x03
#define W25Q128WriteData 0x02
#define W25Q128ReadStatusRegister 0x05

u8 mySPI1_ReadWriteByte(u8 Txdata)
{
	//发送缓冲区为空
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) {};//发送缓冲区为空跳出循环
	SPI_I2S_SendData(SPI1, Txdata);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {};// SPI->DR数据寄存器不为空跳出循环
	
	return SPI_I2S_ReceiveData(SPI1);
}
//W25Q128写使能
void W25Q128_Write_Enable(void)
{
	W25QXX_CS = 0;
	mySPI1_ReadWriteByte(W25Q128WriteEnable);//发送写使能
	W25QXX_CS = 1;
}

//读取W25QXX的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
u8 W25Q128_ReadSR()
{
    u8 temp;
	W25QXX_CS = 0;
	mySPI1_ReadWriteByte(W25Q128ReadStatusRegister);//读状态寄存器
    temp = mySPI1_ReadWriteByte(0xff);
	W25QXX_CS = 1; 
    return temp;   
}
void W25Q128_Wait_Busy()
{
    u8 temp = 0;
/*     while (1)
    {
        temp = mySPI1_ReadWriteByte(W25Q128ReadStatusRegister);
        if (temp & 0x01) //第0bit为零跳出循环
            break;
    } */
    while((W25Q128_ReadSR() & 0x01) == 0x01);//第0bit为零跳出循环
}

//W25Q128写一页数据,page 为Flash的最小单位
void W25Q128_Write_Page(u8* pBuffer, u32 addr, u16 numberToWrite)
{
    int i;
    W25Q128_Write_Enable();//写使能
    W25QXX_CS = 0;//片选
    mySPI1_ReadWriteByte(W25Q128WriteData);//写命令
    mySPI1_ReadWriteByte((u8)addr >> 16);//地址
    mySPI1_ReadWriteByte((u8)addr >> 8);
    mySPI1_ReadWriteByte((u8)addr);
    for (i = 0; i < numberToWrite; i++)
        mySPI1_ReadWriteByte(pBuffer[i]);
    W25Q128_Wait_Busy();
    W25QXX_CS = 1;//取消片选    
}

//扇区sector为FLASH擦除的最小单位
void W25Q128_Write_Sector(u8* pBuffer, u32 addr, u16 numberToWrite)
{
    
}

void W25Q128_Read(u8* pBuffer, u32 addr, u16 size)
{
	int i;
	W25QXX_CS = 0;//片选W25Q128
	mySPI1_ReadWriteByte(0x03);//读命令
	mySPI1_ReadWriteByte(addr >> 16);//地址
	mySPI1_ReadWriteByte(addr >> 8);
	mySPI1_ReadWriteByte(addr);
	
	for (i = 0; i < size; i++)
	{
		pBuffer[i] = mySPI1_ReadWriteByte(0xff);//任意数据
	}
	W25QXX_CS = 1;
}