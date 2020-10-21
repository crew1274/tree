/*
 * modbus.h
 *
 *  Created on: 2018年9月21日
 *      Author: 171104
 */

#ifndef SRC_MODBUS_H_
#define SRC_MODBUS_H_


#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <iconv.h>
#include "../utility.h"
#include "../dev/uart.h"
#include "Poco/Mutex.h"

#define MODBUS_SERIAL_DEV           "/dev/ttyUL5"
#define MODBUS_SERIAL_BAUDRATE      9600    /* 9600, 38400, 115200, ... */
#define MODBUS_SERIAL_PARITY        'N'     /* 'N', 'E', or 'O' */
#define MODBUS_SERIAL_DATABITS      8       /* 5, 6, 7, or 8 */
#define MODBUS_SERIAL_STOPBITS      1       /* 1 or 2 */
#define MODBUS_DEVICE_ID            0x01

#define modbus_function_02 			0x02	// input bits (modbus function code 0x02)
#define modbus_function_01 			0x01	// coils bits (modbus function code 0x01)
#define modbus_function_04			0x04	// input registers (modbus function code 0x04)
#define modbus_function_03			0x03	// holding registers (modbus function code 0x03)
#define modbus_function_05			0x05	// coil (modbus function code 0x05)
#define modbus_function_06			0x06	// holding register (modbus function code 0x06)
#define modbus_function_15			0x0F	// coils (modbus function code 0x0F)
#define modbus_function_16			0x10	// holding registers (modbus function code 0x10)

#define NONE_convert  0
#define utf16_convert 1
#define ascii_convert 2

//0: none  , 1: utf16 , 2: ascii

/**
 * @brief       Send Modbus data to Water device
 *
 * @param[in]    cmd        Data will be sent to slave
 * @param[out]   getdata    Water Data
 *
 * @return      Return 0 if ok
 */
//int modbus(char *cmd, int &getdata);
//int modbus_10(char *cmd, int &getdata);

class Modbus: private Uart
{
public:
	//HMI(unsigned int Uart_bus_index);
	Modbus(const char *uart_device);
	virtual ~Modbus();
//	int modbus(char *cmd, int &getdata);
	int modbus(unsigned char modbus_slave_id,
			unsigned char modbus_function_code, unsigned char *modbus_address,
			const char *data,int datalength, unsigned char *getdata, int converter);
	int modbus_modbus_function_01(unsigned char modbus_slave_id,
			unsigned char modbus_function_code, unsigned char *modbus_address,
			const char *data,int datalength, unsigned char *getdata, int converter);
	int modbus_modbus_function_02(unsigned char modbus_slave_id,
			unsigned char modbus_function_code, unsigned char *modbus_address,
			const char *data,int datalength, unsigned char *getdata, int converter);
	int modbus_modbus_function_03(unsigned char modbus_slave_id,
			unsigned char modbus_function_code, unsigned char *modbus_address,
			const char *data,int datalength, unsigned char *getdata, int converter);
	int modbus_modbus_function_04(unsigned char modbus_slave_id,
			unsigned char modbus_function_code, unsigned char *modbus_address,
			const char *data,int datalength, unsigned char *getdata, int converter);
	int modbus_modbus_function_05(unsigned char modbus_slave_id,
			unsigned char modbus_function_code, unsigned char *modbus_address,
			const char *data,int datalength, unsigned char *getdata, int converter);
	int modbus_modbus_function_06(unsigned char modbus_slave_id,
			unsigned char modbus_function_code, unsigned char *modbus_address,
			const char *data,int datalength, unsigned char *getdata, int converter);
	int modbus_modbus_function_15(unsigned char modbus_slave_id,
			unsigned char modbus_function_code, unsigned char *modbus_address,
			const char *data,int datalength, unsigned char *getdata, int converter);
	int modbus_modbus_function_16(unsigned char modbus_slave_id,
			unsigned char modbus_function_code, unsigned char *modbus_address,
			const char *data,int datalength, unsigned char *getdata, int converter);
	int modbus_test_Err(char *cmd, int &getdata);



private:
	Uart *uart_hmi_;
	int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf, size_t outlen);
	int u8tou16(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
	int u8toascii(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
	Poco::Mutex PLCmutex;
};


#endif /* SRC_MODBUS_H_ */s
