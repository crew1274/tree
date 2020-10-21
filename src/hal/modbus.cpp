/*
 * modbus.cpp
 *
 *  Created on: 2018年9月21日
 *      Author: 171104
 */

#include <hal/modbus.h>

#define en_print 0
Modbus::Modbus(const char *uart_device) :
		Uart(uart_device) {

//	Uart uartPL1(MODBUS_SERIAL_DEV);
	Uart::SetOptions(MODBUS_SERIAL_DATABITS, MODBUS_SERIAL_STOPBITS,
			MODBUS_SERIAL_PARITY);
//	uart_hmi_= uart_device;
//	uart_hmi_->SetOptions(8,1,'N');

}
Modbus::~Modbus() {
	//
}

/**
 * @brief       Modbus Protocol
 * @param[in]   modbus_slave_id      (ID=0x01)
 * @param[in] 	modbus_function_code (0x01: Read Signal bit, 0x03: Read Signal Register, 0x05: Write Signal bit,
 *                                    0x06: Write Signal Register, 0x10: Write Multiple Registers)
 * @param[in] 	modbus_address       (address)
 * @param[in]   data                 (Data or Read Data length)
 * @param[in]   datalength           (datalength  - word )
 * @param[out]  getdata              (Get Modbus Data)
 * @param[in] 	converter            (0: NONE_convert  , 1: utf16_convert , 2: ascii_convert)
 * @return      Return               (0 = ok , -1 = error)
 *
 */
int Modbus::modbus(unsigned char modbus_slave_id,
		unsigned char modbus_function_code, unsigned char *modbus_address,
		const char *data, int datalength, unsigned char *getdata,
		int converter) {

	int int_address = atoi((const char *) modbus_address);
	PLCmutex.lock();
	switch (modbus_function_code) {
	case 0x01:
#if en_print
		printf("Start Function 0x01 ... \r\n");
#endif
		modbus_modbus_function_01(modbus_slave_id, modbus_function_code,
				(unsigned char *) int_address, data, datalength, getdata,
				converter);
		break;
	case 0x02:
#if en_print
		printf("Start Function 0x02 ... \r\n");
#endif
		modbus_modbus_function_02(modbus_slave_id, modbus_function_code,
				(unsigned char *) int_address, data, datalength, getdata,
				converter);
		break;
	case 0x03:
#if en_print
		printf("Start Function 0x03 ... \r\n");
#endif
		modbus_modbus_function_03(modbus_slave_id, modbus_function_code,
				(unsigned char *) int_address, data, datalength, getdata,
				converter);
		break;
	case 0x04:
#if en_print
		printf("Start Function 0x04 ... \r\n");
#endif
		modbus_modbus_function_04(modbus_slave_id, modbus_function_code,
				(unsigned char *) int_address, data, datalength, getdata,
				converter);
		break;
	case 0x05:
#if en_print
		printf("Start Function 0x05 ... \r\n");
#endif
		modbus_modbus_function_05(modbus_slave_id, modbus_function_code,
				(unsigned char *) int_address, data, datalength, getdata,
				converter);
		break;
	case 0x06:
#if en_print
		printf("Start Function 0x06 ... \r\n");
#endif
		modbus_modbus_function_06(modbus_slave_id, modbus_function_code,
				(unsigned char *) int_address, data, datalength, getdata,
				converter);
		break;
	case 0x0F:
#if en_print
		printf("Start Function 0x0F (15) ... \r\n");
#endif
		modbus_modbus_function_15(modbus_slave_id, modbus_function_code,
				(unsigned char *) int_address, data, datalength, getdata,
				converter);
		break;
	case 0x10:
#if en_print
		printf("Start Function 0x10 (16) ... \r\n");
#endif
		modbus_modbus_function_16(modbus_slave_id, modbus_function_code,
				(unsigned char *) int_address, data, datalength, getdata,
				converter);
		break;

	}
	PLCmutex.unlock();
}

/*
 * @brief       modbus_modbus_function_01  : Read Signal bit
 */

