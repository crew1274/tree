/*
 * DatabaseBridge.h
 *
 *  Created on: 2018年5月22日
 *      Author: 171104
 */

#ifndef SRC_APP_DATABASEBRIDGE_H_
#define SRC_APP_DATABASEBRIDGE_H_

#include <iostream>
#include <string>
#include <cstdio>
#include <sstream>
#include <fstream>
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/IniFileConfiguration.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/ODBC/Connector.h"
#include "Poco/Data/MySQL/MySQLException.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Exception.h"
#include "Poco/Delegate.h"
#include "Poco/Thread.h"
#include "Poco/Redis/AsyncReader.h"
#include "Poco/Redis/Command.h"
#include "Poco/Redis/PoolableConnectionFactory.h"
#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "Poco/ThreadLocal.h"
#include "Poco/Stopwatch.h"
#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/Message.h"
#include "Poco/LocalDateTime.h"
#include "Poco/Timespan.h"

#include "app/DatabaseBridge.h"


using namespace std;
using namespace Poco;
using namespace Poco::Data;
using namespace Poco::Util;
using namespace Poco::Redis;

class RedisBridge
{
public:
	RedisBridge(std::string host, int port);
	virtual ~RedisBridge();
	void expire(std::string key, std::string time);
	void cmd(std::vector<std::string> cmds);
	bool set(std::string key, std::string value);
	std::string get(std::string key);
	bool del(std::string key);

	bool sadd(std::string table, std::string key); //新增key
	bool sismember(std::string table, std::string key); //檢查key是否存在
	bool smembers(std::string table, std::vector<std::string> &keys); //擷取所有的value
	bool srem(std::string table, std::string key); //刪除key

	bool hmset(std::string table, std::map<std::string, std::string > fields);
	bool hgetall(std::string table, std::map<std::string, std::string> &fields);
	template<typename T> T cmd_exec(Redis::Array cmd);
private:
	Logger& logger;
	std::string _host;
	int _port;
	AutoPtr<IniFileConfiguration> pconfig;
	Redis::Client _redis;
	Mutex mutex;
protected:
};

#endif /* SRC_APP_DATABASEBRIDGE_H_ */
