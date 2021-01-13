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
#include <map>
#include <set>
#include <cstdio>
#include <sstream>
#include <fstream>

#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/IniFileConfiguration.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/ODBC/Connector.h"
#include "Poco/Data/SQLite/Connector.h"
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
#include "Poco/Dynamic/Var.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/StringTokenizer.h"
#include "Poco/RegularExpression.h"
#include "Poco/Foundation.h"
#include "Poco/DoubleByteEncoding.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/TextConverter.h"
#include "Poco/TextEncoding.h"
#include "Poco/Net/FTPClientSession.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/MailMessage.h"
#include "Poco/Net/SMTPClientSession.h"
#include "Poco/Net/MailRecipient.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/Socket.h"
#include "Poco/String.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/NetException.h"
#include "Poco/StringTokenizer.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/StreamCopier.h"
#include "Poco/NumberFormatter.h"
#include "Poco/NumberParser.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Format.h"
#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/IniFileConfiguration.h"
#include "Poco/LocalDateTime.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "Poco/ActiveMethod.h"
#include "Poco/ActiveResult.h"
#include "Poco/ActiveDispatcher.h"
#include "Poco/ThreadLocal.h"
#include "Poco/Stopwatch.h"
#include "Poco/Exception.h"
#include "Poco/Delegate.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/AutoPtr.h"
#include "Poco/Format.h"
#include "Poco/BinaryWriter.h"
#include "Poco/BinaryReader.h"
#include "Poco/TemporaryFile.h"
#include "Poco/FileStream.h"
#include "Poco/Buffer.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Text.h"
#include "Poco/DOM/NodeIterator.h"
#include "Poco/DOM/NodeFilter.h"
#include "Poco/DOM/AutoPtr.h"
#include "Poco/DOM/DOMWriter.h"
#include "Poco/XML/XMLWriter.h"
#include "Poco/HashTable.h"
#include "Poco/Random.h"
#include "Poco/Condition.h"
#include "Poco/Semaphore.h"
#include "Poco/Mutex.h"
#include "Poco/Timespan.h"
#include "Poco/Process.h"
#include "Poco/PipeStream.h"
#include "Poco/SharedMemory.h"
#include "Poco/CountingStream.h"
#include "Poco/NullStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/Subsystem.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/AutoPtr.h"
#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "Poco/ThreadLocal.h"
#include "Poco/ThreadPool.h"
#include "Poco/Activity.h"
#include "Poco/RunnableAdapter.h"
#include "Poco/Stopwatch.h"
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/TaskNotification.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Notification.h"
#include "Poco/Observer.h"
#include "Poco/NObserver.h"
#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/Message.h"
#include "Poco/Exception.h"
#include "Poco/File.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Nullable.h"

#include "def.h"
#include "app/DatabaseBridge.h"

using namespace std;
using namespace Poco;
using namespace Poco::Dynamic;
using namespace Poco::Util;
using namespace Poco::Net;
using namespace Poco::JSON;
using namespace Poco::Redis;
using namespace Poco::Data;
using namespace Poco::Data::Keywords;

class InfluxBridge
{
public:
	InfluxBridge(std::string host, int port, std::string _database);
	virtual ~InfluxBridge();
	bool Write(std::string org, std::string bucket, std::string payload);
private:
	Logger& logger;
	HTTPClientSession* session;
	std::string token;
};

class ArangoBridge
{
public:
	ArangoBridge(std::string host, int port, std::string _database);
	virtual ~ArangoBridge();
	void FailOver(std::string _host, int _port);
	JSON::Object::Ptr Bridge(std::string method, std::string path, JSON::Object paylod);
private:
	Logger& logger;
	HTTPClientSession* session;
	std::string token;
	std::string database;
	Timer* TokenTimer;
	bool activeFailover;
	std::vector<URI> cluster_nodes;
	uint _i;
protected:
	void getToken(Timer& timer);
};

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