int Modbus::modbus_modbus_function_01(unsigned char modbus_slave_id,
		unsigned char modbus_function_code, unsigned char *modbus_address,
		const char *data, int datalength, unsigned char *getdata,
		int converter) {

#if 1
	int data_length = sizeof(data);
	data_length = 2;
	unsigned char Send_buffer[data_length + 6];
	int len_o = sizeof(Send_buffer);
	unsigned char G_buffer[100];
	len_o = sizeof(G_buffer);
	unsigned char modbus_getdata[512];
	int nnn = 0;
//	printf("modbus 01 start...\r\n");

	Send_buffer[0] = modbus_slave_id;
	Send_buffer[1] = modbus_function_code;
	Send_buffer[2] = (int) modbus_address >> 8 & 0xff;
	Send_buffer[3] = (int) modbus_address & 0xff;

	int int_data = atoi((const char *) data);

	Send_buffer[4] = 0x00;
	Send_buffer[5] = 0x01;
#if 0
	for (int i = 0; i < data_length; i++) {
		Send_buffer[i + 4] = data[i];
		printf("[%d]%02X,%02X\r\n", data_length, Send_buffer[i + 4], data[i]);
	}
#endif
	Send_buffer[data_length + 4] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) >> 8;
	Send_buffer[data_length + 5] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) & 0x00FF;
#if en_print
	printf("\r\n");
	for (int i = 0; i < data_length + 6; i++) {
		printf("%02X,", Send_buffer[i]);
	}
	printf("\r\n");
	printf("Modbus 0x01 Send... ");
	for (int i = 0; i < sizeof(Send_buffer); i++) {
		printf("%02X", Send_buffer[i]);
	}
	printf("\r\n");
#endif

	Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1

	memset(G_buffer, 0, sizeof(G_buffer));
	memset(getdata, 0, sizeof(getdata));

	nnn = 0;
	nnn = Uart::Get(( char*) G_buffer, len_o);
	if (nnn == -1) {
		DBG("Retry...\r\n");
		Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1

		memset(G_buffer, 0, sizeof(G_buffer));
		memset(getdata, 0, sizeof(getdata));

		nnn = Uart::Get(( char*) G_buffer, len_o);
	}
#if en_print
	printf("Modbus 0x01 Get... ");
	for (int i = 0; i < sizeof(G_buffer); i++) {
		printf("%02X", G_buffer[i]);
	}
	printf("\r\n");
#endif
	if (nnn == 0 && G_buffer[0] == modbus_slave_id
			&& G_buffer[1] == modbus_function_code) {	//
		unsigned char getR = G_buffer[3] &0x01;
		memcpy(&getdata[0], &getR , sizeof(G_buffer[3]));
//		memcpy(&getdata[0], &G_buffer[3] , sizeof(G_buffer[3]));
//		printf("\nModbus 0x01 modbus_getdata => %x \r\n", getdata);
		return 0;
	} else {
		DBG("Modbus 0x01 Get Error!\r\n");
		memset(modbus_getdata, -1, sizeof(modbus_getdata));
		memcpy(getdata, modbus_getdata, sizeof(modbus_getdata));
		return -1;
	}
#endif

}

/*
 * @brief       modbus_modbus_function_02  : Read Signal bit (None)
 */

int Modbus::modbus_modbus_function_02(unsigned char modbus_slave_id,
		unsigned char modbus_function_code, unsigned char *modbus_address,
		const char *data, int datalength, unsigned char *getdata,
		int converter) {

#if 1

#endif
}

/*
 * @brief       modbus_modbus_function_03  : Read Signal Register
 */

