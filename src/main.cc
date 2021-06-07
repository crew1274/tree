#include <iostream>
#include <sstream>
#include <istream>
#include <cstdio>
#include <cstdlib>

#include "Poco/Foundation.h"
#include "Poco/DoubleByteEncoding.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/TextConverter.h"
#include "Poco/TextEncoding.h"
#include "Poco/StringTokenizer.h"
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
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/NetException.h"

#include "Poco/JSON/Stringifier.h"
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
#include "Poco/String.h"

#include "app/BlockMemory.h"
#include "app/ModbusRun.h"
#include "app/TecoRun.h"

using namespace std;
using namespace Poco;
using namespace Poco::JSON;
using namespace Poco::Util;
using namespace Poco::Net;

#if 1

class PublicRequestHandler: public HTTPRequestHandler
{
public:
	PublicRequestHandler(Path _target): logger(Logger::get("PublicRequestHandler")), target(_target){}
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		if(target.getExtension() == "html")
		{
			response.setContentType("text/html");
		}
		else if(target.getExtension() == "css")
		{
			response.setContentType("text/css");
		}
		else if(target.getExtension() == "js")
		{
			response.setContentType("application/javascript");
		}
		else if(target.getExtension() == "icon")
		{
			response.setContentType("image/x-icon");
		}
		else
		{
			response.setContentType("text/plain");
		}
		response.set("Access-Control-Allow-Origin", "*");
		response.setVersion(HTTPMessage::HTTP_1_1);
		response.setChunkedTransferEncoding(true);
		std::ostream& ostr = response.send();
		std::ifstream is(target.toString(Path::PATH_UNIX).c_str(), std::ios::binary);
		std::string content( (std::istreambuf_iterator<char>(is) ),(std::istreambuf_iterator<char>()));
		ostr << content;
		return;
	}
private:
	Logger& logger;
	Path target;
};

class RequestHandlerFactory: public HTTPRequestHandlerFactory // route controller
{
public:
	RequestHandlerFactory(NotificationCenter* _nc, LayeredConfiguration *_config):logger(Logger::get("Router")), nc(_nc), config(_config){}
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
//		logger.trace("Request from: %s to %s%s", request.clientAddress().toString(), request.getHost(), request.getURI());
		URI RequestPath(request.getURI());
		std::vector<std::string> PathSegments;
		RequestPath.getPathSegments(PathSegments);
		Path ErrorPage(config->getString("application.dir")+"/public/error.html", Path::PATH_UNIX);

		if(PathSegments.size() == 0)
		{
			return new PublicRequestHandler(ErrorPage.toString(Path::PATH_UNIX));
		}
		else if(PathSegments[0] == "fonts" || PathSegments[0] == "css"
				|| PathSegments[0] == "js" || PathSegments[0] == "img"
				|| PathSegments[0] == "index.html" || PathSegments[0] == "favicon.ico")
		{
			Path target(config->getString("application.dir"), Path::PATH_UNIX);
			target.pushDirectory("dist");
			for(uint i=0; i<PathSegments.size(); i++)
			{
				if(i == PathSegments.size()-1)
				{
					target.setFileName(PathSegments[i]);
				}
				else
				{
					target.pushDirectory(PathSegments[i]);
				}
			}
			try
			{
				File f(target);
				if(!f.exists() || !f.isFile())
				{
					logger.error("Can't find %s", target.toString(Path::PATH_UNIX));
					target = ErrorPage;
				}
			}
			catch(Exception& e)
			{
				logger.error(e.displayText());
				target = ErrorPage;
			}
			return new PublicRequestHandler(target);
		}
		else if(PathSegments[0] == "public")
		{
			Path target(config->getString("application.dir"), Path::PATH_UNIX);
			for(uint i=0; i<PathSegments.size(); i++)
			{
				if(i == PathSegments.size()-1)
				{
					target.setFileName(PathSegments[i]);
				}
				else
				{
					target.pushDirectory(PathSegments[i]);
				}
			}
			try
			{
				File f(target);
				if(!f.exists() || !f.isFile())
				{
					logger.error("Can't find %s", target.toString(Path::PATH_UNIX));
					target = ErrorPage;
				}
			}
			catch(Exception& e)
			{
				logger.error(e.displayText());
				target = ErrorPage;
			}
			return new PublicRequestHandler(target);
		}
		else
		{
			logger.error("Undefined Request");
			return new PublicRequestHandler(ErrorPage.toString(Path::PATH_UNIX));
		}
	}
