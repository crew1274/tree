/*
 * BlockMemory.cpp
 *
 *  Created on: 2018年4月26日
 *      Author: 171104
 */

#include "BlockMemory.h"

BlockMemory::BlockMemory(LayeredConfiguration *_config):
pconfig(_config),
logger(Logger::get("BlockMemory")),
_ActiveMethod(this, &BlockMemory::Upload),
runnableUpload(*this, &BlockMemory::Upload_2)
{
	CurrentThreshold[current_5A] = 1.66;
	CurrentThreshold[current_10A] = 3.33;
	CurrentThreshold[current_15A] = 5;
	CurrentThreshold[current_20A] = 6.66;
	CurrentThreshold[current_30A] = 10;
	CurrentThreshold[current_60A] = 20;
	OneTimeCollect = pconfig->getUInt("ADC.COLLECT_NUMBER");
	CHANNEL_SIZE = pconfig->getUInt("ADC.CHANNEL_SIZE");
	if(pconfig->getBool("ADC.INTO_REDIS"))
	{
		rb = new RedisBridge("127.0.0.1", 6379);
	}
}

BlockMemory::~BlockMemory(){}

void BlockMemory::Load(std::string fileName)
{
	adc_struct adc_struct_temp;
	Object::Ptr object_temp;
	Path filePath(Path::current(), fileName);
	logger.information("Open %s ", filePath.toString());
	std::ostringstream ostr;
	//open file
	if (filePath.isFile())
	{
		File inputFile(filePath);
		if(inputFile.exists())
		{
			FileInputStream fis(filePath.toString());
			StreamCopier::copyStream(fis, ostr);
			logger.information("Load file:%s to FileInputStream.", filePath.toString());
		}
		else
		{
			logger.fatal("%s do not exist!", filePath.toString());
		}
	}
	else
	{
		logger.fatal("%s do not exist!", filePath.toString());
	}
	logger.trace("Total of %z bytes", ostr.str().size());

	Parser str_parser;
	Dynamic::Var DynamicResult = str_parser.parse(ostr.str());

	//Extract array pointer
	JSON::Array::Ptr json_ptr = DynamicResult.extract<JSON::Array::Ptr>();
	uint index = pconfig->getUInt("ADC.INDEX_OF_ARRAY");
	if(index > json_ptr->size())
	{
		logger.fatal("%d is greater than size of json array", index);
		return;
	}
	JSON::Array::Ptr json_ptr_2 =  json_ptr->getArray(index); //second level of array
	for(uint i=0; i< json_ptr_2->size(); i++)
	{
		object_temp = json_ptr_2->getObject(i);
		adc_struct_temp.channel = object_temp -> getValue<int>("port");
		adc_struct_temp.type =  static_cast<CalculateType>(object_temp ->getValue<int>("type"));
		if(object_temp->has("strategy"))
		{
			adc_struct_temp.strategy = object_temp -> getValue<std::string>("strategy");
		}
		else
		{
			//預設都是 INTERVAL_COLLECT
			adc_struct_temp.strategy = "INTERVAL_COLLECT";
		}

		ADC_sensors.push_back(adc_struct_temp);
	}
	//MultipleAreaReads_size = MultipleAreaReads.size();
	logger.information("Total %z of ADC sensor", ADC_sensors.size());
	if(pconfig->getBool("PHM.START")) //啟動PHM前需要先載入模型
	{
		//載入小夫模型參數
		logger.information("載入小夫模型參數");
		for(uint i=0; i< ADC_sensors.size(); i++)
		{
			if(static_cast<Feature>(pconfig->getInt("PHM.FEATURE")) == JsonFeature)
			{
				//模型類型
				cout << ADC_sensors[i].channel << endl;
				ADC_sensors[i].feature = ParesJsonFeature(ADC_sensors[i].type, ADC_sensors[i].channel, true);
			}
			else if(static_cast<Feature>(pconfig->getInt("PHM.FEATURE")) == ControlChart)
			{
				//管制線類型
				Path TargetFile(Path::current(), "heater_resist/ADC_"+Utility::IntToString(pconfig->getUInt("DEVICE.NUMBER"))+"_"+Utility::IntToString(i)+".json");
				std::ostringstream ostr;
				FileInputStream fis(TargetFile.toString());
				StreamCopier::copyStream(fis, ostr);
				logger.information("Load file:%s to stream.", TargetFile.toString());
				Parser parser;
				Object::Ptr mainPtr = parser.parse(ostr.str()).extract<Object::Ptr>();
				ADC_sensors[i].upper = mainPtr->getValue<float>("upper");
				ADC_sensors[i].lower = mainPtr->getValue<float>("lower");
			}
			pre_status.push_back(false);
			pre_pre_status.push_back(false);
		}
	}

	std::string file_name= "";
	if(pconfig->getBool("ADC.INTO_CSV"))
	{
		logger.information("啟動記錄CSV模式");
		for(uint i=0; i<ADC_sensors.size(); i++)
		{
			file_name ="csv/channel_"+Utility::IntToString(ADC_sensors[i].channel)+".csv";
			outputs.push_back(new std::ofstream (file_name.c_str(), std::ofstream::app));
		}
	}
}

json_feature BlockMemory::ParesJsonFeature(CalculateType type, int channel, bool verbose)
{
	json_feature feature;
//	Path scaler_filePath(Path::current(), "LAM_PHM/ADC_"+Utility::IntToString(channel)+".json");
//	std::ostringstream ostr;
//	//open file
//	if (scaler_filePath.isFile())
//	{
//		File inputFile(scaler_filePath);
//		if(inputFile.exists())
//		{
//			FileInputStream fis(scaler_filePath.toString());
//			StreamCopier::copyStream(fis, ostr);
//			logger.information("Load file:%s to stream.", scaler_filePath.toString());
//		}
//	}
//	else
//	{
//		logger.fatal("%s do not exist!", scaler_filePath.toString());
//	}
//	logger.trace("Total %z bytes", ostr.str().size());
//	Parser str_parser;
//	//Extract array pointer
//	Object::Ptr json_ptr = str_parser.parse(ostr.str()).extract<Object::Ptr>();
//	feature.diff_min = json_ptr->getArray("min")->get(0);
//	feature.roughness_min = json_ptr->getArray("min")->get(1);
//	if(verbose)
//	{
//		logger.trace("Channel %d : diff_min is %hf", channel, feature.diff_min);
//		logger.trace("Channel %d : roughness_min is %hf", channel, feature.roughness_min);
//	}
//	feature.diff_max = json_ptr->getArray("max")->get(0);
//	feature.roughness_max = json_ptr->getArray("max")->get(1);
//	if(verbose)
//	{
//		logger.trace("Channel %d : diff_max is %hf", channel, feature.diff_max);
//		logger.trace("Channel %d : roughness_max is %hf", channel, feature.roughness_max);
//	}
//	feature.diff_range = json_ptr->getArray("range")->get(0);
//	feature.roughness_range = json_ptr->getArray("range")->get(1);
//	if(verbose)
//	{
//		logger.trace("Channel %d : diff_range is %hf", channel, feature.diff_range);
//		logger.trace("Channel %d : roughness_range is %hf", channel, feature.roughness_range);
//	}
	std::ostringstream ostr_2;
	Path filePath(Path::current(), "LAM_PHM/ADC_"+Utility::IntToString(channel)+".json");
	//open file
	if (filePath.isFile())
	{
		File inputFile(filePath);
		if(inputFile.exists())
		{
			FileInputStream fis_2(filePath.toString());
			StreamCopier::copyStream(fis_2, ostr_2);
			logger.information("Load file:%s to stream.",filePath.toString());
		}
		else
		{
			logger.fatal("%s do not exist!",filePath.toString());
		}
	}
	logger.trace("Total %z bytes", ostr_2.str().size());
	Parser parser(new ParseHandler, ostr_2.str().size());
	//Extract array pointer
	Object::Ptr json_ptr_2 = parser.parse(ostr_2.str()).extract<Object::Ptr>();
	// Parse gamma
	feature.gamma = json_ptr_2->getValue<float>("_gamma");
	if(verbose)
	{
		logger.trace("Channel %d : gamma is %hf", channel, feature.gamma);
	}

	// Parse support_vectors_
	feature.support_vectors_ = Parse2DFloatArray(json_ptr_2, "support_vectors_", verbose);

	// Parse nSV
	feature.nSV = feature.support_vectors_.size();
	if(verbose)
	{
		logger.trace("Channel %d : nSV is %d", channel, feature.nSV);
	}

	// Parse alpha
	feature.alpha = Parse2DFloatArray(json_ptr_2, "dual_coef_", verbose);

	// Parse intercept
	feature.intercept_ = Parse1DFloatArray(json_ptr_2, "intercept_", verbose);

	// Parse threshold
	feature.threshold = json_ptr_2->getValue<float>("threshold");
	if(verbose)
	{
		logger.trace("Channel %d : threshold is %hf", channel, feature.threshold);
	}
	return feature;
}