int Modbus::modbus_modbus_function_03(unsigned char modbus_slave_id,
		unsigned char modbus_function_code, unsigned char *modbus_address,
		const char *data, int datalength, unsigned char *getdata,
		int converter) {

#if 1
	int data_length = sizeof(data) / 2;
	data_length = 2;
	unsigned char Send_buffer[data_length + 6];
	unsigned char Get_buffer[100];
	int len_o = sizeof(Get_buffer);
	char *in = strdup(data);
	char out[255];
	int nnn = 0;
//	printf("modbus start...\r\n");

	memcpy(Send_buffer, "0", sizeof(Send_buffer));
	memcpy(Send_buffer, &modbus_slave_id, sizeof(modbus_slave_id));
	memcpy(&Send_buffer[1], &modbus_function_code,
			sizeof(modbus_function_code));
	Send_buffer[2] = (int) modbus_address >> 8 & 0xff;
	Send_buffer[3] = (int) modbus_address & 0xff;
#if en_print
	printf("[%d] %x, %x, %x, %x\r\n", sizeof(Send_buffer), Send_buffer[0],
			Send_buffer[1], Send_buffer[2], Send_buffer[3]);
#endif
	int int_data = atoi((const char *) data);
	Send_buffer[4] = (int) int_data >> 8 & 0xff;
	Send_buffer[5] = (int) int_data & 0xff;

	unsigned char modbus_getdata[512];
	memcpy(modbus_getdata, "0", sizeof(modbus_getdata));
	memcpy(getdata, "0", sizeof(getdata));

#if 0
	if (converter == 1) {
		u8tou16(in, strlen(in), out, 255);
		std::cout << "Test... " << out << std::endl;
		std::cout << std::hex << out;
		printf("[%d , %d],%X,%X,%X,%X\r\n", sizeof(data), sizeof(out), out[0],
				out[1], out[2], out[3]);
		memcpy(&Send_buffer[4], out, sizeof(Send_buffer) - 6);
	} else if (converter == 2) {
		u8toascii(in, strlen(in), out, 255);
		std::cout << "Test... " << out << std::endl;
		std::cout << std::hex << out;
		printf("[%d , %d],%X,%X,%X,%X\r\n", sizeof(data), sizeof(out), out[0],
				out[1], out[2], out[3]);
		memcpy(&Send_buffer[4], out, sizeof(Send_buffer) - 6);
	} else {
		for (int i = 0; i < data_length; i++) {
			Send_buffer[i + 4] = data[i];
			printf("[%d]%02X,%02X\r\n", data_length, Send_buffer[i + 4],
					data[i]);
		}
	}
#endif
	// CRC-16
	Send_buffer[data_length + 4] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) >> 8;
	Send_buffer[data_length + 5] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) & 0x00FF;
#if en_print
	printf("Modbus 0x03 Send... ");
	for (int i = 0; i < sizeof(Send_buffer); i++) {
		printf("%02X", Send_buffer[i]);
	}
	printf("\r\n");
#endif
	Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1

	memset(Get_buffer, 0, sizeof(Get_buffer));

	nnn = 0;
	nnn = Uart::Get(( char*) Get_buffer, len_o);
	if (nnn == -1) {
		DBG("Retry...\r\n");
		Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1
		memset(Get_buffer, 0, sizeof(Get_buffer));
		nnn = Uart::Get(( char*) Get_buffer, len_o);
	}
#if en_print
	printf("Modbus 0x03 Get... ");
	for (int i = 0; i < sizeof(Get_buffer); i++) {
		printf("%02X", Get_buffer[i]);
	}
	printf("\r\n");
#endif
	if (nnn == 0 && Get_buffer[0] == modbus_slave_id
			&& Get_buffer[1] == modbus_function_code) {	//
		memcpy(&getdata[0], &Get_buffer[3], Get_buffer[2]);
		getdata[Get_buffer[2]] = 0; // \0
		memcpy(&modbus_getdata[0], &getdata[0], sizeof(*getdata));
#if en_print
		for (int k = 0; k < Get_buffer[2]; k = k + 2) {
#if 0
			modbus_getdata |= Get_buffer[Get_buffer[2]+2-k] << (8 * k);
			printf("[%d,%d]0x%X\t",k, Get_buffer[Get_buffer[2]+2-k],modbus_getdata);
#else

			printf("[%d]0x%X\t", k, modbus_getdata[k]);

#endif
		}
#endif

		return 0;
	} else {
		DBG("Modbus 0x03 Uart Get Error!\r\n");
		memset(modbus_getdata, -1, sizeof(modbus_getdata));
		memcpy(getdata, modbus_getdata, sizeof(*getdata));
		return -1;
	}
	return -1;
#endif

}

/*
 * @brief       modbus_modbus_function_04  : Read Signal Register (None)
 */

int Modbus::modbus_modbus_function_04(unsigned char modbus_slave_id,
		unsigned char modbus_function_code, unsigned char *modbus_address,
		const char *data, int datalength, unsigned char *getdata,
		int converter) {

#if 1

#endif
}

