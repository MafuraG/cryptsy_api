#pragma once
#include "stdafx.h"
#include <string>
#include <ctime>

class market
{
public:
	market();
	~market();

	int marketid;
	std::wstring label;
	std::wstring primary_currency_code;
	std::wstring primary_currency_name;
	std::wstring secondary_currency_code;
	std::wstring secondary_currency_name;
	double current_volume;
	long double last_trade;
	double high_trade;
	double low_trade;
	time_t created;
};