vector<vector<float> > BlockMemory::Parse2DFloatArray(Poco::JSON::Object::Ptr json_ptr, std::string key, bool verbose)
{
	vector<vector<float> > temp_2d;
	Poco::JSON::Array::Ptr array_ptr = json_ptr->getArray(key);
	Poco::JSON::Array::Ptr array_ptr_2; //second dim of array
	for(uint i=0; i< array_ptr->size(); i++)
	{
		array_ptr_2 = array_ptr->getArray(i);
		vector<float> temp_1d;
		for(uint k=0; k<array_ptr_2->size(); k++)
		{
			temp_1d.push_back(array_ptr_2->get(k).convert<float>());
		}
		temp_2d.push_back(temp_1d);
	}
	return temp_2d;
}

vector<float> BlockMemory::Parse1DFloatArray(Poco::JSON::Object::Ptr json_ptr, std::string key, bool verbose)
{
	vector<float> temp_1d;
	Poco::JSON::Array::Ptr array_ptr = json_ptr->getArray(key);
	for(uint i=0; i< array_ptr->size(); i++)
	{
		temp_1d.push_back(array_ptr->get(i).convert<float>());
	}
	return temp_1d;
}

float BlockMemory::GetAverageValue(int channel, CalculateType Type)
{
    union
	{
    	u32 u;
    	float f;
    } un[bram_size/64];

    u32 bram32_block[bram_size/4];

	uint ads = channel / 16;
	uint port = channel % 16;
	off_t start_address;

	if(ads == 0)
	{
		start_address = brame_1;
	}
	else if(ads == 1)
	{
		start_address = brame_2;
	}
	else if(ads == 2)
	{
		start_address = brame_3;
	}
	else if(ads == 3)
	{
		start_address = brame_4;
	}
	else
	{
		logger.error("Invalid channel %d", channel);
		return -65535;
	}

	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fd == -1)
	{
		logger.error("Can not open /dev/mem");
		return -65535;
	}
	u32* bram32_ptr = (u32 *)mmap(NULL, bram_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, start_address);
	memcpy(bram32_block, bram32_ptr, bram_size); //0x1000 * 16 = 0x10000
	munmap(bram32_ptr, bram_size); //free bram32_ptr
	close(fd);

	//extract channel interval
	for(uint i=0; i< bram_size/64 ; i++)
	{
		un[i].u = bram32_block[port*(bram_size/64) + i];
	}
#if 1
	int index = ((un[0].u >> 12)/4); // 取得index
	/*
	int gap = (bram_size / 64) / length;
	for(int i=0; i<length; i++)
	{
		counter = gap * i;

		if(counter <= index)
		{
			value = value + un[index - counter + 1].f;
		}
		else // counter > index
		{
			value = value + un[(bram_size / 64)  + index - counter -1].f;
		}
	}
	*/
	//value = ConvertADCValue(value/length, static_cast<int>(Type));
	//printf("%d-%d index: %d value: %f \r\n", ads, port, index, value);
	std::cout << un[index].f << std::endl;
	return un[index].f;
#endif
}

void BlockMemory::GetMultiValue(Timer& timer)
{
	std::stringstream payload;
	std::vector<float> arr(channel_size-2);
	u32* bram32_ptr;
	u32 bram32_block[bram_size/4];
	off_t start_address = brame_1;
	int index, counter;
    union
	{
    	u32 u;
    	float f;
    }un[channel_size];
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fd == -1)
	{
		logger.fatal("Can not open /dev/mem");
		return;
	}

	bram32_ptr = (u32 *)mmap(NULL, bram_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, start_address);
	memcpy(bram32_block, bram32_ptr, bram_size); //0x1000 * 16 = 0x10000
	munmap(bram32_ptr, bram_size); //munmap bram32_ptr
	close(fd);

	for(uint k=14; k<16; k++)
	{
		for(uint c=0; c<channel_size; c++)
		{
			un[c].u = bram32_block[(k*channel_size)+c];
		}
		index = (un[0].u >> 12)/4;
		counter = channel_size - 2;

		//copy un[] to arr[]
		while(counter > 0)
		{
			if(counter <= index)
			{
				arr[channel_size - counter -2] = un[index - counter + 1].f;
			}
			else // counter > index
			{
				arr[channel_size - counter -2] = un[channel_size + index - counter -1].f;
			}
			counter--;
		}

		for(uint m=0; m<(channel_size-2); m++)
		{
			*outputs[k] << arr[m];
			if(m != channel_size-3)
			{
				payload << ",";
			}
		}
	}
}

void BlockMemory::LongTimeRecord(Timer& timer)
{
	std::vector<float> arr(channel_size-2);
	u32* bram32_ptr;
	u32 bram32_block[bram_size/4];
	off_t start_address;
	int index, counter;
    union
	{
    	u32 u;
    	float f;
    }un[channel_size];
	for(uint i=0; i<=(ADC_sensors.size()-1)/16; i++)
	{
		switch(i)
		{
			case 0:
				start_address = brame_1;
				break;
			case 1:
				start_address = brame_2;
				break;
			case 2:
				start_address = brame_3;
				break;
			case 3:
				start_address = brame_4;
				break;
			default:
				logger.fatal("channel %u over 64", ADC_sensors.size());
				return;
		}
		int fd = open("/dev/mem", O_RDWR | O_SYNC);
		if(fd == -1)
		{
			logger.fatal("Can not open /dev/mem");
			return;
		}

		bram32_ptr = (u32 *)mmap(NULL, bram_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, start_address);
		memcpy(bram32_block, bram32_ptr, bram_size); //0x1000 * 16 = 0x10000
		munmap(bram32_ptr, bram_size); //munmap bram32_ptr
		close(fd);
		payload.clear();
		for(uint k=0; k<16; k++)
		{
			/*超過ADC sensor長度則跳過*/
			if((i*16+k) >= ADC_sensors.size())
			{
				break;
			}
			// copy
			for(uint c=0; c<channel_size; c++)
			{
				un[c].u = bram32_block[(k*channel_size)+c];
			}
			index = (un[0].u >> 12)/4;
			counter = channel_size - 2;

			//copy un[] to arr[]
			while(counter > 0)
			{
				if(counter <= index)
				{
					arr[channel_size - counter -2] = un[index - counter + 1].f;
				}
				else // counter > index
				{
					arr[channel_size - counter -2] = un[channel_size + index - counter -1].f;
				}
				counter--;
			}
			if(ADC_sensors[16*i+k].type == current_5A || ADC_sensors[16*i+k].type == current_10A ||
					ADC_sensors[16*i+k].type == current_15A || ADC_sensors[16*i+k].type == current_20A ||
					ADC_sensors[16*i+k].type == current_30A || ADC_sensors[16*i+k].type == current_60A ) //交流電
			{
				payload << "ADC_" << pconfig->getUInt("DEVICE.NUMBER") << "_" << 16*i+k; //目標資料表
				payload << ",UTC=+8 "; //狀態描述
				for(uint m=0; m<200; m++)
				{
					payload << m << "=" << arr[m];
					if(m != 199)
					{
						payload << ",";
					}
				}
			}
			/*溫度暫時不收*/
			/*
			if(ADC_sensors[16*i+k].type == thermistor)//溫度
			{
				payload << "ADC_" << pconfig->getUInt("DEVICE.NUMBER") << "_" << 16*i+k; //目標資料表
				payload << ",start=unknown "; //狀態描述
				payload << "mean=" << std::accumulate(arr.begin(), arr.end(), 0.0)/arr.size(); //計算平均
			}
			*/
			else if(ADC_sensors[16*i+k].type == DCCurrent) //直流電
			{
//				if(std::accumulate(arr.begin(), arr.end(), 0.0)/arr.size() > 0.6)
//				{
//					logger.information("%u DCCurrent: %f", 16*i+k, std::accumulate(arr.begin(), arr.end(), 0.0)/arr.size());
					payload << "ADC_" << pconfig->getUInt("DEVICE.NUMBER") << "_" << 16*i+k; //目標資料表
					payload << ",UTC=+8 "; //狀態描述
					payload << "mean" << "=" << std::accumulate(arr.begin(), arr.end(), 0.0)/arr.size(); //計算平均
//				}
			}
			payload << endl;
		}
	}
	Upload_2();
}