private:
	Logger& logger;
	NotificationCenter *nc;
	LayeredConfiguration *config;
};

class ADC: public ServerApplication
{
public:
	ADC():_helpRequested(false){}
	~ADC(){}
protected:
	void initialize(Application& self)
	{
		ServerApplication::loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
		if(_helpRequested)
		{
			terminate();
		}
		std::ofstream fs((config().getString("application.dir")+"/ADC.pid").c_str(), std::ofstream::trunc);
		fs << getpid();
		// create logger
		AutoPtr<SplitterChannel> DoubleChannel(new SplitterChannel());
		Path current(Path::current(), Path::PATH_UNIX);
		current = current.pushDirectory("ADC_log");
		File log_folder(current);
		try
		{
			log_folder.createDirectory();
			current.append("log");
		}
		catch(Poco::Exception& e)
		{
			cerr << e.displayText() << endl;
		}
		//File Channel
		AutoPtr<FileChannel> FileConsole(new FileChannel(current.toString(Path::PATH_UNIX)));
		FileConsole ->setProperty("rotation", "daily");
		FileConsole ->setProperty("archive", "timestamp");
		FileConsole ->setProperty("compress", "true");
		FileConsole ->setProperty("rotateOnOpen", "false");
		FileConsole ->setProperty("times", "local");
		//Console Channel
		AutoPtr<ColorConsoleChannel> ColorConsole(new ColorConsoleChannel);
		ColorConsole->setProperty("traceColor", "blue");
		ColorConsole->setProperty("debugColor", "white");
		ColorConsole->setProperty("informationColor", "green");
		ColorConsole->setProperty("noticeColor", "cyan");
		ColorConsole->setProperty("warningColor", "yellow");
		ColorConsole->setProperty("errorColor", "red");
		ColorConsole->setProperty("criticalColor", "lightMagenta");
		ColorConsole->setProperty("fatalColor", "magenta");

		DoubleChannel -> addChannel(FileConsole);
		DoubleChannel -> addChannel(ColorConsole);

		AutoPtr<PatternFormatter> Channel_Formatter(new PatternFormatter);
		Channel_Formatter->setProperty("pattern", "[%Y-%m-%d %H:%M:%S][%s][%p] %t");
		Channel_Formatter->setProperty("times", "local");

		Logger::root().setChannel(new FormattingChannel(Channel_Formatter, DoubleChannel));
		Logger::root().setLevel(Message::PRIO_TRACE);
		Logger& logger = Logger::get("Initialize");
		logger.information("Process Initialize");
		Thread::sleep(config().getInt("PROGRAM.SLEEP", 0));
		adc = new BlockMemory(&config());
		adc->Load(config().getString("ADC.DESCRIPTION_FILE"));

		nc = new NotificationCenter;
		nc->addObserver(Observer<ADC, Notification>(*this, &ADC::handleReload));
		ThreadPool::defaultPool().addCapacity(32); //Set max threa d of ThreadPool for TCPServerParams

		if(config().getBool("TECO.START"))
		{
			// 切割config檔
			StringTokenizer path_array(config().getString("TECO.PATH_ARRAY"), ",", StringTokenizer::TOK_TRIM);
			StringTokenizer id_array(config().getString("TECO.ID_ARRAY"), ",", StringTokenizer::TOK_TRIM);
			StringTokenizer delay_array(config().getString("TECO.DELAY_TIME"), ",", StringTokenizer::TOK_TRIM);
			for(uint i=0; i<config().getUInt("TECO.NUMBER_OF_OBJECT"); i++)
			{
				trs.push_back(  std::make_pair(
						new TecoRun(path_array[i].c_str(), NumberParser::parse(id_array[i]), &config(), path_array[i]),
						new	Timer(0, NumberParser::parse(delay_array[i]))
				));
			}
			for(uint i=0; i<config().getUInt("TECO.NUMBER_OF_OBJECT"); i++)
			{
				logger.information("啟動TECO COLLECT每%d秒執行一次", (NumberParser::parse(delay_array[i])/1000));
				trs[i].second->start(TimerCallback<TecoRun>(*trs[i].first, &TecoRun::Background));
			}
		}
	}

