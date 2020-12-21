/*
 * TecoRS485.cpp
 *
 *  Created on: 2020�~11��26��
 *      Author: 191004
 */

#include <hal/TecoRS485.h>

TecoRS485::TecoRS485(const char *uart_device):Modbus(uart_device)
{
	// TODO Auto-generated constructor stub
}

TecoRS485::~TecoRS485()
{
	// TODO Auto-generated destructor stub
}

void TecoRS485::SetDevID(unsigned char _RemoteSlaveID)
{
	RemoteSlaveID = _RemoteSlaveID;
}

int TecoRS485::ReFleshOneCatch(std::string axis) {

	unsigned char getdata[20]={0};
    int get_response=0;
	if(axis == "Z")
	{
		get_response = Modbus::modbus_modbus_function_06(RemoteSlaveID,
				modbus_function_06,10032,"259",1,getdata,NONE_convert);
		//{ MODBUS_DEVICE_ID, modbus_function_05, "1001", "1",1, read, NONE_convert }
		if(get_response==-1)
		{
			printf("Erroe!!There is No response Setting Z Fail!!");
		}
		else
		{
		    printf("Sucess Setting Z !!");
		}
	}
	else if(axis == "Y")
	{
		get_response=Modbus::modbus_modbus_function_06(RemoteSlaveID,
				modbus_function_06,10032,"258",1,getdata,NONE_convert);
		//{ MODBUS_DEVICE_ID, modbus_function_05, "1001", "1",1, read, NONE_convert }
		if(get_response==-1)
		{
			printf("Erroe!!There is No response Setting Y Fail!!");
		}
		else
		{
		    printf("Sucess Setting Y !!");
		}
	}
	else if(axis == "X")
	{
		get_response=Modbus::modbus_modbus_function_06(RemoteSlaveID,
				modbus_function_06,10032,"257",1,getdata,NONE_convert);
		//{ MODBUS_DEVICE_ID, modbus_function_05, "1001", "1",1, read, NONE_convert }
		if(get_response==-1)
		{
			printf("Erroe!!There is No response Setting X Fail!!");
		}
		else
		{
		    printf("Sucess Setting X !!");
		}
	}

	return get_response;

}

int TecoRS485::ReadBufferData(uint32_t *geDataBack) {
	unsigned char getdata[20]={0};
	int get_response=0;
	get_response=Modbus::modbus_modbus_function_03(RemoteSlaveID,
			modbus_function_03,10000,"2",2,getdata,NONE_convert);
	if(get_response!=-1)
	{
		printf("Get Response!");
		Utility::ShowHexData("RevDATA:", getdata, 20);
		for(int i=0; i<get_response;i+=4)
		{
			if(i<=4)
			{
			geDataBack[0]=(getdata[i]<<24)+(getdata[i+1]<<16)+(getdata[i+2]<<8)+(getdata[i+3]);
			printf("DataLength: %d ,%d,",get_response,geDataBack[0]);
			}
			else
			{
				geDataBack[i/4]=(getdata[i]<<24)+(getdata[i+1]<<16)+(getdata[i+2]<<8)+(getdata[i+3]);
				printf("DataLength: %d ,%d,",get_response,geDataBack[i/4]);
			}
		}
	}
	else
	{
		printf("Somthing Wrong!");
	}

	return get_response;
}

void IntconvertChar(int inputData,unsigned char *CarryBack, int bufferSize)
{
	memset(CarryBack,0,bufferSize);
	for(int i=0;i<bufferSize;i++)
	{

		CarryBack[bufferSize-1-i]=inputData%10+0x30;
		inputData=inputData/10;
	}
}

int TecoRS485::ReadBufferData6000(std::vector<uint32_t> &geDataBack)
{
	geDataBack.clear();
	unsigned char getdata[200];
	int get_response = -1;
//	int dd=0;
//	unsigned char Convert[5]={0};
//	IntconvertChar(1234,Convert,5);
	uint32_t temp_val;
    for(uint i=0; i<40 ;i++)
    {
		get_response = Modbus::modbus_modbus_function_03(RemoteSlaveID, modbus_function_03,(6000+i * 100), "100", 2, getdata, NONE_convert);
		if(get_response != -1)
		{
			printf("Get Data Number: %d",get_response);
			for( int j=0; j<get_response;j+=4)
			{
				temp_val = (getdata[j]<<24)+(getdata[j+1]<<16)+(getdata[j+2]<<8)+(getdata[j+3]);
//				if( temp_val >= 3500000000)
//				{
//					temp_val = temp_val - 4281466653;
//				}
				geDataBack.push_back(temp_val);
//				dd++;
//				printf("Count:%d",dd);
			}
		}
    }
  return get_response;
}

int TecoRS485::ReadOfficialFFT(uint32_t *geDataBack) {
	unsigned char getdata[200]={0};
	int get_response=-1;
	int dd=0;
//	unsigned char Convert[5]={0};
//	IntconvertChar(1234,Convert,5);
    for(int i=0;i<20;i++)
    {
		get_response=Modbus::modbus_modbus_function_03(RemoteSlaveID,
				modbus_function_03,(0+i*100),"100",2,getdata,NONE_convert);
		if(get_response!=-1)
		{
			printf("Get Data Number: %d",get_response);
			for(int j=0; j<get_response;j+=4)
			{
				if(j<=4)
				{
					geDataBack[0]=(getdata[j]<<24)+(getdata[j+1]<<16)+(getdata[j+2]<<8)+(getdata[j+3]);
				}
				else
				{
					geDataBack[j/4]=(getdata[j]<<24)+(getdata[j+1]<<16)+(getdata[j+2]<<8)+(getdata[j+3]);
				}
				dd++;
				printf("Count:%d",dd);

			}
		}
    }
    return get_response;
}

int TecoRS485::Read_FW_Version(uint32_t &geDataBack)
{
	unsigned char getdata[20]={0};
	int get_response=-1;
//	unsigned char Convert[5]={0};
//	IntconvertChar(1234,Convert,5);
	get_response=Modbus::modbus_modbus_function_03(RemoteSlaveID,
			modbus_function_03,10020,"1",2,getdata,NONE_convert);
	if(get_response!=-1)
	{
		printf("Get Data Number: %d",get_response);
		geDataBack = (getdata[0]<<8)+getdata[1];
	}
    return get_response;
}

int TecoRS485::Read_Core_Temp(uint32_t &geDataBack)
{
	unsigned char getdata[20]={0};
	int get_response=-1;
//	unsigned char Convert[5]={0};
//	IntconvertChar(1234,Convert,5);
	get_response=Modbus::modbus_modbus_function_03(RemoteSlaveID,
			modbus_function_03,10015,"1",2,getdata,NONE_convert);
	if(get_response!=-1)
	{
		printf("Get Data Number: %d",get_response);
		geDataBack = (getdata[0]<<8)+getdata[1];

	}
    return get_response;
}

int TecoRS485::ResetSensorID(int NewID)
{
	unsigned char getdata[20]={0};
	unsigned char Convert[3]={0};
    int get_response=0;
    IntconvertChar(NewID,Convert,5);

    get_response=Modbus::modbus_modbus_function_06(RemoteSlaveID,
    		modbus_function_06,10033,"1",1,getdata,NONE_convert);
    if(get_response!=-1)get_response=Modbus::modbus_modbus_function_06(RemoteSlaveID,modbus_function_06,10040,Convert,1,getdata,NONE_convert);
	return get_response;
}