void BlockMemory::PHM(Timer& timer)
{
	int each_channel_size = 1024; // 0x400
	int channel_bram_size = each_channel_size * 4;	//0x1000
	std::stringstream payload;
	std::vector<double> arr(channel_size-2);
	u32* bram32_ptr;
	off_t start_address;
	double sampling_rate = 2500.; // data sampling rate
	double freq = 0;
	int index, counter;
	std::vector<float> temp;
    union
	{
    	u32 u;
    	int i;
    }un[channel_size];

	for(uint i=0; i<ADC_sensors.size(); i++)
	{
		cout << ADC_sensors[i].channel << endl;
		switch(ADC_sensors[i].channel / 16)
		{
			case 0:
				start_address = brame_1;
				break;
			case 1:
				start_address = brame_2;
				break;
			case 2:
				start_address = brame_3;
				break;
			case 3:
				start_address = brame_4;
				break;
			case 4:
				start_address = brame_5;
				break;
			default:
				logger.fatal("channel %u over 64", ADC_sensors[i].channel);
				return;
		}
		start_address = start_address + ((ADC_sensors[i].channel % 16) * channel_bram_size);
		int fd = open("/dev/mem", O_RDWR | O_SYNC);
		if(fd == -1)
		{
			logger.fatal("Can not open /dev/mem");
			return;
		}
		bram32_ptr = (u32 *)mmap(NULL, channel_bram_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, start_address);
//		memcpy(bram32_block, bram32_ptr, channel_bram_size); //0x1000 * 16 = 0x10000
//		munmap(bram32_ptr, channel_bram_size); //munmap bram32_ptr
		close(fd);
		for(uint c=0; c<channel_size; c++)
		{
			un[c].u = *(bram32_ptr + c);
		}

		index = (un[0].u >> 12)/4;
		counter = channel_size - 2;
		//copy un[] to arr[]
		while(counter > 0)
		{
			if(counter <= index)
			{
				arr[channel_size - counter -2] = (200 * float(un[index - counter + 1].i - 524288) / 1048576);
			}
			else // counter > index
			{
				arr[channel_size - counter -2] = (200 * float( un[channel_size + index - counter -1].i - 524288) / 1048576);
			}
			counter--;
		}
		freq = freqFromFFT(arr, sampling_rate);


		int n = arr.size();
		vector<double> freqBand = dftFreqBand(n, 1./sampling_rate);
		// Get FFT amplitude
		vector<double> amplitude = fftAmplitude(arr);
		// Get overall vibration value
		double oa = calOverAllValue(freqBand, amplitude, 10, 1000);
		cout << "OA value: " << oa << endl;
		double max = percentile(arr, 95);
		double min = percentile(arr, 5);
		double amp = max - min;
		cout << "amp value: " << amp << endl;
		if(amp <  0.5)
		{
			break;
		}
		temp.clear();
		temp.push_back(oa);
		temp.push_back(amp);
		double dec = svm.decision_function_V2(temp,
											ADC_sensors[i].feature.support_vectors_,
											ADC_sensors[i].feature.alpha[0],
											ADC_sensors[i].feature.gamma,
											ADC_sensors[i].feature.intercept_[0],
											ADC_sensors[i].feature.threshold);
//
//
//		double max = percentile(arr, 95);
//		double min = percentile(arr, 5);
//		std::vector<double> sinWave(arr.size());
//		for (uint x=0; x < sinWave.size(); x++)
//		{
//			sinWave[x] = 0.5 * (max - min) *sin(2 * M_PI*freq*x / sampling_rate) + 0.5 * (max + min);
//		}
//		//PHM
//		// Get shift with highest cross-correlation
//		vector<double> xcorr = correlate(arr, sinWave);
//		shift = 1 - arr.size() + distance(xcorr.begin(), max_element(xcorr.begin(), xcorr.end()));
//			cout << "shift:" << shift << endl;
//		rollVector(sinWave, shift);
//			cout << "roll done" << endl;
//		vector<double>::const_iterator init1;
//		vector<double>::const_iterator end1;
//		vector<double>::const_iterator init2;
//		vector<double>::const_iterator end2;
//		if (shift > 0)
//		{
//			init1 = sinWave.begin() + shift;
//			end1 = sinWave.begin() + sinWave.size();
//			init2 = arr.begin() + shift;
//			end2 = arr.begin() + arr.size();
//		}
//		else
//		{
//			init1 = sinWave.begin();
//			end1 = sinWave.begin() + sinWave.size() + shift;
//			init2 = arr.begin();
//			end2 = arr.begin() + arr.size() + shift;
//		}
//		cout << "shift done" << endl;
//		vector<double> sinWave2(init1, end1);
//		vector<double> data2(init2, end2);
//		double f1 = vectorMean(vectorAbs(difference(sinWave2, data2)));
//		cout << "vectorMean done:"<< f1 << endl;
//		double f2 = roughness(data2);
//		cout << "roughness done:" << f2 << endl;
//		double f1 = roughness(data2);
//		double f2 = vectorMean(vectorAbs(difference(sinWave2, data2)));
//		printf("[f1, f2] = [%g, %g]\n", f1, f2);
//		f1 = (f1- ADC_sensors[i].feature.diff_min) / ADC_sensors[i].feature.diff_range;
//		cout << "ADC_sensors[channel].feature.diff_range:"<< ADC_sensors[channel].feature.diff_range << endl;
//		f2 = (f2- ADC_sensors[channel].feature.roughness_min) / ADC_sensors[channel].feature.roughness_range;
//		printf("After normalize:[f1, f2] = [%g, %g]\n", f1, f2);
//		temp.clear();
//		temp.push_back(f1);
//		temp.push_back(f2);
//		double dec = svm.decision_function_V2(temp,
//											ADC_sensors[i].feature.support_vectors_,
//											ADC_sensors[i].feature.alpha[0],
//											ADC_sensors[i].feature.gamma,
//											ADC_sensors[i].feature.intercept_[0],
//											ADC_sensors[i].feature.threshold);
		cout << "decision_function_v2: " << dec << endl;
		if(dec < 0 && pre_status[i] && pre_pre_status[i])
		{
			logger.error(" Channel %d PHM 異常, decision: %f", ADC_sensors[i].channel, dec);
			payload << "ADC_" << ADC_sensors[i].channel << "_ANOMALY"; //目標資料表
			payload << ",freq=" << freq;
			payload << ",percentile_max=" << max;
			payload << ",percentile_min=" << min;
			payload << ",normalize_f1=" << oa;
			payload << ",normalize_f2=" << amp;
			payload << ",decision=" << dec;
			payload << " ";
			for(uint m=0; m<arr.size(); m++)
			{
				payload << m << "=" << arr[m];
				if(m != arr.size()-1)
				{
					payload << ",";
				}
			}
			payload << endl;
		}
		pre_pre_status[i] = pre_status[i];
		pre_status[i] = dec < 0 ? true: false;
		munmap(bram32_ptr, channel_bram_size);
	}
	if(pconfig->getBool("ADC.INTO_DB") && payload.str().length()>0)
	{

		this->_ActiveMethod(payload.str());
	}
}