/*
 * @brief       modbus_modbus_function_05  : Write Signal bit
 */

int Modbus::modbus_modbus_function_05(unsigned char modbus_slave_id,
		unsigned char modbus_function_code, unsigned char *modbus_address,
		const char *data, int datalength, unsigned char *getdata,
		int converter) {

#if 1
	int data_length = sizeof(data) / 2;
#if en_print
	printf("sizeof(data) ==> [%d]\r\n", sizeof(data) / 2);
#endif
	unsigned char Send_buffer[data_length + 6]; //Send buffer
	int len_o = sizeof(Send_buffer);
	unsigned char G_buffer[100]; // Get buffer
	len_o = sizeof(G_buffer);
	unsigned char modbus_getdata[512];
	int nnn = 0;
//	printf("modbus start...\r\n");
	memcpy(Send_buffer, "0", sizeof(Send_buffer));
	Send_buffer[0] = modbus_slave_id;
	Send_buffer[1] = modbus_function_code;
	Send_buffer[2] = (int) modbus_address >> 8 & 0xff;
	Send_buffer[3] = (int) modbus_address & 0xff;

	if (data[0] == '0') {
		Send_buffer[4] = {0x00};
		Send_buffer[5] = {0x00};
	} else if (data[0]=='1') {
		Send_buffer[4] = {0xFFFF};  // 0xFF
		Send_buffer[5] = {0x0000};
	}
	Send_buffer[data_length + 4] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) >> 8;
	Send_buffer[data_length + 5] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) & 0x00FF;
#if en_print
	printf("\r\n");
	for (int i = 0; i < data_length + 6; i++) {
		printf("%02X,", Send_buffer[i]);
	}
	printf("\r\n");

	printf("Modbus 0x05 Send... ");
	for (int i = 0; i < sizeof(Send_buffer); i++) {
		printf("%02X", Send_buffer[i]);
	}
	printf("\r\n");
#endif

	Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1

	memset(G_buffer, 0, sizeof(G_buffer));
	nnn = 0;
	nnn = Uart::Get(( char*) G_buffer, len_o);
	if (nnn == -1) {
		DBG("Retry...\r\n");
		Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1
		memset(G_buffer, 0, sizeof(G_buffer));
		nnn = Uart::Get(( char*) G_buffer, len_o);
	}
#if en_print
	printf("Modbus 0x05 Get... ");
	for (int i = 0; i < sizeof(G_buffer); i++) {
		printf("%02X", G_buffer[i]);
	}
	printf("\r\n");
#endif

	if (nnn == 0 && G_buffer[0] == modbus_slave_id
			&& G_buffer[1] == modbus_function_code) {	//
		memcpy(modbus_getdata, "0", sizeof(modbus_getdata));
		if (modbus_getdata[0] == '0') {
#if en_print
			DBG("Modbus 0x05 Set Finish!\r\n");
#endif
			memcpy(getdata, modbus_getdata, sizeof(getdata));
			return 0;
		} else {
			DBG("Modbus 0x05 Set Error!\r\n");
			memcpy(getdata, modbus_getdata, sizeof(getdata));
			return -1;
		}

	} else {
		DBG("Modbus 0x05 Get Error!\r\n");
		memcpy(getdata, modbus_getdata, sizeof(getdata));
		return -1;
	}
#endif

}

/*
 * @brief       modbus_modbus_function_06  : Write Signal Register
 */

