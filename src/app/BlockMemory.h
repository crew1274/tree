/*
 * BlockMemory.h
 *
 *  Created on: 2018年4月26日
 *      Author: 171104
 */

#ifndef SRC_APP_BLOCKMEMORY_H_
#define SRC_APP_BLOCKMEMORY_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <numeric>
#include <complex>
#include <iostream>
#include <valarray>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include "Poco/RunnableAdapter.h"
#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "Poco/ThreadLocal.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/ParseHandler.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/Environment.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Stopwatch.h"
#include "Poco/Dynamic/Var.h"

#include "Poco/ActiveMethod.h"
#include "Poco/ActiveResult.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/IniFileConfiguration.h"
#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "Poco/ThreadLocal.h"
#include "Poco/ThreadPool.h"
#include "Poco/Stopwatch.h"
#include "Poco/Logger.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/AutoPtr.h"

#include "app/algo/SVM.h"
#include "app/algo/Model.h"
#include "app/algo/frequency.h"
#include "app/algo/vibration.h"
#include "app/algo/window.h"
#include "../utility.h"
#include "../def.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;
using namespace Poco::JSON;

// ----------------------------------------------------------
//                  BlockMemory address
// ----------------------------------------------------------
#define brame_1 0x40000000
#define brame_2 0x42000000
#define brame_3 0x44000000
#define brame_4 0x46000000

#define brame_5 0X48000000
// ----------------------------------------------------------
//                  BlockMemory size
// ----------------------------------------------------------
#define bram_size 0X10000

#define channel_size 1024

typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;

typedef enum
{
	none = -1,
	thermistor = 0,
	axis = 1,
	current_5A = 2,
	current_10A =3,
	current_15A = 4,
	current_20A = 5,
	current_30A = 6,
	current_60A = 7,
	DC_25A = 8,
	DCCurrent = 9,
	Conductivity = 10,
	Manual = 11
}CalculateType;

typedef enum
{
	JsonFeature = 0,
	ControlChart = 1
}Feature;

struct json_feature
{
	// for temperature
	float DQI_x_T_U;

	// for ADC
	float diff_max;
	float diff_min;
	float diff_range;
	float roughness_max;
	float roughness_min;
	float roughness_range;
	std::vector<std::vector<float> > support_vectors_;
	float gamma;
	std::vector<std::vector<float> > alpha;
	std::vector<float> intercept_;
	int nSV;
	float threshold;
};

struct adc_struct
{
	CalculateType type;
	int channel;
	json_feature feature;
	float upper;
	float lower;
};

class BlockMemory
{
    public:
        BlockMemory(LayeredConfiguration *_config);

        ~BlockMemory();

        template <class T>
        float Convet(T value, CalculateType type);

        void Load(std::string fileName); //Load ADC description JSON

        void LongTimeRecord(Timer& timer); //record ADC sensor

        void Collector(Timer& timer); //record ADC sensor

        void Upload(const std::string& payload);

        void Upload_2();

        float GetAverageValue(int channel, CalculateType Type);

        void GetMultiValue(Poco::Timer& timer);

        void PHM(Poco::Timer& timer);

        void PHM_ControlChart(Timer& timer);
        //float ConvertADCValue(float value, CalculateType Type);

        //static std::map<uint, uint> IndexOfPLC(std::vector<uint> channel, MemoryAreaTable current, MemoryAreaTable plc);
        std::vector<adc_struct> ADC_sensors;

        bool isWave(std::vector<float> arr, uint length, uint diff);

        void IntervalCollect(Timer& timer);

        void generateSinWave();

        void setIntervalCollectTime(int time);
        int getIntervalCollectTime();

        void setIntervalCollectDuration(int time);
        int getIntervalCollectDuration();

    private:
    	LayeredConfiguration *pconfig; // configuration
    	Logger& logger;
        ActiveMethod<void, std::string, BlockMemory> _ActiveMethod;
    	RunnableAdapter<BlockMemory> runnableUpload;
    	SVM svm;
    	int PHM_CHANNEL;
    	int IntervalCollectTime;
    	int IntervalCollectDuration;
    	uint OneTimeCollect;
    	uint CHANNEL_SIZE;
    	uint PHM_SIZE;
    	json_feature ParesJsonFeature(CalculateType type, int port, bool verbose); // parse model feature JSON
    	std::vector<std::vector<float> > Parse2DFloatArray(Poco::JSON::Object::Ptr json_ptr, std::string key, bool verbose);
    	std::vector<float> Parse1DFloatArray(Poco::JSON::Object::Ptr json_ptr, std::string key, bool verbose);

    	off_t start_address;
        union
    	{
        	u32 u;
        	float f;
        }un[channel_size];
        //std::vector<float> arr;
        uint counter;
        u32* bram32_ptr;
        u32 bram32_block[bram_size/4];
        std::vector<std::vector<double> > sinWaveSet;
        int fd;
        uint index;
        //std::string payload;
        std::stringstream payload;
    	std::string DEVICE_ID;
    	std::string DEVICE_IP;
    	std::map<CalculateType, float> CurrentThreshold;
    	std::vector<ofstream*> outputs;
    	std::vector<bool> pre_status;
    	std::vector<bool> pre_pre_status;
        //vector<vector<float> > Parse2DFloatArray(Document& document, const char* key, bool verbose);
    	//unsigned long long GetTimeStamp();
    	void fft(CArray& x);
    	bool isSineWave(std::vector<float> arr, int threshold);
};
//float ConvertADCValue(float value, CalculateType Type);
#endif /* SRC_APP_BLOCKMEMORY_H_ */