//
//void BlockMemory::PHM(Timer& timer)
//{
//	std::stringstream payload;
//	std::vector<double> arr(channel_size-2);
//	u32* bram32_ptr;
//	u32 bram32_block[bram_size/4];
//	off_t start_address;
//	int index, counter, shift, channel;
//	double sampling_rate = 2604.; // data sampling rate
//	double freq;
//    union
//	{
//    	u32 u;
//    	float f;
//    }un[channel_size];
//	for(uint i=0; i< ADC_sensors.size(); i++)
//	{
//		switch(i)
//		{
//			case 0:
//				start_address = brame_1;
//				break;
//			case 1:
//				start_address = brame_2;
//				break;
//			case 2:
//				start_address = brame_3;
//				break;
//			case 3:
//				start_address = brame_4;
//				break;
//			default:
//				logger.fatal("channel %u over 64", ADC_sensors.size());
//				return;
//		}
//		int fd = open("/dev/mem", O_RDWR | O_SYNC);
//		if(fd == -1)
//		{
//			logger.fatal("Can not open /dev/mem");
//			return;
//		}
//		bram32_ptr = (u32 *)mmap(NULL, bram_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, start_address);
//		memcpy(bram32_block, bram32_ptr, bram_size); //0x1000 * 16 = 0x10000
//		munmap(bram32_ptr, bram_size); //munmap bram32_ptr
//		close(fd);
//		std::vector<float> temp;
//		for(uint k=0; k<15; k++)
//		{
//			channel = i*16+k;
//			/*超過ADC sensor長度則跳過*/
//			if(channel >= PHM_CHANNEL)
//			{
//				break;
//			}
//			// copy
//			for(uint c=0; c<channel_size; c++)
//			{
//				un[c].u = bram32_block[(k*channel_size)+c];
//			}
//			index = (un[0].u >> 12)/4;
//			counter = channel_size - 2;
//			arr.clear();
//			//copy un[] to arr[]
//			while(counter > 0)
//			{
//				if(counter <= index)
//				{
//					arr[channel_size - counter -2] = un[index - counter + 1].f;
//				}
//				else // counter > index
//				{
//					arr[channel_size - counter -2] = un[channel_size + index - counter -1].f;
//				}
//				counter--;
//			}
//			cout << "copy done" << endl;
//
//			std::vector<double> arr_temp(PHM_SIZE);
//			for(uint m=0; m<PHM_SIZE; m++)
//			{
//				arr_temp[m] = arr[m];
//			}
//
//			freq = freqFromFFT(arr_temp, sampling_rate);
//			logger.information("channel:%d, freq:%f", channel, freq);
//			if(freq < 50 || freq > 70)
//			{
//				continue;
//			}
//			double max = percentile(arr_temp, 95);
//			double min = percentile(arr_temp, 5);
//			std::vector<double> sinWave(arr_temp.size());
//			for (uint x=0; x < sinWave.size(); x++)
//			{
//				sinWave[x] = 0.5 * (max - min) *sin(2 * M_PI*freq*x / sampling_rate) + 0.5 * (max + min);
//			}
////			sinWaveSet.push_back(sinWave);
////			cout << i*16+k << " generate sin wave" << endl;
//
//			/* 儲存CSV
//			for(uint j=0; j<PHM_SIZE; j++)
//			{
//				arr_temp[j] = arr[j];
//				*outputs[k] << arr_temp[j] << ",";
//				*outputs[k] << endl;
//			}
//			*outputs[k] << endl;
//			*/
//
//			//PHM
//			// Get shift with highest cross-correlation
//			vector<double> xcorr = correlate(arr_temp, sinWave);
//			shift = 1 - arr_temp.size() + distance(xcorr.begin(), max_element(xcorr.begin(), xcorr.end()));
////			cout << "shift:" << shift << endl;
//			rollVector(sinWave, shift);
////			cout << "roll done" << endl;
//			vector<double>::const_iterator init1;
//			vector<double>::const_iterator end1;
//			vector<double>::const_iterator init2;
//			vector<double>::const_iterator end2;
//			if (shift > 0)
//			{
//				init1 = sinWave.begin() + shift;
//				end1 = sinWave.begin() + sinWave.size();
//				init2 = arr_temp.begin() + shift;
//				end2 = arr_temp.begin() + arr_temp.size();
//			}
//			else
//			{
//				init1 = sinWave.begin();
//				end1 = sinWave.begin() + sinWave.size() + shift;
//				init2 = arr_temp.begin();
//				end2 = arr_temp.begin() + arr_temp.size() + shift;
//			}
//			cout << "shift done" << endl;
//			vector<double> sinWave2(init1, end1);
//			vector<double> data2(init2, end2);
//			double f1 = vectorMean(vectorAbs(difference(sinWave2, data2)));
//			cout << "vectorMean done:"<< f1 << endl;
//			double f2 = roughness(data2);
//			cout << "roughness done:" << f2 << endl;
//			printf("[f1, f2] = [%g, %g]\n", f1, f2);
//			f1 = (f1- ADC_sensors[channel].feature.diff_min) / ADC_sensors[channel].feature.diff_range;
//			cout << "ADC_sensors[channel].feature.diff_range:"<< ADC_sensors[channel].feature.diff_range << endl;
//			f2 = (f2- ADC_sensors[channel].feature.roughness_min) / ADC_sensors[channel].feature.roughness_range;
//			printf("After normalize:[f1, f2] = [%g, %g]\n", f1, f2);
//			temp.clear();
//			temp.push_back(f1);
//			temp.push_back(f2);
//			double dec = svm.decision_function_V2(temp,
//												ADC_sensors[channel].feature.support_vectors_,
//												ADC_sensors[channel].feature.alpha[0],
//												ADC_sensors[channel].feature.gamma,
//												ADC_sensors[channel].feature.intercept_[0],
//												ADC_sensors[channel].feature.threshold);
//			cout << "decision_function_v2: " << dec << endl;
//			if(dec < 0 && pre_status[k] && pre_pre_status[k])
//			{
//				logger.error(" Channel %d PHM 異常, decision: %f", ADC_sensors[channel].channel, dec);
//				payload << "ADC_" << channel; //目標資料表
//				payload << ",freq=" << freq;
//				payload << ",percentile_max=" << max;
//				payload << ",percentile_min=" << min;
//				payload << ",normalize_f1=" << f1;
//				payload << ",normalize_f2=" << f2;
//				payload << ",decision=" << dec;
//				payload << " ";
//				for(uint m=0; m<arr_temp.size(); m++)
//				{
//					payload << m << "=" << arr_temp[m];
//					if(m != arr_temp.size()-1)
//					{
//						payload << ",";
//					}
//				}
//				payload << endl;
//			}
//			pre_pre_status[k] = pre_status[k];
//			pre_status[k] = dec < 0 ? true: false;
//		}
//	}
//	this->_ActiveMethod(payload.str());
//}
//
//void BlockMemory::PHM_ControlChart(Timer& timer)
//{
//	std::stringstream payload;
//	std::vector<double> arr(channel_size-2);
//	u32* bram32_ptr;
//	u32 bram32_block[bram_size/4];
//	off_t start_address;
//	int index, counter;
//	int channel;
//	double freq, sampling_rate = 2604.; // data sampling rate
//	double meansquare;
//    union
//	{
//    	u32 u;
//    	float f;
//    }un[channel_size];
//	for(uint i=0; i<ADC_sensors.size(); i++)
//	{
//		switch(ADC_sensors[i].channel / 16)
//		{
//			case 0:
//				start_address = brame_1;
//				break;
//			case 1:
//				start_address = brame_2;
//				break;
//			case 2:
//				start_address = brame_3;
//				break;
//			case 3:
//				start_address = brame_4;
//				break;
//			case 4:
//				start_address = brame_5;
//				break;
//			default:
//				logger.fatal("channel %u over 80", ADC_sensors.size());
//				return;
//		}
//		start_address = start_address + ((ADC_sensors[i].channel % 16) * channel_bram_size);
//
//		int fd = open("/dev/mem", O_RDWR | O_SYNC);
//		if(fd == -1)
//		{
//			logger.fatal("Can not open /dev/mem");
//			return;
//		}
//		bram32_ptr = (u32 *)mmap(NULL, bram_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, start_address);
//		memcpy(bram32_block, bram32_ptr, bram_size); //0x1000 * 16 = 0x10000
//		munmap(bram32_ptr, bram_size); //munmap bram32_ptr
//		close(fd);
//		std::vector<float> temp;
//		for(uint k=0; k<15; k++)
//		{
//			channel = i*16+k;
//			/*超過ADC sensor長度則跳過*/
//			if(channel >= PHM_CHANNEL)
//			{
//				break;
//			}
//			// copy
//			for(uint c=0; c<channel_size; c++)
//			{
//				un[c].u = bram32_block[(k*channel_size)+c];
//			}
//			index = (un[0].u >> 12)/4;
//			counter = channel_size - 2;
//			arr.clear();
//			//copy un[] to arr[]
//			while(counter > 0)
//			{
//				if(counter <= index)
//				{
//					arr[channel_size - counter -2] = un[index - counter + 1].f;
//				}
//				else // counter > index
//				{
//					arr[channel_size - counter -2] = un[channel_size + index - counter -1].f;
//				}
//				counter--;
//			}
//
//			std::vector<double> arr_temp(PHM_SIZE);
//			for(uint m=0; m<PHM_SIZE; m++)
//			{
//				arr_temp[m] = arr[m];
//			}
//			freq = freqFromFFT(arr_temp, sampling_rate);
//			logger.information("channel:%d, freq:%f", channel, freq);
//			if(freq < 50 || freq > 70)
//			{
//				continue;
//			}
//			meansquare = sqrt( ( std::inner_product( arr_temp.begin(), arr_temp.end(), arr_temp.begin(), 0 ) ) / static_cast<double>( arr_temp.size() ) );
//			cout << channel << ":" << meansquare << endl;
//			meansquare = 1/meansquare;
//			cout << channel << ":" << meansquare << endl;
//
//			/* 儲存CSV
//			for(uint j=0; j<PHM_SIZE; j++)
//			{
//				arr_temp[j] = arr[j];
//				*outputs[k] << arr_temp[j] << ",";
//				*outputs[k] << endl;
//			}
//			*outputs[k] << endl;
//			*/
//
//			if(meansquare > ADC_sensors[channel].upper || meansquare < ADC_sensors[channel].lower)
//			{
//				logger.error("Channel %d PHM 異常, 1/meansquare: %f", ADC_sensors[channel].channel, meansquare);
//				payload << "ADC_" << pconfig->getUInt("DEVICE.NUMBER") << "_" << channel; //目標資料表
//				payload << ",GMT=+8";
//				payload << " ";
//				for(uint m=0; m<arr_temp.size(); m++)
//				{
//					payload << m << "=" << arr_temp[m];
//					if(m != arr_temp.size()-1)
//					{
//						payload << ",";
//					}
//				}
//				payload << endl;
//			}
//		}
//	}
//	this->_ActiveMethod(payload.str());
//}