int Modbus::modbus_modbus_function_06(unsigned char modbus_slave_id,
		unsigned char modbus_function_code, unsigned char *modbus_address,
		const char *data, int datalength, unsigned char *getdata,
		int converter) {

#if 1
	int data_length = sizeof(data) / 2;
	data_length = 2;
	unsigned char Send_buffer[data_length + 6];
	unsigned char Get_buffer[100];
	int len_o = sizeof(Get_buffer);
	char *in = strdup(data);
	char out[255];
	int nnn = 0;
//	printf("modbus start...\r\n");

	memcpy(Send_buffer, "0", sizeof(Send_buffer));
	memcpy(Send_buffer, &modbus_slave_id, sizeof(modbus_slave_id));
	memcpy(&Send_buffer[1], &modbus_function_code,
			sizeof(modbus_function_code));
	Send_buffer[2] = (int) modbus_address >> 8 & 0xff;
	Send_buffer[3] = (int) modbus_address & 0xff;
#if en_print
	printf("[%d] %x, %x, %x, %x\r\n", sizeof(Send_buffer), Send_buffer[0],
			Send_buffer[1], Send_buffer[2], Send_buffer[3]);
#endif
	unsigned char modbus_getdata[512];
	if (converter == 1) {
		for (int i = 0; i < data_length; i++) {
			u8tou16(in, strlen(in), out, 255);
#if en_print
			std::cout << "Test... " << out << std::endl;
			std::cout << std::hex << out;
#endif
		}
#if en_print
		printf("[%d , %d],%X,%X,%X,%X\r\n", sizeof(data), sizeof(out), out[0],
				out[1], out[2], out[3]);
#endif
		memcpy(&Send_buffer[4], out, sizeof(Send_buffer));
	} else {

		int int_data = atoi((const char *) data);
		Send_buffer[4] = (int) int_data >> 8 & 0xff;
		Send_buffer[5] = (int) int_data & 0xff;
	}

	// CRC-16
	Send_buffer[data_length + 4] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) >> 8;
	Send_buffer[data_length + 5] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) & 0x00FF;
#if en_print
	printf("Modbus 0x06 Send... ");
	for (int i = 0; i < sizeof(Send_buffer); i++) {
		printf("%02X", Send_buffer[i]);
	}
	printf("\r\n");
#endif
	Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1
	memset(Get_buffer, 0, sizeof(Get_buffer));

	nnn = 0;
	nnn = Uart::Get(( char*) Get_buffer, len_o);
	if (nnn == -1) {
		DBG("Retry...\r\n");
		Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1

		memset(Get_buffer, 0, sizeof(Get_buffer));

		nnn = Uart::Get(( char*) Get_buffer, len_o);
	}
#if en_print
	printf("Modbus 0x06 Get... ");
	for (int i = 0; i < sizeof(Get_buffer); i++) {
		printf("%02X", Get_buffer[i]);
	}
	printf("\r\n");
#endif
	if (nnn == 0 && Get_buffer[0] == modbus_slave_id
			&& Get_buffer[1] == modbus_function_code) {	//
		for (int k = 0; k < sizeof(Send_buffer); k++) {
			if (Send_buffer[k] == Get_buffer[k]) {
				memcpy(modbus_getdata, "0", sizeof("0"));
#if en_print
				DBG("Modbus 0x06 Set Finish!\r\n");
#endif
				return 0;
			} else {
				memcpy(modbus_getdata, "-1", sizeof("-1"));
				DBG("Modbus 0x06 Set Error!\r\n");
				return -1;
			}

		}
		return 0; //
	} else {
		DBG("Modbus 0x06 Uart Get Error!\r\n");
		memcpy(getdata, modbus_getdata, sizeof(getdata));
		return -1;
	}
	return -1;
#endif
}

/*
 * @brief       modbus_modbus_function_15  : Write Multiple bits
 */

int Modbus::modbus_modbus_function_15(unsigned char modbus_slave_id,
		unsigned char modbus_function_code, unsigned char *modbus_address,
		const char *data, int datalength, unsigned char *getdata,
		int converter) {

#if 1
	int data_length = sizeof(data);
	unsigned char Send_buffer[data_length + 9]; //Send buffer
	int len_o = sizeof(Send_buffer);
	unsigned char G_buffer[100]; // Get buffer
	len_o = sizeof(G_buffer);
	unsigned char modbus_getdata[512];
	int nnn = 0;
//	printf("modbus start...\r\n");
	Send_buffer[0] = modbus_slave_id;
	Send_buffer[1] = modbus_function_code;
//	Send_buffer[2] = modbus_address[0];
//	Send_buffer[3] = modbus_address[1];
	Send_buffer[2] = (int) modbus_address >> 8 & 0xff;
	Send_buffer[3] = (int) modbus_address & 0xff;
	Send_buffer[4] = 0;
	Send_buffer[5] = 2; //data_length / 2 ;
	Send_buffer[6] = data_length;

	for (int i = 0; i < data_length; i++) {
		Send_buffer[i + 7] = data[i];
#if en_print
		printf("[%d]%02X,%02X\r\n", data_length, Send_buffer[i + 7], data[i]);
#endif
	}

	Send_buffer[data_length + 7] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) >> 8;
	Send_buffer[data_length + 8] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) & 0x00FF;
