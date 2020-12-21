/*
 * TecoRun.cpp
 *
 *  Created on: 2020年12月17日
 *      Author: 171104
 */

#include <app/TecoRun.h>

TecoRun::TecoRun(const char *uart_device, LayeredConfiguration *_config):
TecoRS485(uart_device),
pconfig(_config),
logger(Logger::get("ModbusRun")),
_ActiveMethod(this, &TecoRun::Upload)
{
	// TODO Auto-generated constructor stub
	SetDevID(7);
	uint32_t geDataBack;
	Read_FW_Version(geDataBack);
	printf("FW VERSION: %d \n",(geDataBack));
	Read_Core_Temp(geDataBack);
	printf("%d /n",(geDataBack));
	ReFleshOneCatch("X");
//	std::vector<uint32_t> geDataBacks;
//	ReadBufferData6000(geDataBacks);
//	logger.information("%u ", geDataBacks[500]);
//	std::string file_name = "teco_g.csv";
//	csv_outputs = new std::ofstream (file_name.c_str(), std::ofstream::app);
	std::vector<uint32_t> geDataBacks;
	ReadBufferData6000(geDataBacks);
	cout << "geDataBacks.size():" << geDataBacks.size() << endl;
	std::vector<uint32_t> data;
	for(uint i=500; i<= ( geDataBacks.size() < 1000? geDataBacks.size() : 1000); i++)
	{
//		*csv_outputs << geDataBacks[i];
//		*csv_outputs << ",";
		data.push_back(geDataBacks[i]);
	}
	//	*csv_outputs << endl;
	isSineWave(data);
}

TecoRun::~TecoRun() {
	// TODO Auto-generated destructor stub
}

void TecoRun::Background(Timer& timer)
{
	std::vector<uint32_t> geDataBack;
	std::stringstream payload;

	ReFleshOneCatch("X");
	ReadBufferData6000(geDataBack);

	payload << "TECO_X"; //目標資料表
	payload << ",utc=+8 "; //狀態描述
	for(uint m=0; m<1022; m++)
	{
		payload << m << "=" << geDataBack[m];
		if(m != 1021)
		{
			payload << ",";
		}
	}
	payload << endl;

	ReFleshOneCatch("Y");
	ReadBufferData6000(geDataBack);
	payload << "TECO_Y"; //目標資料表
	payload << ",utc=+8 "; //狀態描述
	for(uint m=0; m<1022; m++)
	{
		payload << m << "=" << geDataBack[m];
		if(m != 1021)
		{
			payload << ",";
		}
	}
	payload << endl;

	ReFleshOneCatch("Z");
	ReadBufferData6000(geDataBack);
	payload << "TECO_Z"; //目標資料表
	payload << ",utc=+8 "; //狀態描述
	for(uint m=0; m<1022; m++)
	{
		payload << m << "=" << geDataBack[m];
		if(m != 1021)
		{
			payload << ",";
		}
	}
	payload << endl;
	this->_ActiveMethod(payload.str());


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
        x[k    ] = even[k] + t;
        x[k+N/2] = even[k] - t;
    }
}