template <class T>
float BlockMemory::Convet(T value, CalculateType type)
{
	float r = 0;
	if(type == thermistor)	//溫度
	{
		double temp = 1;
		temp = log(((40950000/(4095.0-(value*1.0))) - 10000));
		temp = 1 / (0.001129148 + (0.000234125 * temp) + (0.0000000876741 * temp * temp * temp));
		r = temp - 273.15;           // Convert Kelvin to Celcius
	}
	else if(type == axis)
	{
		r = ((1.0 * value * 6 / (3.3*4095.0/5)) - 3);
	}
	else if(type == current_5A )	// AC
	{
		r = (((((1.0 * value) - 2048.0 - 50)* 5000.00 * 5.0) / 4096.0))/1000;
	}
	else if(type == current_10A )
	{
		r = (((((1.0 * value) - 2048.0 - 50)* 5000.00 * 10.0) / 4096.0))/1000;
	}
	else if(type == current_15A )
	{
		r = (((((1.0 * value) - 2048.0 - 50)* 5000.00 * 15.0) / 4096.0))/1000;
	}
	else if(type == current_20A )
	{
		r = (((((1.0 * value) - 2048.0 - 50)* 5000.00 * 20.0) / 4096.0))/1000;
	}
	else if(type == current_30A )
	{
		r = (((((1.0 * value) - 2048.0 - 50)* 5000.00 * 30.0) / 4096.0))/1000;
	}
	else if(type == current_60A )
	{
		r = (((((1.0 * value) - 2048.0 - 50)* 5000.00 * 60.0) / 4096.0))/1000;
	}
	else if(type == DC_25A)	//DC
	{
		r = (((((1.0 * value))* 5.0) / 4096.0))/0.16;
	}
	else if(type == DCCurrent)	//DC
	{
		r = (((((1.0 * value) - 2048.0) * 5.0) / 4096.0));
	}
	else if(type == Conductivity)	//導電度計
	{
		r = (value - 655.2) * 2000 / 2621;
	}
	else if(type == Manual)	//手動g sensor
	{
		r = (200 * float(value - 524288) / 1048576);
	}
	return r;
}

