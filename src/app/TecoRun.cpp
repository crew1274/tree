/*
 * TecoRun.cpp
 *
 *  Created on: 2020年12月17日
 *      Author: 171104
 */

#include <app/TecoRun.h>

TecoRun::TecoRun(const char *uart_device, int _id, LayeredConfiguration *_config, std::string _table_name):
TecoRS485(uart_device),
id(_id),
pconfig(_config),
logger(Logger::get("ModbusRun")),
_ActiveMethod(this, &TecoRun::Upload),
table_name(_table_name)
{
	// TODO Auto-generated constructor stub
	SetDevID(id);
	uint32_t geDataBack;
	Read_FW_Version(geDataBack);
	printf("FW VERSION: %d \n",(geDataBack));
//	Read_Core_Temp(geDataBack);
//	printf("%d /n",(geDataBack));
//	ReFleshOneCatch("X");
//	std::vector<uint32_t> geDataBacks;
//	ReadBufferData6000(geDataBacks);
//	logger.information("%u ", geDataBacks[500]);
//	std::string file_name = "teco_g.csv";
//	csv_outputs = new std::ofstream (file_name.c_str(), std::ofstream::app);
//
//	ReadBufferData6000(geDataBacks);
//	cout << "geDataBacks.size():" << geDataBacks.size() << endl;
//	std::vector<uint32_t> data;
//	for(uint i=500; i<= ( geDataBacks.size() < 1000? geDataBacks.size() : 1000); i++)
//	{
//		*csv_outputs << geDataBacks[i];
//		*csv_outputs << ",";
//		data.push_back(geDataBacks[i]);
//	}
//		*csv_outputs << endl;
//	isSineWave(data);
	ib = new InfluxBridge(pconfig->getString("INFLUXDB2_DATABASE.HOST"),
			pconfig->getInt("INFLUXDB2_DATABASE.PORT"), "NLh1unesRWF3KEUZp86LCKA5oB_-vM3-zhVJb7V3WB_HlK-uj6kCShHRQpjB6FUV_0Hpe3Y0Jx6HDmKjPVBLsQ==");
}

TecoRun::~TecoRun() {
	// TODO Auto-generated destructor stub
}

void TecoRun::Test()
{
	std::vector<uint32_t> geDataBack;
	std::stringstream payload;
	SetDevID(id);
	ReFleshOneCatch("X");
	ReadBufferData6000(geDataBack);
	LocalDateTime nowX;
	for(uint m=500; m<1000; m++)
	{
		payload << table_name << "-" << id;
		payload << ",AXIS=X "; //狀態描述
		payload << "value=" << geDataBack[m];
		payload << " " << nowX.utc().timestamp().epochMicroseconds() - (1000 - m);
		payload << " \n";
	}
	ib->Write(pconfig->getString("INFLUXDB2_DATABASE.ORG"), pconfig->getString("INFLUXDB2_DATABASE.BUCKET"), payload.str());
	payload.str("");


	ReFleshOneCatch("Y");
	ReadBufferData6000(geDataBack);
	LocalDateTime nowY;
	for(uint m=500; m<1000; m++)
	{
		payload << table_name << "-" << id;
		payload << ",AXIS=Y "; //狀態描述
		payload << "value=" << geDataBack[m];
		payload << " " << nowY.utc().timestamp().epochMicroseconds() - (1000 - m);
		payload << " \n";
	}

	ib->Write(pconfig->getString("INFLUXDB2_DATABASE.ORG"), pconfig->getString("INFLUXDB2_DATABASE.BUCKET"), payload.str());
	payload.str("");

	ReFleshOneCatch("Z");
	ReadBufferData6000(geDataBack);
	LocalDateTime nowZ;
	for(uint m=500; m<1000; m++)
	{
		payload << table_name << "-" << id;
		payload << ",AXIS=Z "; //狀態描述
		payload << "value=" << geDataBack[m];
		payload << " " << nowZ.utc().timestamp().epochMicroseconds() - (1000 - m);
		payload << " \n";
	}
	ib->Write(pconfig->getString("INFLUXDB2_DATABASE.ORG"), pconfig->getString("INFLUXDB2_DATABASE.BUCKET"), payload.str());
	payload.str("");
}

void TecoRun::Background(Timer& timer)
{
	std::vector<uint32_t> geDataBack;
	std::stringstream payload;

	ReFleshOneCatch("X");
	ReadBufferData6000(geDataBack);
	LocalDateTime nowX;
	for(uint m=500; m<1000; m++)
	{
		payload << table_name << "-" << id;
		payload << ",AXIS=X "; //狀態描述
		payload << "value=" << geDataBack[m];
		payload << " " << nowX.utc().timestamp().epochMicroseconds() - (1000 - m);
		payload << " \n";
	}
	ib->Write(pconfig->getString("INFLUXDB2_DATABASE.ORG"), pconfig->getString("INFLUXDB2_DATABASE.BUCKET"), payload.str());
	payload.str("");


	ReFleshOneCatch("Y");
	ReadBufferData6000(geDataBack);
	LocalDateTime nowY;
	for(uint m=500; m<1000; m++)
	{
		payload << table_name << "-" << id;
		payload << ",AXIS=Y "; //狀態描述
		payload << "value=" << geDataBack[m];
		payload << " " << nowY.utc().timestamp().epochMicroseconds() - (1000 - m);
		payload << " \n";
	}

	ib->Write(pconfig->getString("INFLUXDB2_DATABASE.ORG"), pconfig->getString("INFLUXDB2_DATABASE.BUCKET"), payload.str());
	payload.str("");

	ReFleshOneCatch("Z");
	ReadBufferData6000(geDataBack);
	LocalDateTime nowZ;
	for(uint m=500; m<1000; m++)
	{
		payload << table_name << "-" << id;
		payload << ",AXIS=Z "; //狀態描述
		payload << "value=" << geDataBack[m];
		payload << " " << nowZ.utc().timestamp().epochMicroseconds() - (1000 - m);
		payload << " \n";
	}
	ib->Write(pconfig->getString("INFLUXDB2_DATABASE.ORG"), pconfig->getString("INFLUXDB2_DATABASE.BUCKET"), payload.str());
	payload.str("");
//	for(uint i=500; i< ( geDataBack.size() < 1000? geDataBack.size() : 1000); i++)
//	{
//		*csv_outputs << geDataBack[i];
//		*csv_outputs << ",";
//	}
//	*csv_outputs << endl;
}

void TecoRun::Upload(const std::string& payload)
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

bool TecoRun::isSineWave(std::vector<uint32_t> arr)
{
	Complex test[500];

	for(uint i=0; i<500; i++)
	{
		test[i] = Complex(double(arr[i]), 0.0);
	}
    CArray data(test, 500);

    // forward fft
    fft(data);
	std::string file_name = "teco_g_fft.csv";
	csv_outputs = new std::ofstream (file_name.c_str(), std::ofstream::app);
	for(uint i=0; i<= data.size() ; i++)
	{
		*csv_outputs << std::abs(data[i]);
		*csv_outputs << ",";
	}
	*csv_outputs << endl;
    return false;
}

void TecoRun::fft(CArray& x)
{
    const size_t N = x.size();
    double PI = 3.141596;
    if (N <= 1) return;

    // divide
    CArray even = x[std::slice(0, N/2, 2)];
    CArray  odd = x[std::slice(1, N/2, 2)];

    // conquer
    fft(even);
    fft(odd);

    // combine
    for (size_t k = 0; k < N/2; ++k)
    {
        Complex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k+N/2] = even[k] - t;
    }
}
