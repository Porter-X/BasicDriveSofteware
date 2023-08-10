#include <stdio.h>
#include <stdint.h>
#include <types.h>

#define W25QXX_CS PBout //Ƭѡ�ź�
#define W25Q128WriteEnable 0x06
#define W25Q128ReadData 0x03
#define W25Q128WriteData 0x02
#define W25Q128ReadStatusRegister 0x05

u8 mySPI1_ReadWriteByte(u8 Txdata)
{
	//���ͻ�����Ϊ��
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) {};//���ͻ�����Ϊ������ѭ��
	SPI_I2S_SendData(SPI1, Txdata);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {};// SPI->DR���ݼĴ�����Ϊ������ѭ��
	
	return SPI_I2S_ReceiveData(SPI1);
}
//W25Q128дʹ��
void W25Q128_Write_Enable(void)
{
	W25QXX_CS = 0;
	mySPI1_ReadWriteByte(W25Q128WriteEnable);//����дʹ��
	W25QXX_CS = 1;
}

//��ȡW25QXX��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
u8 W25Q128_ReadSR()
{
    u8 temp;
	W25QXX_CS = 0;
	mySPI1_ReadWriteByte(W25Q128ReadStatusRegister);//��״̬�Ĵ���
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
        if (temp & 0x01) //��0bitΪ������ѭ��
            break;
    } */
    while((W25Q128_ReadSR() & 0x01) == 0x01);//��0bitΪ������ѭ��
}

//W25Q128дһҳ����,page ΪFlash����С��λ
void W25Q128_Write_Page(u8* pBuffer, u32 addr, u16 numberToWrite)
{
    int i;
    W25Q128_Write_Enable();//дʹ��
    W25QXX_CS = 0;//Ƭѡ
    mySPI1_ReadWriteByte(W25Q128WriteData);//д����
    mySPI1_ReadWriteByte((u8)addr >> 16);//��ַ
    mySPI1_ReadWriteByte((u8)addr >> 8);
    mySPI1_ReadWriteByte((u8)addr);
    for (i = 0; i < numberToWrite; i++)
        mySPI1_ReadWriteByte(pBuffer[i]);
    W25Q128_Wait_Busy();
    W25QXX_CS = 1;//ȡ��Ƭѡ    
}

//����sectorΪFLASH��������С��λ
void W25Q128_Write_Sector(u8* pBuffer, u32 addr, u16 numberToWrite)
{
    
}

void W25Q128_Read(u8* pBuffer, u32 addr, u16 size)
{
	int i;
	W25QXX_CS = 0;//ƬѡW25Q128
	mySPI1_ReadWriteByte(0x03);//������
	mySPI1_ReadWriteByte(addr >> 16);//��ַ
	mySPI1_ReadWriteByte(addr >> 8);
	mySPI1_ReadWriteByte(addr);
	
	for (i = 0; i < size; i++)
	{
		pBuffer[i] = mySPI1_ReadWriteByte(0xff);//��������
	}
	W25QXX_CS = 1;
}