void BlockMemory::Collector(Timer& timer)
{
	int each_channel_size = 1024; // 0x400
	int channel_bram_size = each_channel_size * 4;	//0x1000
	std::stringstream payload;
	std::vector<float> arr(channel_size-2);
	u32* bram32_ptr;
	off_t start_address;
	int index, counter;
    union
	{
    	u32 u;
    	float f;
    	int i;
    }un[channel_size];
    try
    {
		for(uint i=0; i<ADC_sensors.size(); i++)
		{
			if(ADC_sensors[i].strategy == "COLLECT")
			{
				switch(ADC_sensors[i].channel / 16)
				{
					case 0:
						start_address = brame_1;
						break;
					case 1:
						start_address = brame_2;
						break;
					case 2:
						start_address = brame_3;
						break;
					case 3:
						start_address = brame_4;
						break;
					case 4:
						start_address = brame_5;
						break;
					default:
						logger.fatal("channel %u over 64", ADC_sensors[i].channel);
						return;
				}
				start_address = start_address + ((ADC_sensors[i].channel % 16) * channel_bram_size);
				int fd = open("/dev/mem", O_RDWR | O_SYNC);
				if(fd == -1)
				{
					logger.fatal("Can not open /dev/mem");
					return;
				}
				bram32_ptr = (u32 *)mmap(NULL, channel_bram_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, start_address);
		//		memcpy(bram32_block, bram32_ptr, channel_bram_size); //0x1000 * 16 = 0x10000
		//		munmap(bram32_ptr, channel_bram_size); //munmap bram32_ptr
				close(fd);
				for(uint c=0; c<channel_size; c++)
				{
					un[c].u = *(bram32_ptr + c);
				}
				index = (un[0].u >> 12)/4;
				counter = channel_size - 2;
				//copy un[] to arr[]
				while(counter > 0)
				{
					if(counter <= index)
					{
						arr[channel_size - counter -2] =
								(pconfig->getBool("ADC.DO_CONVERT") ?
								un[index - counter + 1].i :
								un[index - counter + 1].f);
					}
					else //   > index
					{
						arr[channel_size - counter -2] =
								(pconfig->getBool("ADC.DO_CONVERT") ?
								un[channel_size + index - counter -1].i:
								un[channel_size + index - counter -1].f);
					}
					counter--;
				}
				/*
				if(ADC_sensors[i].type == axis)//震動
				{
					if(pconfig->getBool("ADC.ENABLE_AXIS"))
					{
						payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
						payload << ",utc=+8 "; //狀態描述 -> 時區
						for(uint m=0; m<(channel_size-2); m++)
						{
							payload << m << "=" << (pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
							if(m != channel_size-3)
							{
								payload << ",";
							}
						}
						if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
						{
							for(uint m=0; m<(channel_size-2); m++)
							{
								*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
										Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
								*outputs[i] << ",";
							}
							*outputs[i] << endl;
						}
					}
				}
				*/
				if(ADC_sensors[i].type == current_5A || ADC_sensors[i].type == current_10A ||
						ADC_sensors[i].type == current_15A || ADC_sensors[i].type == current_20A ||
						ADC_sensors[i].type == current_30A || ADC_sensors[i].type == current_60A ) //交流電
				{
					if(pconfig->getBool("ADC.ENABLE_AC"))
					{
						LocalDateTime now;
						if(isSineWave(arr, 500))
						{
//							logger.information("channel[%d] 波形偵測成功", ADC_sensors[i].channel);
							payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
							payload << ",utc=+8 "; //狀態描述
							for(uint m=0; m<220; m++)
							{
								payload << m << "=" << (pconfig->getBool("ADC.DO_CONVERT")?
										Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
								if(m != 220-1)
								{
									payload << ",";
								}
							}
							if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
							{
								for(uint m=0; m<220; m++)
								{
									*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
											Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
									*outputs[i] << ",";
								}
								*outputs[i] << endl;
							}
							if(pconfig->getBool("ADC.INTO_REDIS"))
							{
								// 計算RMS

								float square = 0;
								for(uint m=0; m<430; m++)
								{
									square += pow((pconfig->getBool("ADC.DO_CONVERT")?
											Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]), 2);
								}
								rb->set(pconfig->getString("DEVICE.ID") + "_ADC" +  NumberFormatter::format(ADC_sensors[i].channel),
								NumberFormatter::format(sqrt(square/ (float)(430)), 2) + "," +
								DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%S"));
							}
						}
						else
						{
							rb->set(pconfig->getString("DEVICE.ID") + "_ADC" +  NumberFormatter::format(ADC_sensors[i].channel),
									"0," + DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%S"));
						}
					}
				}
				else if(ADC_sensors[i].type == DCCurrent || ADC_sensors[i].type == DC_25A) //DC
				{
					if(pconfig->getBool("ADC.ENABLE_DC"))
					{
						payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
						payload << ",utc=+8 "; //狀態描述
						for(uint m=0; m<200; m++)
						{
							payload << m << "=" << (pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
							if(m != 200-1)
							{
								payload << ",";
							}
						}
						if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
						{
							for(uint m=0; m<200; m++)
							{
								*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
										Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
								*outputs[i] << ",";
							}
							*outputs[i] << endl;
						}
					}
				}
				else if(ADC_sensors[i].type == thermistor)//溫度
				{
					if(pconfig->getBool("ADC.ENABLE_THERMISTER"))
					{
						payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
						payload << ",utc=+8 "; //狀態描述
						float mean = std::accumulate(arr.begin(), arr.end(), 0.0)/arr.size();	//計算平均
						mean = (pconfig->getBool("ADC.DO_CONVERT") ?
								Convet<float>(mean, ADC_sensors[i].type) : mean);
						payload << "mean=" << mean;
						if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
						{
							*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(mean, ADC_sensors[i].type) : mean);
							*outputs[i] << ",";
							*outputs[i] << endl;
						}
					}
				}
				else if(ADC_sensors[i].type == Conductivity) //導電度計
				{
					if(pconfig->getBool("ADC.ENABLE_CONDUCTIVITY"))
					{
						payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
						payload << ",utc=+8 "; //狀態描述
						float mean = std::accumulate(arr.begin(), arr.end(), 0.0)/arr.size();	//計算平均
						mean = pconfig->getBool("ADC.DO_CONVERT") ?
								Convet<float>(mean, ADC_sensors[i].type) : mean;
						payload << "mean=" << mean;
						if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
						{
							*outputs[i] << pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(mean, ADC_sensors[i].type) : mean;
							*outputs[i] << ",";
							*outputs[i] << endl;
						}
					}
				}
				else if(ADC_sensors[i].type == Manual)
				{
					payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
					payload << ",utc=+8 "; //狀態描述

					for(uint m=0; m<1022; m++)
					{
						payload << m << "=" << (pconfig->getBool("ADC.DO_CONVERT")?
								Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
						if(m != 1021)
						{
							payload << ",";
						}
					}
					if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
					{
						for(uint m=0; m<1022; m++)
						{
							*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
							*outputs[i] << ",";
						}
						*outputs[i] << endl;
					}
					if(pconfig->getBool("ADC.INTO_REDIS"))
					{
						// 計算RMS
						LocalDateTime now;
						float square = 0;
						for(uint m=0; m<1022; m++)
						{
							square += pow((pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]), 2);
						}
						rb->set(pconfig->getString("DEVICE.ID") + "_ADC" +  NumberFormatter::format(ADC_sensors[i].channel),
						NumberFormatter::format(sqrt(square/ (float)(1022)), 2) + "," +
						DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%S"));
					}
				}
				else if(ADC_sensors[i].type == pulse)
				{
					if(pconfig->getBool("ADC.ENABLE_PULSE"))
					{
						payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
						payload << ",utc=+8 "; //狀態描述
						for(uint m=0; m<990; m++)
						{
							payload << m << "=" << (pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
							if(m != 990 -1)
							{
								payload << ",";
							}
						}
						if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
						{
							for(uint m=0; m < 990; m++)
							{
								*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
										Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
								*outputs[i] << ",";
							}
							*outputs[i] << endl;
						}
					}
				}
				payload << endl;
				munmap(bram32_ptr, channel_bram_size);
			}
		}
	//	cout << "done:" << payload.str()  <<  endl;
		if(pconfig->getBool("ADC.INTO_DB") && payload.str().length()>0)
		{
			this->_ActiveMethod(payload.str());
		}
	}
	catch (Exception& exc)
	{
		logger.error(exc.displayText());
	}
}

void BlockMemory::_Collector(Timer& timer)
{
	int each_channel_size = 1024; // 0x400
	int channel_bram_size = each_channel_size * 4;	//0x1000
	std::stringstream payload;
	std::vector<float> arr(channel_size-2);
	u32* bram32_ptr;
	off_t start_address;
	int index, counter;
    union
	{
    	u32 u;
    	float f;
    	int i;
    }un[channel_size];
    try
    {
		for(uint i=0; i<ADC_sensors.size(); i++)
		{
			// iterator for ADC_sensors
			if(ADC_sensors[i].strategy == "INTERVAL_COLLECT")
			{
				// 背景常態執行
				switch(ADC_sensors[i].channel / 16)
				{
					case 0:
						start_address = brame_1;
						break;
					case 1:
						start_address = brame_2;
						break;
					case 2:
						start_address = brame_3;
						break;
					case 3:
						start_address = brame_4;
						break;
					case 4:
						start_address = brame_5;
						break;
					default:
						logger.fatal("channel %u over 64", ADC_sensors[i].channel);
						return;
				}
				start_address = start_address + ((ADC_sensors[i].channel % 16) * channel_bram_size);
				int fd = open("/dev/mem", O_RDWR | O_SYNC);
				if(fd == -1)
				{
					logger.fatal("Can not open /dev/mem");
					return;
				}
				bram32_ptr = (u32 *)mmap(NULL, channel_bram_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, start_address);
		//		memcpy(bram32_block, bram32_ptr, channel_bram_size); //0x1000 * 16 = 0x10000
		//		munmap(bram32_ptr, channel_bram_size); //munmap bram32_ptr
				close(fd);
				for(uint c=0; c<channel_size; c++)
				{
					un[c].u = *(bram32_ptr + c);
				}
				index = (un[0].u >> 12)/4;
				counter = channel_size - 2;
				//copy un[] to arr[]
				while(counter > 0)
				{
					if(counter <= index)
					{
						arr[channel_size - counter -2] =
								(pconfig->getBool("ADC.DO_CONVERT") ?
								un[index - counter + 1].i :
								un[index - counter + 1].f);
					}
					else //   > index
					{
						arr[channel_size - counter -2] =
								(pconfig->getBool("ADC.DO_CONVERT") ?
								un[channel_size + index - counter -1].i:
								un[channel_size + index - counter -1].f);
					}
					counter--;
				}

				if(ADC_sensors[i].type == axis)//震動
				{
					if(pconfig->getBool("ADC.ENABLE_AXIS"))
					{
						payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
						payload << ",utc=+8 "; //狀態描述 -> 時區
						for(uint m=0; m<(channel_size-2); m++)
						{
							payload << m << "=" << (pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
							if(m != channel_size-3)
							{
								payload << ",";
							}
						}
						if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
						{
							for(uint m=0; m<(channel_size-2); m++)
							{
								*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
										Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
								*outputs[i] << ",";
							}
							*outputs[i] << endl;
						}
					}
				}

				if(ADC_sensors[i].type == current_5A || ADC_sensors[i].type == current_10A ||
						ADC_sensors[i].type == current_15A || ADC_sensors[i].type == current_20A ||
						ADC_sensors[i].type == current_30A || ADC_sensors[i].type == current_60A ) //交流電
				{
					if(pconfig->getBool("ADC.ENABLE_AC"))
					{
//						if(isSineWave(arr, 900))
//						{
//							logger.information("channel[%d] 波形偵測成功", ADC_sensors[i].channel);
							payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
							payload << ",utc=+8 "; //狀態描述
							for(uint m=0; m<220; m++)
							{
								payload << m << "=" << (pconfig->getBool("ADC.DO_CONVERT")?
										Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
								if(m != 220-1)
								{
									payload << ",";
								}
							}
							if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
							{
								for(uint m=0; m<220; m++)
								{
									*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
											Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
									*outputs[i] << ",";
								}
								*outputs[i] << endl;
							}
							if(pconfig->getBool("ADC.INTO_REDIS"))
							{
								// 計算RMS
								LocalDateTime now;
								float square = 0;
								for(uint m=0; m<430; m++)
								{
									square += pow((pconfig->getBool("ADC.DO_CONVERT")?
											Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]), 2);
								}
								rb->set(pconfig->getString("DEVICE.ID") + "_ADC" +  NumberFormatter::format(ADC_sensors[i].channel),
								NumberFormatter::format(sqrt(square/ (float)(430)), 2) + "," +
								DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%S"));
							}
//						}
//						else
//						{
//							logger.warning("channel[%d] 波形偵測失敗", ADC_sensors[i].channel);
//						}
					}
				}
				else if(ADC_sensors[i].type == DCCurrent || ADC_sensors[i].type == DC_25A) //DC
				{
					if(pconfig->getBool("ADC.ENABLE_DC"))
					{
						payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
						payload << ",utc=+8 "; //狀態描述
						for(uint m=0; m<200; m++)
						{
							payload << m << "=" << (pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
							if(m != 200-1)
							{
								payload << ",";
							}
						}
						if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
						{
							for(uint m=0; m<200; m++)
							{
								*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
										Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
								*outputs[i] << ",";
							}
							*outputs[i] << endl;
						}
					}
				}
				else if(ADC_sensors[i].type == thermistor)//溫度
				{
					if(pconfig->getBool("ADC.ENABLE_THERMISTER"))
					{
						payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
						payload << ",utc=+8 "; //狀態描述
						float mean = std::accumulate(arr.begin(), arr.end(), 0.0)/arr.size();	//計算平均
						mean = (pconfig->getBool("ADC.DO_CONVERT") ?
								Convet<float>(mean, ADC_sensors[i].type) : mean);
						payload << "mean=" << mean;
						if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
						{
							*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(mean, ADC_sensors[i].type) : mean);
							*outputs[i] << ",";
							*outputs[i] << endl;
						}
					}
				}
				else if(ADC_sensors[i].type == Conductivity) //導電度計
				{
					if(pconfig->getBool("ADC.ENABLE_CONDUCTIVITY"))
					{
						payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
						payload << ",utc=+8 "; //狀態描述
						float mean = std::accumulate(arr.begin(), arr.end(), 0.0)/arr.size();	//計算平均
						mean = pconfig->getBool("ADC.DO_CONVERT") ?
								Convet<float>(mean, ADC_sensors[i].type) : mean;
						payload << "mean=" << mean;
						if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
						{
							*outputs[i] << pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(mean, ADC_sensors[i].type) : mean;
							*outputs[i] << ",";
							*outputs[i] << endl;
						}
					}
				}
				else if(ADC_sensors[i].type == Manual)
				{
					payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
					payload << ",utc=+8 "; //狀態描述

					for(uint m=0; m<1022; m++)
					{
						payload << m << "=" << (pconfig->getBool("ADC.DO_CONVERT")?
								Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
						if(m != 1021)
						{
							payload << ",";
						}
					}
					if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
					{
						for(uint m=0; m<1022; m++)
						{
							*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
							*outputs[i] << ",";
						}
						*outputs[i] << endl;
					}
					if(pconfig->getBool("ADC.INTO_REDIS"))
					{
						// 計算RMS
						LocalDateTime now;
						float square = 0;
						for(uint m=0; m<1022; m++)
						{
							square += pow((pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]), 2);
						}
						rb->set(pconfig->getString("DEVICE.ID") + "_ADC" +  NumberFormatter::format(ADC_sensors[i].channel),
						NumberFormatter::format(sqrt(square/ (float)(1022)), 2) + "," +
						DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%S"));
					}
				}
				else if(ADC_sensors[i].type == pulse)
				{
					if(pconfig->getBool("ADC.ENABLE_PULSE"))
					{
						payload << "ADC_" << ADC_sensors[i].channel; //目標資料表
						payload << ",utc=+8 "; //狀態描述
						for(uint m=0; m<990; m++)
						{
							payload << m << "=" << (pconfig->getBool("ADC.DO_CONVERT")?
									Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
							if(m != 990 -1)
							{
								payload << ",";
							}
						}
						if(pconfig->getBool("ADC.INTO_CSV")) // dump to csv
						{
							for(uint m=0; m < 990; m++)
							{
								*outputs[i] << (pconfig->getBool("ADC.DO_CONVERT")?
										Convet<int>(arr[m], ADC_sensors[i].type) : arr[m]);
								*outputs[i] << ",";
							}
							*outputs[i] << endl;
						}
					}
				}
				payload << endl;
				munmap(bram32_ptr, channel_bram_size);
			}
		}
	//	cout << "done:" << payload.str()  <<  endl;
		if(pconfig->getBool("ADC.INTO_DB") && payload.str().length()>0)
		{
			this->_ActiveMethod(payload.str());
		}
	}
	catch (Exception& exc)
	{
		logger.error(exc.displayText());
	}
}


bool BlockMemory::isSineWave(std::vector<float> arr, int threshold)
{
    if(arr.size() < 128)
    {
    	return false;
    }
	Complex test[128];

	for(uint i=0; i<128; i++)
	{
		test[i] = Complex(double(arr[i]), 0.0);
	}
    CArray data(test, 128);

    // forward fft
    fft(data);
    double max, second_max;

    if(std::abs(data[0]) < std::abs(data[1]))
    {
       max = std::abs(data[0]);
       second_max = std::abs(data[1]);
    }
    else
    {
        max = std::abs(data[1]);
        second_max = std::abs(data[0]);
    }

    for (int i = 2; i < 64; ++i)
    {
        if (std::abs(data[i]) > max)
        {
            second_max = max;
            max = std::abs(data[i]);
        }
        else if(std::abs(data[i]) > second_max && std::abs(data[i]) != max)
        {
        	second_max = std::abs(data[i]);
        }
    }
    cout << "max - second_max: " << max - second_max << endl;
    if((max - second_max) > threshold)
    {
    	return true;
    }
    return false;
}

bool BlockMemory::isWave(std::vector<float> arr, uint length, uint diff)
{
	float max = 0;
	float min = 0;
	int index_max = 0;
	int index_min = 0;
	for(uint i=0; i<length; i++)
	{
		if(arr[i] > max)
		{
			max = arr[i];
			index_max = i;
		}
		if(arr[i] < min)
		{
			min = arr[i];
			index_min = i;
		}
	}
	/*
	for(uint i = 0; i<3; i++) //腳氏濾波器
	{
		if(max < arr[index_max - i])
		{
			max = arr[index_max - i];
		}
		if(max < arr[index_max + i])
		{
			max = arr[index_max + i];
		}
	}
	for(uint i = 1; i<3; i++) //腳氏濾波器
	{
		if(min > arr[index_min - i])
		{
			min = arr[index_min - i];
		}
		if(min > arr[index_min + i])
		{
			min = arr[index_min + i];
		}
	}
	*/
	if((max - min) > diff)
	{
		return true;
	}

	max = 0;
	min = 0;
	uint size = arr.size() - 1;
	for(uint i=0; i<length; i++)
	{
		if(arr[size-i] > max)
		{
			max = arr[size-i];
			index_max = i;
		}
		if(arr[size-i] < min)
		{
			min = arr[size-i];
			index_min = i;
		}
	}
	/*
	for(uint i = 0; i<3; i++)
	{
		if(max < arr[index_max - i])
		{
			max = arr[index_max - i];
		}
		if(max < arr[index_max + i])
		{
			max = arr[index_max + i];
		}
	}
	for(uint i = 1; i<3; i++)
	{
		if(min > arr[index_min - i])
		{
			min = arr[index_min - i];
		}
		if(min > arr[index_min + i])
		{
			min = arr[index_min + i];
		}
	}
	*/
	if((max - min) > diff)
	{
		return true;
	}

	return false;
}

/*
void BlockMemory::FDC()
{
	std::vector<float> arr(1022);
	float dec = 0;
	for(uint i=0; i<ADC_sensors.size(); i++)
	{
		dec = 0; //reset dec
		if(ADC_sensors[i].type == thermistor) //溫度
		{
			dec = GetAverageValue(ADC_sensors[i].channel, ADC_sensors[i].type, 100); //從BlockMemory取值


			if(dec > ADC_sensors[i].feature.DQI_x_T_U)
			{
				logger.error(" Channel %d ", ADC_sensors[i].channel);
				Utility::WriteLog("log/channel_"+Utility::IntToString(ADC_sensors[i].channel), "[axis] decision: "+ Utility::FloatToString(dec));
			}

		}
		else if(ADC_sensors[i].type == axis) //震動
		{
			GetMultiValue(ADC_sensors[i].channel, ADC_sensors[i].type, arr); //從BlockMemory取值


			if(svm.STD_Filter_V2(arr, 0.5, 5)) //判斷雜訊
			{
				dec = svm.decision_function_V2(
						svm.Histogram_V2(arr, ADC_sensors[i].feature.bins, ADC_sensors[i].feature.range_down, ADC_sensors[i].feature.range_up),
											ADC_sensors[i].feature.support_vectors_,
											ADC_sensors[i].feature.alpha[0],
											ADC_sensors[i].feature.gamma);
				cout << "decision_function_v2: " << dec << endl;
				if(dec+ ADC_sensors[i].feature.intercept_[0]+ ADC_sensors[i].feature.threshold > 0)
				{
					logger.error(" Channel %d ", ADC_sensors[i].channel);
					Utility::WriteLog("log/channel_"+Utility::IntToString(ADC_sensors[i].channel), "[axis] decision: "+ Utility::FloatToString(dec+ ADC_sensors[i].feature.intercept_[0]+ ADC_sensors[i].feature.threshold));
				}
			}

		}
		else //電流
		{
			GetMultiValue(ADC_sensors[i].channel, ADC_sensors[i].type, arr); //從BlockMemory取值


			if(svm.STD_Filter_V2(arr, 1.5, 5)) //判斷雜訊
			{
				dec = svm.decision_function_V2(
						svm.Histogram_V2(arr, ADC_sensors[i].feature.bins, ADC_sensors[i].feature.range_down, ADC_sensors[i].feature.range_up),
											ADC_sensors[i].feature.support_vectors_,
											ADC_sensors[i].feature.alpha[0],
											ADC_sensors[i].feature.gamma);
				cout << "decision_function_v2: " << dec << endl;
				if(dec+ ADC_sensors[i].feature.intercept_[0]+ ADC_sensors[i].feature.threshold > 0)
				{
					logger.error(" Channel %d ", ADC_sensors[i].channel);
					Utility::WriteLog("log/channel_"+Utility::IntToString(ADC_sensors[i].channel), "[axis] decision: "+ Utility::FloatToString(dec+ ADC_sensors[i].feature.intercept_[0]+ ADC_sensors[i].feature.threshold));
				}
			}

		}
	}
}
*/

void BlockMemory::Upload(const std::string& payload)
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

void BlockMemory::Upload_2() //單次上傳
{
	std::string temp = payload.str();
	//logger.information("ThreadPool used %d", ThreadPool::defaultPool().used());
	try
	{
		URI target;
		target.setScheme("http");
		target.setHost(pconfig->getString("ADC.SERVER_HOST", "10.11.0.156"));
		target.setPort(pconfig->getInt("ADC.SERVER_PORT", 8086));
		target.setPath("write");
		target.addQueryParameter("db", pconfig->getString("ADC.SERVER_DB", "test")); //選擇db

		Poco::Net::HTTPClientSession session(target.getHost(), target.getPort());
		Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, target.toString(), Poco::Net::HTTPMessage::HTTP_1_1);
		request.setContentType("application/x-www-form-urlencoded");

		request.setContentLength(temp.length());
		std::ostream& BodyOstream = session.sendRequest(request); // sends request, returns open stream
		BodyOstream << temp;  // sends the body
	}
	catch (Poco::Exception& exc)
	{
		logger.error("Upload:%s", exc.displayText());
	}
	return;
}

void BlockMemory::setIntervalCollectTime(int time)
{
	IntervalCollectTime = time;
}

int BlockMemory::getIntervalCollectTime()
{
	return IntervalCollectTime;
}

void BlockMemory::setIntervalCollectDuration(int time)
{
	IntervalCollectDuration = time;
}

int BlockMemory::getIntervalCollectDuration()
{
	return IntervalCollectDuration;
}

void BlockMemory::IntervalCollect(Timer& timer)
{
	Timer shortTimeTimer(0, IntervalCollectTime);
	shortTimeTimer.start(TimerCallback<BlockMemory>(*this, &BlockMemory::_Collector));
	Thread::sleep(IntervalCollectDuration);
	shortTimeTimer.stop();
}

void BlockMemory::fft(CArray& x)
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

void BlockMemory::generateSinWave()
{
	std::vector<double> arr(channel_size-2);
	u32* bram32_ptr;
	u32 bram32_block[bram_size/4];
	off_t start_address;
	int index, counter;
    union
	{
    	u32 u;
    	float f;
    }un[channel_size];

    for(uint i=0; i<=(ADC_sensors.size()-1)/16; i++)
	{
		cout << "=========" << i << "=========" << endl;
    	switch(i)
		{
			case 0:
				start_address = brame_1;
				break;
			case 1:
				start_address = brame_2;
				break;
			case 2:
				start_address = brame_3;
				break;
			case 3:
				start_address = brame_4;
				break;
			default:
				logger.fatal("channel %u over 64", ADC_sensors.size());
				return;
		}
		int fd = open("/dev/mem", O_RDWR | O_SYNC);
		if(fd == -1)
		{
			logger.fatal("Can not open /dev/mem");
			return;
		}

		bram32_ptr = (u32 *)mmap(NULL, bram_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, start_address);
		memcpy(bram32_block, bram32_ptr, bram_size); //0x1000 * 16 = 0x10000
		munmap(bram32_ptr, bram_size); //munmap bram32_ptr
		close(fd);
		for(uint k=0; k<16; k++)
		{
			/*超過ADC sensor長度則跳過*/
			if((i*16+k) >= ADC_sensors.size())
			{
				break;
			}
			// copy
			for(uint c=0; c<channel_size; c++)
			{
				un[c].u = bram32_block[(k*channel_size)+c];
			}
			index = (un[0].u >> 12)/4;
			counter = channel_size - 2;

			//copy un[] to arr[]
			while(counter > 0)
			{
				if(counter <= index)
				{
					arr[channel_size - counter -2] = un[index - counter + 1].f;
				}
				else // counter > index
				{
					arr[channel_size - counter -2] = un[channel_size + index - counter -1].f;
				}
				counter--;
			}

			std::vector<double> arr_temp(PHM_SIZE);
			for(uint m=0; m<PHM_SIZE; m++)
			{
				arr_temp[m] = arr[m];
			}
			double sampling_rate = 2604.; // data sampling rate
			double freq = freqFromFFT(arr_temp, sampling_rate);
			cout << i*16+k << " freq:" << freq << endl;
			double max = percentile(arr_temp, 95);
			double min = percentile(arr_temp, 5);
			std::vector<double> sinWave(arr_temp.size());
			for (uint x=0; x < sinWave.size(); x++)
			{
				sinWave[x] = 0.5 * (max - min) *sin(2 * M_PI*freq*x / sampling_rate) + 0.5 * (max + min);
			}
			sinWaveSet.push_back(sinWave);
			cout << i*16+k << " generate sin wave" << endl;
		}
	}
}
