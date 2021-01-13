/*
 * TecoRun.h
 *
 *  Created on: 2020年12月17日
 *      Author: 171104
 */

#ifndef SRC_APP_TECORUN_H_
#define SRC_APP_TECORUN_H_

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

#include "hal/TecoRS485.h"
#include "DatabaseBridge.h"
#include "../utility.h"
#include "../def.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;
using namespace Poco::JSON;

typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;

class TecoRun  : private TecoRS485
{
public:
	TecoRun(const char *uart_device, int _id, LayeredConfiguration *_config, std::string _table_name);
	virtual ~TecoRun();
	void Background(Timer& timer);
	void Upload(const std::string& payload);
	bool isSineWave(std::vector<uint32_t> arr);
	void fft(CArray& x);
	void Test();
private:
	int id;
	LayeredConfiguration *pconfig; // configuration
	Logger& logger;
	ActiveMethod<void, std::string, TecoRun> _ActiveMethod;
	TecoVibrationStruct dataContainer;
	ofstream* csv_outputs;
	std::string table_name;
	InfluxBridge* ib;
};

#endif /* SRC_APP_TECORUN_H_ */