	void uninitialize()
	{
		// add your own uninitialization code here
		ServerApplication::uninitialize();
	}

	void reinitialize(Application& self)
	{
		ServerApplication::reinitialize(self);
		// add your own reinitialization code here
	}

	int main(const ArgVec& args)
	{
		Logger& logger = Logger::get("Main");

		Timer Modbus_Timer(0, config().getInt("MODBUS.DELAY_TIME", 10000));
		if(config().getBool("MODBUS.START", false))
		{
			mr = new ModbusRun(UART_PL4, &config());
			logger.information("啟動MODBUS COLLECT每%d秒執行一次", (config().getInt("MODBUS.DELAY_TIME", 10000)/1000));
			Modbus_Timer.start(TimerCallback<ModbusRun>(*mr, &ModbusRun::Background));
		}

//		Timer Teco_Timer(0, config().getInt("TECO.DELAY_TIME", 10000));
//		Timer Teco2_Timer(0, config().getInt("TECO.DELAY_TIME", 10000));
//		if(config().getBool("TECO.START", false))
//		{
//			tr = new TecoRun(UART_PL4, 15, &config(), "UART_PL4");
//			logger.information("啟動TECO COLLECT每%d秒執行一次", (config().getInt("TECO.DELAY_TIME")/1000));
//			Teco_Timer.start(TimerCallback<TecoRun>(*tr, &TecoRun::Background));
//
//			tr_2 = new TecoRun(UART_PL5, 15, &config(), "UART_PL5");
//			logger.information("啟動TECO COLLECT每%d秒執行一次", (config().getInt("TECO.DELAY_TIME")/1000));
//			Teco2_Timer.start(TimerCallback<TecoRun>(*tr_2, &TecoRun::Background));
//		}

		Timer COLLECT_Timer(0, config().getInt("COLLECT.DELAY_TIME", 10000));
		Timer IntervalCollect_Timer(0, config().getInt("INTERVAL_COLLECT.DELAY_TIME", 10000));
		Timer PHM_Timer(0, config().getInt("PHM.PHM_DELAY_TIME", 10000));
		if(config().getBool("COLLECT.START"))
		{
			logger.information("啟動ADC COLLECT每%d秒執行一次", (config().getInt("COLLECT.DELAY_TIME", 10000)/1000));
			COLLECT_Timer.start(TimerCallback<BlockMemory>(*adc, &BlockMemory::Collector));
		}
		if(config().getBool("INTERVAL_COLLECT.START"))
		{
			logger.information("啟動ADC INTERVAL COLLECT，每%d秒執行一次，持續%d秒",
					(config().getInt("INTERVAL_COLLECT.DELAY_TIME", 10000)/1000),
					(config().getInt("INTERVAL_COLLECT.DURATION", 10000)/1000));
			adc->setIntervalCollectTime(config().getInt("INTERVAL_COLLECT.TIME"));
			adc->setIntervalCollectDuration(config().getInt("INTERVAL_COLLECT.DURATION"));
			IntervalCollect_Timer.start(TimerCallback<BlockMemory>(*adc, &BlockMemory::IntervalCollect));
		}

		if(config().getBool("PHM.START"))
		{
			logger.information("啟動ADC PHM，每%d秒執行一次", (config().getInt("ADC.PHM_DELAY_TIME", 10000)/1000));
			PHM_Timer.start(TimerCallback<BlockMemory>(*adc, &BlockMemory::PHM));
		}

		//set HTTP Server Params
		HTTPServerParams* Params = new HTTPServerParams;
		Params->setSoftwareVersion("ADC/1.0");
		RequestHandlerFactory RouterHandler(nc, &config());
		// set-up a server socket
		//ServerSocket svs(SocketAddress(AddressFamily::IPv4, "127.0.0.1", 9998));
		ServerSocket svs(SocketAddress(AddressFamily::IPv4, config().getString("DEVICE.HOST", "0.0.0.0"), config().getInt("DEVICE.PORT", 9999)));
		// set-up a HTTPServer instance
		HTTPServer srv(&RouterHandler, ThreadPool::defaultPool(), svs, Params);
		// start the HTTPServer
		srv.start();
		// wait for CTRL-C or kill
		waitForTerminationRequest();
		// Stop the HTTPServer
		srv.stop();

		return Application::EXIT_OK;
	}

