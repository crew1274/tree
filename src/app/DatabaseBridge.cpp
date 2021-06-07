/*
 * DatabaseBridge.cpp
 *
 *  Created on: 2018年5月22日
 *      Author: 171104
 */

#include <app/DatabaseBridge.h>

//----------------------------------------------------------------------------------------------//
InfluxBridge::InfluxBridge(std::string _host, int _port, std::string _token):
logger(Logger::get("InfluxBridge")), token(_token)
{
	session = new HTTPClientSession( _host, _port); //建立HTTP session
}

InfluxBridge::~InfluxBridge(){}

bool InfluxBridge::Write(std::string org, std::string bucket, std::string payload)
{
	std::string path = "/api/v2/write?org=" + org + "&bucket=" + bucket + "&precision=us";
	HTTPRequest request(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
	request.add("Authorization","Token " +  token);
//	request.setContentType("application/x-www-form-urlencoded");
	request.setContentLength(payload.length());
	std::ostream& BodyOstream = session->sendRequest(request); // sends request, returns open stream
	BodyOstream << payload;
	HTTPResponse response;
	istream& rs = session->receiveResponse(response);
	string s((istreambuf_iterator<char>(rs)), istreambuf_iterator<char>());
	// return status parse
	cout << s << endl;
	return true;
}


//----------------------------------------------------------------------------------------------//
ArangoBridge::ArangoBridge(std::string _host, int _port, std::string _database):
logger(Logger::get("ArangoBridge")), token(""), database(_database), activeFailover(false)
{
	session = new HTTPClientSession(_host, _port); //建立HTTP session
	TokenTimer = new Timer(0, 1000 * 60 * 5); //初始化Timer，每5分鐘更新一次token
	TokenTimer->start(TimerCallback<ArangoBridge>(*this, &ArangoBridge::getToken));
//	try /*取得clusters*/
//	{
//		JSON::Object EmptyObject;
//		JSON::Object::Ptr cluster = this->Bridge(HTTPRequest::HTTP_GET, "/_api/cluster/endpoints", EmptyObject);
//		JSON::Array::Ptr clusters = cluster->getArray("endpoints");
//		for(uint i=0; i<clusters->size(); i++)
//		{
//			URI node_path(clusters->getObject(i)->get("endpoint").convert<std::string>());
//			cluster_nodes.push_back(node_path);
//		}
//		logger.information("啟動容錯轉移");
//		activeFailover = true;
//		_i = 0;
//	}
//    catch (Exception &e)
//    {
//        logger.error(e.displayText());
//    }
}

ArangoBridge::~ArangoBridge(){}

void ArangoBridge::FailOver(std::string _host, int _port)
{
	session = new HTTPClientSession(_host, _port); //重新建立HTTP session
	logger.information("重新建立HTTP session: %s:%d", _host, _port);
}

JSON::Object::Ptr ArangoBridge::Bridge(std::string method, std::string path, JSON::Object paylod)
{
	HTTPRequest request(method, path, HTTPMessage::HTTP_1_1);
	request.add("Authorization", token);
	request.setContentType("application/json");
	if(method != "HTTP_POST")
	{
		std::stringstream ss;
		paylod.stringify(ss);
		request.setContentLength(ss.str().size());
	}
	std::ostream& BodyOstream = session->sendRequest(request); // sends request, returns open stream
	paylod.stringify(BodyOstream);

	HTTPResponse response;
	istream& rs = session->receiveResponse(response);
	string s((istreambuf_iterator<char>(rs)), istreambuf_iterator<char>());
	JSON::Parser parser;
	return parser.parse(s).extract<JSON::Object::Ptr>();
}

void ArangoBridge::getToken(Timer& timer)
{
	try
	{
		JSON::Object paylod;
		paylod.set("username", "root");
		paylod.set("password", "root");
		JSON::Object::Ptr ret = this->Bridge(HTTPRequest::HTTP_POST, "/_open/auth", paylod);
		token = "Bearer " + ret->get("jwt").convert<std::string>();
	}
    catch (Exception &e)
    {
        logger.error(e.displayText());
        if(this->activeFailover)
        {
        	_i != cluster_nodes.size() - 1 ? _i ++ : _i = 0;
        	this->FailOver( cluster_nodes[_i].getHost(), cluster_nodes[_i].getPort()) ;
        }
    }
}

//------------------------------------------------------------------------------------------------//

//-----------------------------------------Redis--------------------------------------------------//
RedisBridge::RedisBridge(std::string host, int port): logger(Logger::get("RedisBridge")), _host(host), _port(port)
{
	// TODO Auto-generated constructor stub
	try
	{
		Poco::Timespan t(10, 0);
		_redis.connect(_host, _port, t);
		logger.information("Test connect redis server [%s:%d]", _host, _port);
	}
	catch(RedisException& e)
	{
		logger.error(e.message());
	}
}
RedisBridge::~RedisBridge(){
	// TODO Auto-generated destructor stub
}

void RedisBridge::expire(std::string key, std::string time)
{
	Redis::Array cmd;
	cmd << "EXPIRE";
	cmd << key;
	cmd << time;
	_redis.execute<std::string>(cmd);
}


void RedisBridge::cmd(std::vector<std::string> cmds)
{
	Redis::Array cmd;
	for(uint i=0; i< cmds.size(); i++)
	{
		cmd << cmds[i];
	}
	_redis.execute<Poco::Int64>(cmd);
}

bool RedisBridge::set(std::string key, std::string value)
{
	Command set = Command::set(key, value);
	try
	{
		mutex.lock();
		if(_redis.execute<std::string>(set) == "OK")
		{
//			logger.information("SET <%s> -> <%s>", key, value);
			mutex.unlock();
			return true;
		}
	}
	catch (RedisException& e)
	{
		logger.error(e.message());
	}
	mutex.unlock();
	return false;
}

bool RedisBridge::del(std::string key)
{
	Command del = Command::del(key);
	try
	{
		if(_redis.execute<Poco::Int64>(del) == 1)
		{
			logger.information("DEL <%s>", key);
			return true;
		}
	}
	catch (RedisException& e)
	{
		logger.error(e.message());
	}
	return false;
}

bool RedisBridge::srem(std::string table, std::string key)
{
	Command srem = Command::srem(table, key);
	try
	{
		if(_redis.execute<Poco::Int64>(srem) == 1)
		{
			return true;
		}
	}
	catch(RedisException& e)
	{
		logger.error(e.message());
	}
	return false;
}

bool RedisBridge::sadd(std::string table, std::string key)
{
	Command sadd = Command::sadd(table, key);
	try
	{
		if(_redis.execute<Poco::Int64>(sadd) == 1)
		{
			return true;
		}
	}
	catch (RedisException& e)
	{
		logger.error(e.message());
	}
	return false;
}

bool RedisBridge::sismember(std::string table, std::string key)
{
	Command sismember = Command::sismember(table, key); // check
	try
	{
		Poco::Int64 result = _redis.execute<Poco::Int64>(sismember);
		if(result == 0)//not exist
		{
			return false;
		}
		else if(result == 1) //is exist
		{
			return true;
		}
	}
	catch (RedisException& e)
	{
		logger.error(e.message());
	}
	return false;
}

bool RedisBridge::smembers(std::string table, std::vector<std::string> &keys)
{
	Command smembers = Command::smembers(table); // check
	mutex.lock();
	try
	{
		Redis::Array result = _redis.execute<Redis::Array>(smembers);
		if(result.isNull())
		{
			logger.error("Table %s is Null from Redis", table);
		}
		else
		{
			keys.clear();
			for(uint i=0; i< result.size(); i++)
			{
				keys.push_back(result.get<BulkString>(i).value());
			}
			mutex.unlock();
			return true;
		}
	}
	catch (RedisException& e)
	{
		logger.error(e.message());
	}
	mutex.unlock();
	return false;
}

bool RedisBridge::hmset(std::string table, std::map<std::string, std::string > fields)
{
	Command hmset = Command::hmset(table, fields);
	mutex.lock();
	try
	{
		if(_redis.execute<std::string>(hmset) == "OK");
		{
			logger.information("HMSET <%s> OK", table);
			mutex.unlock();
			return true;
		}
	}
	catch (RedisException& e)
	{
		logger.error(e.message());
	}
	mutex.unlock();
	return false;
}

bool RedisBridge::hgetall(std::string table, std::map<std::string, std::string> &fields)
{
	Command hgetall = Command::hgetall(table);
	mutex.lock();
	try
	{
		Redis::Array result = _redis.execute<Redis::Array>(hgetall);
		if(result.isNull())
		{
			logger.error("Key %s is Null from Redis", table);
			mutex.unlock();
			return false;
		}
		else
		{
			for(uint i=0; i< result.size(); i+=2)
			{
				if(result.get<Redis::BulkString>(i).value() == "2445")
				{
					fields["2446"] = result.get<Redis::BulkString>(i+1).value();
				}
				else
				{
					fields[result.get<BulkString>(i).value()] = result.get<BulkString>(i+1).value();
				}
			}
			mutex.unlock();
			return true;
		}
	}
	catch (RedisException& e)
	{
		logger.error(e.message());
	}
	mutex.unlock();
	return false;
}

std::string RedisBridge::get(std::string key)
{
	std::string value = "";
	try
	{
		mutex.lock();
		BulkString result = _redis.execute<BulkString>(Command::get(key));
		if(result.isNull())
		{
			logger.error("Key %s is Null from Redis", key);
		}
		else
		{
			value = result.value();
		}
		mutex.unlock();
	}
	catch (Exception& e)
	{
		logger.error(e.message());
	}
	return value;
}
//----------------------------------------------------------------------------------------------//

