/*
 * TecoRS485.h
 *
 *  Created on: 2020�~11��26��
 *      Author: 191004
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../hal/modbus.h"
#include "utility.h"

#ifndef SRC_TECORS485_H_
#define SRC_TECORS485_H_

struct TecoVibrationStruct
{
	char Cmd;
	char Pointtype;
    int  ElmentStart;
	char ErrorCode;
	int  ElementNum;
	char BitNumber;
	int  BufferLength;
	unsigned int WriteDataBuffer;
	unsigned int ReadDataBuffer;
};

class TecoRS485 : private Modbus
{
public:
	TecoRS485(const char *uart_device);
	void SetDevID(unsigned char _RemoteSlaveID);
	int ReFleshOneCatch(std::string axis);
	int ReadBufferData(uint32_t *geDataBack);
	int ReadBufferData6000(std::vector<uint32_t> &geDataBack);
	int ReadOfficialFFT(uint32_t *geDataBack);
	int Read_FW_Version(uint32_t &geDataBack);
	int Read_Core_Temp(uint32_t &geDataBack);
	int ResetSensorID(int NewID);
	virtual ~TecoRS485();

private:
	unsigned char RemoteSlaveID;
};

#endif /* SRC_TECORS485_H_ */