#if en_print
	printf("\r\n");
	for (int i = 0; i < data_length + 6; i++) {
		printf("%02X,", Send_buffer[i]);
	}
	printf("\r\n");

	printf("Modbus 0x0F (15) Send... ");
	for (int i = 0; i < sizeof(Send_buffer); i++) {
		printf("%02X", Send_buffer[i]);
	}
	printf("\r\n");
#endif

	Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1

	memset(G_buffer, 0, sizeof(G_buffer));

	nnn = 0;
	nnn = Uart::Get(( char*) G_buffer, len_o);
	if (nnn == -1) {
		DBG("Retry...\r\n");
		Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1

		memset(G_buffer, 0, sizeof(G_buffer));

		nnn = Uart::Get(( char*) G_buffer, len_o);
	}
#if en_print
	printf("Modbus 0x0F (15) Get... ");
	for (int i = 0; i < sizeof(G_buffer); i++) {
		printf("%02X", G_buffer[i]);
	}
	printf("\r\n");
#endif
	if (nnn == 0 && G_buffer[0] == 0x01
			&& G_buffer[1] == modbus_function_code) {	//

		for (int k = 0; k < 6; k++) { // 01 10 00 01 00 02 (id,fc,address(H),address(L),length(H),length(L))
			if (Send_buffer[k] == G_buffer[k]) {
				memcpy(modbus_getdata, "0", sizeof(modbus_getdata));
			} else {
				memcpy(modbus_getdata, "-1", sizeof(modbus_getdata));
			}

		}
		if (modbus_getdata[0] == '0') {
#if en_print
			DBG("Modbus 0x0F (15) Set Finish!\r\n");
#endif
			memcpy(getdata, modbus_getdata, sizeof(getdata));
			return 0;
		} else {
			DBG("Modbus 0x0F (15) Set Error!\r\n");
			memcpy(getdata, modbus_getdata, sizeof(getdata));
			return -1;
		}

	} else {
		DBG("Modbus 0x0F (15) Get Error!\r\n");
		memcpy(getdata, modbus_getdata, sizeof(getdata));
		return -1;
	}
#endif
}

/*
 * @brief       modbus_modbus_function_16  : Write Multiple Registers
 */

