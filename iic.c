#include "myiic.h"

#define IIC_DELAY delay_us(2);

uint8_t iic_read_byte(unsigned char ack);/* IIC��ȡһ���ֽ� */

/**
 * @brief       ��ʼ��IIC
 * @param       ��
 * @retval      ��
 */
void iic_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    IIC_SCL_GPIO_CLK_ENABLE();  /* SCL����ʱ��ʹ�� */
    IIC_SDA_GPIO_CLK_ENABLE();  /* SDA����ʱ��ʹ�� */

    gpio_init_struct.Pin = IIC_SCL_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;        /* ������� */
    gpio_init_struct.Pull = GPIO_PULLUP;                /* ���� */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; /* ���� */
    HAL_GPIO_Init(IIC_SCL_GPIO_PORT, &gpio_init_struct);/* SCL */

    gpio_init_struct.Pin = IIC_SDA_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_OD;        /* ��©��� */
    HAL_GPIO_Init(IIC_SDA_GPIO_PORT, &gpio_init_struct);/* SDA */
    /* SDA����ģʽ����,��©���,����, �����Ͳ���������IO������, ��©�����ʱ��(=1), Ҳ���Զ�ȡ�ⲿ�źŵĸߵ͵�ƽ */

    iic_stop();     /* ֹͣ�����������豸 */
}

/**
	* @brief		������ʼ�ź�
	* @para			none
	* @retval   none
	*/
void iic_start()
{
	IIC_SCL(1);
	IIC_SDA(1);//SDA �ڸߵ͵�ƽ�ڼ��ɸ߱��Ϊ��ʼ
	IIC_DELAY;
	IIC_SDA(0);
	IIC_DELAY;
	IIC_SCL(0);//ǯסʱ��Ϊ�͵�ƽ�ȴ���������
	IIC_DELAY;//����һ��
}

/**
	* @brief		���������ź�
	* @para			none
	* @retval   none
	*/
void iic_stop(void)
{
	IIC_SDA(0);//SDA �ڸߵ͵�ƽ�ڼ��ɵͱ��Ϊ����
	IIC_DELAY;
	IIC_SCL(1);
	IIC_DELAY;
	IIC_SDA(1);
	IIC_DELAY;
}

/**
	* @brief		����һ���ֽ�
	* @para			���͵�����
	* @retval   none
	*/
void iic_send_byte(uint8_t txd)
{
	//SDA��©���������������裬����Ҫ���������������
	uint8_t i;
	
	for (i = 0; i < 8; i++)
	{
		IIC_SDA((txd & 0x80)>>7);//�ȷ�MSB
		txd <<= 1;//����
		IIC_DELAY;
		IIC_SCL(1);//ά��һ��ʱ��
		IIC_DELAY;
		IIC_SCL(0);
		IIC_DELAY;
	}
	IIC_SDA(1);//�ͷ�IIC����
}

/**
	* @brief		�ȴ�ack
	* @para			���͵�����
	* @retval   none
	*/
uint8_t iic_wait_ack(void)
{
	uint8_t ucErrTime = 0;
	
	IIC_SDA(1);//����
	IIC_DELAY;
	IIC_SCL(1);//�ȴ��ӻ�����SDA
	IIC_DELAY;
	while (IIC_READ_SDA)
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			iic_stop();
			return 1;
		}
	}
	IIC_SCL(0);
	IIC_DELAY;
	return 0;
}
/**
	* @brief		��Ӧ��
	* @para			none
	* @retval   none
	*/
void iic_nack()
{
	IIC_SDA(1);//һֱΪ��
	IIC_DELAY;
	IIC_SCL(1);
	IIC_DELAY;
	IIC_SCL(0);
	IIC_DELAY;
	
}
/**
	* @brief		Ӧ��
	* @para			none
	* @retval   none
	*/
void iic_ack()
{

	IIC_SDA(0);
	IIC_DELAY;
	IIC_SCL(1);
	IIC_DELAY;
	IIC_SDA(1);//�͵���
	IIC_DELAY;	
}

/**
	* @brief		��ȡһ���ֽ�
	* @para			�Ƿ�Ӧ��
	* @retval   ��ȡ������
	*/
uint8_t iic_read_byte(unsigned char ack)
{
	uint8_t i,recv = 0;
	for (i = 0; i < 8; i++)
	{
		recv <<= 1;
		IIC_SCL(0);
		IIC_DELAY;
		IIC_SCL(1);
		IIC_DELAY;
		if(IIC_READ_SDA)//SDA����������SCL=0�ǲ�����ת
			recv++;
		//����յ������ݲ�������
		IIC_DELAY;
	}
	if(!ack)
		iic_nack();
	else
		iic_ack();
	
	return recv;
}


//uint8_t iic_read_byte(uint8_t ack)
//{
//    uint8_t i, receive = 0;

//    for (i = 0; i < 8; i++ )    /* ����1���ֽ����� */
//    {
//        receive <<= 1;  /* ��λ�����,�������յ�������λҪ���� */
//        IIC_SCL(1);
//        IIC_DELAY

//        if (IIC_READ_SDA)
//        {
//            receive++;
//        }
//        
//        IIC_SCL(0);
//        IIC_DELAY
//    }

//    if (!ack)
//    {
//        iic_nack();     /* ����nACK */
//    }
//    else
//    {
//        iic_ack();      /* ����ACK */
//    }

//    return receive;
//}

