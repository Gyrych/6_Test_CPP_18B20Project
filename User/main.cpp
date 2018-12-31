#include <iostream>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <wiringPi.h>

#define DQ 21	//数据引脚编号

class DS18B20 
{
	public:
	DS18B20();			//构造函数
	~DS18B20();			//析构函数
	double GetTemperature(void);	//读取温度
	
	private:
	void _init(void);		//初始化IO
	int _ds18b20_init(void);	//初始化18B20
	void _write_byte(unsigned char data);	//写一个字节
	unsigned char _read_byte(void);	//读一个字节
	void _update(void);		//更新
	double temperature;		//温度值
};

//构造函数
DS18B20::DS18B20():temperature{0}
{
	std::cout << "Debug: In DS18B20" << std::endl;
	wiringPiSetup();	//初始化wiring
	_init();		//初始化IO
}

//析构函数
DS18B20::~DS18B20()
{
	std::cout << "Debug: In ~DS18B20" << std::endl;
}

//初始化IO
void DS18B20::_init(void)
{
	pinMode(DQ, INPUT);
}

//读取温度值
double DS18B20::GetTemperature(void)
{
	_update();
	return temperature;
}

//更新温度值
void DS18B20::_update(void)
{
    int flag;
    unsigned long err;
    unsigned char result[2] = {0x00, 0x00};
    //struct ds18b20_device *dev = filp->private_data;

    flag = _ds18b20_init();
    if (flag)
    {
        std::cout << "ds18b20 init failed!" << std::endl;
        return;
    }
        
    _write_byte(0xcc);
    _write_byte(0x44);
    
    delay(1500);

    flag = _ds18b20_init();
    if (flag)
    {
        std::cout << "ds18b20 init failed!" << std::endl;
        return;
    }

    _write_byte(0xcc);
    _write_byte(0xbe);

    result[0] = _read_byte();    // 温度低八位
    result[1] = _read_byte();    // 温度高八位
    
    temperature = (((int)result[1] << 8) | result[0]) * 0.0625;
    
    return;
}

//复位ds18b20
int DS18B20::_ds18b20_init(void)
{
    int retval = 0;

    digitalWrite(DQ, HIGH);	//保证输出是高电平
    pinMode(DQ, OUTPUT);	//恢复输出
    delayMicroseconds(2);	//维持一会儿
    digitalWrite(DQ, LOW);	//拉低，复位
    delayMicroseconds(500);	//保持复位电平500us
    pinMode(DQ, INPUT);		//释放总线
    delayMicroseconds(60);	//若复位成功，18b20发出存在脉冲（低电平，持续60~240us）
    retval = digitalRead(DQ);	//读取总线上的电平
    delayMicroseconds(500);	//延时到18b20输出完

    digitalWrite(DQ, HIGH);
    pinMode(DQ, INPUT);		//释放总线

    return retval;
}

//向18b20写入一个字节数据
void DS18B20::_write_byte(unsigned char data)
{
    int i = 0;

    digitalWrite(DQ, HIGH);	//保证输出是高电平
    pinMode(DQ, OUTPUT);	//恢复输出

    for (i = 0; i < 8; i ++)
    {
        // 总线从高拉至低电平时，就产生写时隙
        digitalWrite(DQ, HIGH);
        delayMicroseconds(2);
        digitalWrite(DQ, LOW);
        delayMicroseconds(1);
	digitalWrite(DQ, (data & 0x01)?HIGH:LOW);
        delayMicroseconds(60);
	data >>= 1;
    }
    digitalWrite(DQ, HIGH);
    pinMode(DQ, INPUT);		//释放总线
}

//从ds18b20读出一个字节数据
unsigned char DS18B20::_read_byte(void)
{
    int i;
    unsigned char data = 0;

    for (i = 0; i < 8; i++)
    {
        // 总线从高拉至低，只需维持低电平17ts，再把总线拉高，就产生读时隙
        digitalWrite(DQ, HIGH);
	pinMode(DQ, OUTPUT);
        delayMicroseconds(2);
        digitalWrite(DQ, LOW);
        delayMicroseconds(1);
	pinMode(DQ, INPUT);
        delayMicroseconds(8);
	data >>= 1;
	if(digitalRead(DQ))
	    data |= 0x80;
	delayMicroseconds(50);
    }
    digitalWrite(DQ, HIGH);
    pinMode(DQ, INPUT);

    return data;
}

int main(void)
{
    DS18B20 ds18b20_1;
   
    while(1)
    {
    	std::cout << "Temperature: " << ds18b20_1.GetTemperature() << std::endl;
    }

    return 0;
}
