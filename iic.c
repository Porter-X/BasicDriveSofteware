#include "myiic.h"

#define IIC_DELAY delay_us(2);

uint8_t iic_read_byte(unsigned char ack);/* IIC读取一个字节 */

/**
 * @brief       初始化IIC
 * @param       无
 * @retval      无
 */
void iic_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    IIC_SCL_GPIO_CLK_ENABLE();  /* SCL引脚时钟使能 */
    IIC_SDA_GPIO_CLK_ENABLE();  /* SDA引脚时钟使能 */

    gpio_init_struct.Pin = IIC_SCL_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;        /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; /* 快速 */
    HAL_GPIO_Init(IIC_SCL_GPIO_PORT, &gpio_init_struct);/* SCL */

    gpio_init_struct.Pin = IIC_SDA_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_OD;        /* 开漏输出 */
    HAL_GPIO_Init(IIC_SDA_GPIO_PORT, &gpio_init_struct);/* SDA */
    /* SDA引脚模式设置,开漏输出,上拉, 这样就不用再设置IO方向了, 开漏输出的时候(=1), 也可以读取外部信号的高低电平 */

    iic_stop();     /* 停止总线上所有设备 */
}

/**
	* @brief		产生起始信号
	* @para			none
	* @retval   none
	*/
void iic_start()
{
	IIC_SCL(1);
	IIC_SDA(1);//SDA 在高低电平期间由高变低为开始
	IIC_DELAY;
	IIC_SDA(0);
	IIC_DELAY;
	IIC_SCL(0);//钳住时钟为低电平等待发送数据
	IIC_DELAY;//保持一会
}

/**
	* @brief		产生结束信号
	* @para			none
	* @retval   none
	*/
void iic_stop(void)
{
	IIC_SDA(0);//SDA 在高低电平期间由低变高为结束
	IIC_DELAY;
	IIC_SCL(1);
	IIC_DELAY;
	IIC_SDA(1);
	IIC_DELAY;
}

/**
	* @brief		发送一个字节
	* @para			发送的数据
	* @retval   none
	*/
void iic_send_byte(uint8_t txd)
{
	//SDA开漏输出，外接上拉电阻，不需要调整输入输出方向
	uint8_t i;
	
	for (i = 0; i < 8; i++)
	{
		IIC_SDA((txd & 0x80)>>7);//先发MSB
		txd <<= 1;//左移
		IIC_DELAY;
		IIC_SCL(1);//维持一段时间
		IIC_DELAY;
		IIC_SCL(0);
		IIC_DELAY;
	}
	IIC_SDA(1);//释放IIC总线
}

/**
	* @brief		等待ack
	* @para			发送的数据
	* @retval   none
	*/
uint8_t iic_wait_ack(void)
{
	uint8_t ucErrTime = 0;
	
	IIC_SDA(1);//拉高
	IIC_DELAY;
	IIC_SCL(1);//等待从机拉低SDA
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
	* @brief		不应答
	* @para			none
	* @retval   none
	*/
void iic_nack()
{
	IIC_SDA(1);//一直为高
	IIC_DELAY;
	IIC_SCL(1);
	IIC_DELAY;
	IIC_SCL(0);
	IIC_DELAY;
	
}
/**
	* @brief		应答
	* @para			none
	* @retval   none
	*/
void iic_ack()
{

	IIC_SDA(0);
	IIC_DELAY;
	IIC_SCL(1);
	IIC_DELAY;
	IIC_SDA(1);//低到高
	IIC_DELAY;	
}

/**
	* @brief		读取一个字节
	* @para			是否应答
	* @retval   读取的数据
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
		if(IIC_READ_SDA)//SDA总线数据在SCL=0是才允许翻转
			recv++;
		//最后收到的数据不用左移
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

//    for (i = 0; i < 8; i++ )    /* 接收1个字节数据 */
//    {
//        receive <<= 1;  /* 高位先输出,所以先收到的数据位要左移 */
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
//        iic_nack();     /* 发送nACK */
//    }
//    else
//    {
//        iic_ack();      /* 发送ACK */
//    }

//    return receive;
//}

