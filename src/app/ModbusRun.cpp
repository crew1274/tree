/*
 * ModbusRun.cpp
 *
 *  Created on: 2020年3月18日
 *      Author: 171104
 */

#include <app/ModbusRun.h>

ModbusRun::ModbusRun(const char *uart_device, LayeredConfiguration *_config):
Modbus(uart_device),
pconfig(_config),
logger(Logger::get("ModbusRun")),
_ActiveMethod(this, &ModbusRun::Upload)
{
	// TODO Auto-generated constructor stub

}

ModbusRun::~ModbusRun() {
	// TODO Auto-generated destructor stub
}

void ModbusRun::Background(Timer& timer)
{
	unsigned char read[100];
	std::stringstream payload;
	payload << "MODBUS" ; //目標資料表
	payload << ",utc=+8 "; //狀態描述

	modbus(0x01, modbus_function_03, "28", "1", 1, read, NONE_convert);
    payload << "28" << "=" << float( ((int)read[1])*1.0/100 );

    payload << ",";
    modbus(0x01, modbus_function_03, "29", "1", 1, read, NONE_convert);
    payload << "29" << "=" << float( ((int)read[1]) * 1.0/100 );

    payload << ",";
    modbus(0x02, modbus_function_03, "28", "1", 1, read, NONE_convert);
    payload << "28_2" << "=" << float( ((int)read[1])*1.0/100 );

    payload << ",";
    modbus(0x02, modbus_function_03, "29", "1", 1, read, NONE_convert);
    payload << "29_2" << "=" << float( ((int)read[1]) * 1.0/100 );

    payload << endl;
//    cout << "done:" << payload.str()  <<  endl;
	if(pconfig->getBool("MODBUS.INTO_DB") && payload.str().length()>0)
	{

		this->_ActiveMethod(payload.str());
	}
}

void ModbusRun::Upload(const std::string& payload)
{
	try
	{
		URI target;
		target.setScheme("http");
		target.setHost(pconfig->getString("DATABASE.HOST", "10.11.0.156"));
		target.setPort(8086);
		target.setPath("write");
		target.addQueryParameter("db", pconfig->getString("DATABASE.DB_NAME")); //選擇db

		HTTPClientSession session(target.getHost(), target.getPort());
		HTTPRequest request(Net::HTTPRequest::HTTP_POST, target.toString(), Net::HTTPMessage::HTTP_1_1);
		request.setContentType("application/x-www-form-urlencoded");
		request.setContentLength(payload.length());
		std::ostream& BodyOstream = session.sendRequest(request); // sends request, returns open stream
		BodyOstream << payload;  // sends the body
	}
	catch (Exception& exc)
	{
		logger.error(exc.displayText());
	}
}