int Modbus::modbus_modbus_function_16(unsigned char modbus_slave_id,
		unsigned char modbus_function_code, unsigned char *modbus_address,
		const char *data, int datalength, unsigned char *getdata,
		int converter) {
//	char *cmd3 = "01 10 00 01 00 02 04 00 00 4F 60 06 7B";
#if 1
	int data_length;

	if (converter == 1) {
		data_length = strlen(data); //* 2;
	} else {
		data_length = strlen(data);
	}
	if ((data_length % 2) == 0) {
		data_length = data_length;
	} else {
		data_length = data_length + 1;
	}
	if (converter == 1) {
		if (((data_length / 2) % 2) == 0) {
			data_length = data_length;
		} else {
			data_length = data_length + 2;
		}
	}
	if (data_length <= datalength * 2)
		data_length = datalength * 2;
#if en_print
	printf("data_length==>%d \r\n",data_length);
#endif
	unsigned char Send_buffer[data_length + 9];
	unsigned char Get_buffer[100];
	int len_o = sizeof(Get_buffer);
	char *in = strdup(data);
	char out[255];
	int nnn = 0;
//	printf("modbus start...\r\n");

	memcpy(Send_buffer, "0", sizeof(Send_buffer));
	memcpy(Send_buffer, &modbus_slave_id, sizeof(modbus_slave_id));
	memcpy(&Send_buffer[1], &modbus_function_code,
			sizeof(modbus_function_code));

//	memcpy(&Send_buffer[2], modbus_address, sizeof(modbus_address));
	Send_buffer[2] = (int) modbus_address >> 8 & 0xff;
	Send_buffer[3] = (int) modbus_address & 0xff;
	Send_buffer[4] = 0;
	Send_buffer[5] = data_length / 2;
	Send_buffer[6] = data_length;
#if en_print
	printf("[%d] %x, %x, %x, %x\r\n", sizeof(Send_buffer), Send_buffer[0],
			Send_buffer[1], Send_buffer[2], Send_buffer[3]);
#endif
	unsigned char modbus_getdata[512];
	if (converter == 1) {
		u8tou16(in, strlen(in), out, 255);
#if en_print
		std::cout << "Test... " << out << std::endl;
		std::cout << std::hex << out;
		printf("[%d , %d],%X,%X,%X,%X\r\n", sizeof(data), sizeof(out), out[0],
				out[1], out[2], out[3]);
#endif
		memcpy(&Send_buffer[7], out, sizeof(Send_buffer) - 9);
	} else if (converter == 2) {
		u8toascii(in, strlen(in), out, 255);
#if en_print
		std::cout << "Test... " << out << std::endl;
		std::cout << std::hex << out;
		printf("[%d , %d],%X,%X,%X,%X\r\n", sizeof(data), sizeof(out), out[0],
				out[1], out[2], out[3]);
#endif
		memcpy(&Send_buffer[7], out, sizeof(Send_buffer) - 9);
	} else {
		for (int i = 0; i < data_length; i++) {
			Send_buffer[i + 7] = data[i];
#if en_print
			printf("[%d]%02X,%02X\r\n", data_length, Send_buffer[i + 4],
					data[i]);
#endif
		}
	}

	// CRC-16
	Send_buffer[data_length + 7] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) >> 8;
	Send_buffer[data_length + 8] = Utility::GetCRC16(Send_buffer,
			sizeof(Send_buffer) - 2) & 0x00FF;
#if en_print
	printf("Modbus 0x10 (16) Send... ");
	for (int i = 0; i < sizeof(Send_buffer); i++) {
		printf("%02X", Send_buffer[i]);
	}
	printf("\r\n");
#endif
	Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1

	memset(Get_buffer, 0, sizeof(Get_buffer));

	nnn = 0;
	nnn = Uart::Get(( char*) Get_buffer, len_o);
	if (nnn == -1) {
		DBG("Retry...\r\n");
		Uart::Send((const char*) Send_buffer, sizeof(Send_buffer)); // sizeof +1

		memset(Get_buffer, 0, sizeof(Get_buffer));

		nnn = Uart::Get(( char*) Get_buffer, len_o);
	}
#if en_print
	printf("Modbus 0x10 (16) Get... ");
	for (int i = 0; i < sizeof(Get_buffer); i++) {
		printf("%02X", Get_buffer[i]);
	}
	printf("\r\n");
#endif
	if (nnn == 0 && Get_buffer[0] == modbus_slave_id
			&& Get_buffer[1] == modbus_function_code) {	//
		for (int k = 0; k < sizeof(Send_buffer); k++) {
			if (Send_buffer[k] == Get_buffer[k]) {
				memcpy(modbus_getdata, "0", sizeof("0"));
#if en_print
				DBG("Modbus 0x10 (16) Set Finish!\r\n");
#endif
				return 0;
			} else {
				memcpy(modbus_getdata, "-1", sizeof("-1"));
				DBG("Modbus 0x10 (16) Set Error!\r\n");
				return -1;
			}

		}
		return 0; //
	} else {
		DBG("Modbus 0x10 (16) Uart Get Error!\r\n");
		memcpy(getdata, modbus_getdata, sizeof(getdata));
		return -1;
	}
	return -1;
#endif
}


/*
 * @brief       code_convert
 */
int Modbus::code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf, size_t outlen)
{
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if(cd == 0)
	{
		return -1;
	}
	memset(outbuf, 0, outlen);
	if(iconv(cd, pin, &inlen, pout, &outlen) == -1)
	{
		return -1;
	}
	iconv_close(cd);
	return 0;
}

int Modbus::u8tou16(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("UTF-8", "UTF-16BE", inbuf, inlen, outbuf, outlen);
}
int Modbus::u8toascii(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("UTF-8", "ascii", inbuf, inlen, outbuf, outlen);
}

