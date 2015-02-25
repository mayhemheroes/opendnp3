/**
 * Licensed to Green Energy Corp (www.greenenergycorp.com) under one or
 * more contributor license agreements. See the NOTICE file distributed
 * with this work for additional information regarding copyright ownership.
 * Green Energy Corp licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This project was forked on 01/01/2013 by Automatak, LLC and modifications
 * may have been made to this file. Automatak, LLC licenses these modifications
 * to you under the terms of the License.
 */
#include "LogTester.h"

#include <asiodnp3/ConsoleLogger.h>

#include <iostream>

using namespace openpal;

namespace opendnp3
{

LogRecord::LogRecord() :
	id(),
	filters(0),	
	location(),
	message(),
	errorCode(-1)
{}

LogRecord::LogRecord(const LogEntry& entry) : 
	id(entry.GetAlias()), 
	filters(entry.GetFilters()), 
	location(entry.GetLocation()),
	message(entry.GetMessage()),
	errorCode(entry.GetErrorCode())
{

}

LogTester::LogTester(uint32_t filters) : 
	root(this, "test", filters), 
	outputToStdIO(false),
	logger(root.GetLogger())
{

}


void LogTester::Log( const LogEntry& entry )
{
	if (outputToStdIO)
	{
		asiodnp3::ConsoleLogger::Instance().Log(entry);
	}

	messages.push_back(entry);
}

int32_t LogTester::PopFilter()
{
	if (messages.size() > 0)
	{
		auto flags = messages.front().filters.GetBitfield();
		messages.pop_front();
		return flags;	
	}
	else
	{
		return 0;
	}
}

bool LogTester::PopOneEntry(int32_t filter)
{
	if (messages.size() == 1)
	{
		if (messages.front().filters.IsSet(filter))
		{
			messages.pop_front();
			return true;
		}
		else return false;
	}
	else return false;
}

bool LogTester::PopErrorCode(int code)
{
	while (!messages.empty())
	{
		bool match = messages.front().errorCode == code;
		messages.pop_front();
		if (match)
		{
			return true;
		}
	}

	return false;
}

bool LogTester::PopUntil(int32_t filter)
{
	while (!messages.empty())
	{
		bool match = messages.front().filters.IsSet(filter);
		messages.pop_front();
		if (match)
		{
			return true;
		}
	}

	return false;
}

int LogTester::ClearLog()
{
	int max = -1;
	LogEntry le;
	while (!messages.empty())
	{
		if (messages.front().errorCode > max) max = le.GetErrorCode();
		messages.pop_front();
	}

	return max;
}

void LogTester::Log(const std::string& location, const std::string& message)
{
	logger.Log(flags::EVENT, location.c_str(), message.c_str());
}

void LogTester::WriteToStdIo()
{
	this->outputToStdIO = true;
}

int LogTester::NextErrorCode()
{
	LogRecord rec;
	while (!messages.empty())
	{
		rec = messages.front();
		messages.pop_front();
		if (rec.errorCode >= 0)
		{
			return rec.errorCode;
		}
	}
	return -1;
}

bool LogTester::GetNextEntry(LogRecord& record)
{
	if (messages.empty()) return false;
	else
	{
		record = messages.front();
		messages.pop_front();
		return true;
	}
}

void LogTester::Pop(openpal::ILogHandler& log)
{
	LogRecord record;
	while (GetNextEntry(record))
	{
		LogEntry le(record.id.c_str(), record.filters, record.location.c_str(), record.message.c_str(), record.errorCode);
		log.Log(le);
	}
}

openpal::Logger LogTester::GetLogger()
{
	return root.GetLogger();
}

bool LogTester::IsLogErrorFree()
{
	return ClearLog() < 0;
}

}

