#include <stdio.h>
#include <stdint.h>
#include <types.h>

#define W25QXX_CS PBout //片选信号
#define W25Q128WriteEnable 0x06
#define W25Q128ReadData 0x03
#define W25Q128WriteData 0x02
#define W25Q128ReadStatusRegister 0x05
#define W25Q128EraseSector 0x20

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
    mySPI1_ReadWriteByte((u8)(addr >> 16));//地址
    mySPI1_ReadWriteByte((u8)(addr >> 8));
    mySPI1_ReadWriteByte((u8)addr);
    for (i = 0; i < numberToWrite; i++)
        mySPI1_ReadWriteByte(pBuffer[i]);
    W25QXX_CS = 1;//取消片选    
	W25QXX_Wait_Busy();					   //等待写入结束
}

//扇区sector为FLASH擦除的最小单位 numberToWrite max 4096
void W25Q128_Write_Sector(u8* pBuffer, u32 addr, u16 numberToWrite)
{
    //int pagePos = addr/256;
	//int pageOffset = addr%256;
	int pageRemain = 256 - addr%256;
	if (numberToWrite < pageRemain)
		pageRemain = numberToWrite;
	while (1)
	{
		W25Q128_Write_Page(pBuffer, addr, pageRemain);//写满一页
		if (pageRemain == numberToWrite) break;
		else
		{
			pBuffer += pageRemain;
			addr += pageRemain;
			numberToWrite -=pageRemain;
			if (numberToWrite > 256)
				pageRemain = 256;
			else
				pageRemain = numberToWrite;
		}

	}
}

//擦除一个扇区，时间为多长？
void W25Q128_Erase_Sector(u32 addr)
{
	W25Q128_Write_Enable();//写使能
	W25Q128_Wait_Busy();
	W25QXX_CS = 0;
	mySPI1_ReadWriteByte(W25Q128EraseSector);
    mySPI1_ReadWriteByte((u8)addr >> 16);//地址
    mySPI1_ReadWriteByte((u8)addr >> 8);
    mySPI1_ReadWriteByte((u8)addr);
	W25QXX_CS = 1;
	W25Q128_Wait_Busy();//等待擦除扇区

}

u8 writeBuf[4096];
//写Flash W25Q128 max 65536bytes 1 block
void W25Q128_Write_Block(u8* pBuffer, u32 addr, u16 numberToWrite)
{
	int i;
	int secPos = addr / 4096;
	int secOffset = addr % 4096;
	int secRemain = 4096 - secOffset;
	if (numberToWrite < secRemain)
		secRemain = numberToWrite;
	while (1)
	{
		//需要判断整个扇区是否被写过
		//先把扇区数据读入buf
		W25Q128_Read(writeBuf, secPos*4096, 4096);
		//判断
		for  (i = 0; i < secRemain; i++)
		{
			if (writeBuf[secOffset + i] != 0xFF)
				break;//Flash中有保存其他数据，需要擦除
		}
		if (i < secRemain)//跳出了循环
		{
			//擦除扇区
			W25Q128_Erase_Sector(secPos*4096);
			//数据填入buf
			for  (i = 0; i < secRemain; i++)
			{
				writeBuf[secOffset + i] = pBuffer[i];			
			}
			//填充扇区
			W25Q128_Write_Sector(writeBuf, secPos*4096, 4096);

		}
		else
		{
			W25Q128_Write_Sector(pBuffer, addr, secRemain);
		}
		if (numberToWrite == secRemain)
			break;
		else
		{
			//偏移计算
			secPos += 1;//扇区偏移
			secOffset = 0;
			numberToWrite -= secRemain; //写的字节数递减
			pBuffer +=secRemain; 	//指针偏移
			addr += secRemain;		//地址偏移
			if (numberToWrite < 4096)//下一扇区可以写完
				secRemain = numberToWrite;
			else
				secRemain = 4096;
		}

	}
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



//W25Q128写一页数据,page 为Flash的最小单位
void W25Q128_Write_Page(u8* pBuffer, u32 addr, u16 numberToWrite)
{
    int i;
    W25QXX_Write_Enable();//写使能
    W25QXX_CS = 0;//片选
    SPI1_ReadWriteByte(W25Q128WriteData);//写命令
    SPI1_ReadWriteByte((u8)addr >> 16);//地址
    SPI1_ReadWriteByte((u8)addr >> 8);
    SPI1_ReadWriteByte((u8)addr);
    for (i = 0; i < numberToWrite; i++)
        SPI1_ReadWriteByte(pBuffer[i]);

    W25QXX_CS = 1;//取消片选    
	  W25QXX_Wait_Busy();
}

//W25Q128写一页数据,page 为Flash的最小单位
void W25Q128_Write_Page(u8* pBuffer, u32 addr, u16 numberToWrite)
{
 	u16 i;  
    W25Q128_Write_Enable();                  //SET WEL 
	W25QXX_CS=0;                            //使能器件   
    mySPI1_ReadWriteByte(W25X_PageProgram);      //发送写页命令   
	mySPI1_ReadWriteByte(addr >> 16);//地址
	mySPI1_ReadWriteByte(addr >> 8);
	mySPI1_ReadWriteByte(addr);  
    for (i = 0; i < numberToWrite; i++)
        mySPI1_ReadWriteByte(pBuffer[i]);
	W25QXX_CS=1;                            //取消片选 
	W25QXX_Wait_Busy();					   //等待写入结束
}