	void handleReload(Notification* pNf)
	{
		std::cout << "handleBase referenceCount: " << pNf->referenceCount() << std::endl;
		logger().information("Configuration Reload");
		ServerApplication::loadConfiguration();
		pNf->release(); // we got ownership, so we must release
	}

	void defineOptions(OptionSet& options)
	{
		// Define Options
		Application::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
				.required(false)
				.repeatable(false));

		options.addOption(
			Option("define", "d", "define a configuration property")
				.required(false)
				.repeatable(true)
				.argument("name=value"));

		options.addOption(
			Option("config-file", "f", "load configuration data from a file")
				.required(false)
				.repeatable(true)
				.argument("file"));
	}

	void handleOption(const std::string& name, const std::string& value)
	{
		ServerApplication::handleOption(name, value);

		if(name == "help")
		{
			_helpRequested = true;
			HelpFormatter helpFormatter(options());
			helpFormatter.setCommand(commandPath());
			helpFormatter.setUsage("OPTIONS");
			helpFormatter.setHeader("ADC資料處理");
			helpFormatter.setFooter("包含資料蒐集及異常偵測");
			helpFormatter.format(std::cout);
			stopOptionsProcessing();
		}
		else if(name == "define")
		{
			defineProperty(value);
			config().setString(name, value);
		}
		else if(name == "config-file")
		{
			loadConfiguration(value);
		}
	}

	void defineProperty(const std::string& def)
	{
		std::string name;
		std::string value;
		std::string::size_type pos = def.find('=');
		if (pos != std::string::npos)
		{
			name.assign(def, 0, pos);
			value.assign(def, pos + 1, def.length() - pos);
		}
		else name = def;
		config().setString(name, value);
	}

	void printProperties(const std::string& base)
	{
		AbstractConfiguration::Keys keys;
		config().keys(base, keys);
		if (keys.empty())
		{
			if (config().hasProperty(base))
			{
				std::string msg;
				msg.append(base);
				msg.append(" = ");
				msg.append(config().getString(base));
				//print on console
				logger().information(msg);
			}
		}
		else
		{
			for (AbstractConfiguration::Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
			{
				std::string fullKey = base;
				if (!fullKey.empty()) fullKey += '.';
				fullKey.append(*it);
				printProperties(fullKey);
 			}
		}
	}

private:
	Thread runnableProduciton;
	NotificationCenter *nc;
	BlockMemory *adc;
	ModbusRun *mr;
	std::vector< std::pair<TecoRun*, Timer*> > trs;
	TecoRun *tr;
	TecoRun *tr_2;
	InfluxBridge* ib;
	bool _helpRequested;
};

POCO_SERVER_MAIN(ADC)

#endif
