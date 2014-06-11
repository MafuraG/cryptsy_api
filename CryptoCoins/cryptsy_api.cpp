#include "stdafx.h"
#include <string>
#include "cpprest/containerstream.h"
#include "cpprest/filestream.h"
#include "cpprest/http_client.h"
#include "cpprest/json.h"
#include "cpprest/producerconsumerstream.h"
#include <openssl/hmac.h>
#include "cryptsy_api.h"
#include "Windows.h"
#include <locale>
#include <codecvt>
#include <ctime>
#include "market.h"
#include <fstream>
#include <boost/lexical_cast.hpp>
//#include <curl/curl.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams
using namespace concurrency;
using namespace std;
using namespace web::json;
using namespace pplx;



cryptsy_api::cryptsy_api()
{
}


cryptsy_api::~cryptsy_api()
{
}

//std::wstring cryptsy_api::s2ws(const std::string& s)
//{
//	int len;
//	int slength = (int)s.length() + 1;
//	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
//	wchar_t* buf = new wchar_t[len];
//	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
//	std::wstring r(buf);
//	delete[] buf;
//	return r;
//}

std::wstring cryptsy_api::s2ws(const std::string& str)
{	
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string cryptsy_api::ws2s(const std::wstring& wstr)
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

std::string cryptsy_api::sign_hash_hmac(std::string msg, std::string secret_key)
{
	// The secret key for hashing	
	const char* key = secret_key.c_str();

	// The data that we're going to hash
	const char* data = msg.c_str();

	unsigned char* digest;

	// Using sha1 hash engine here.
	// You may use other hash engines. e.g EVP_md5(), EVP_sha224, EVP_sha512, etc
	digest = HMAC(EVP_sha512(), key, strlen(key), (unsigned char*)data, strlen(data), NULL, NULL);

	// Be careful of the length of string with the choosen hash engine. SHA1 produces a 20-byte hash value which rendered as 40 characters.
	// Change the length accordingly with your choosen hash engine
	const int len = 64;	
	char mdString[2*len +1];
	for (int i = 0; i < len; i++)
	{
		sprintf_s(mdString + i*2,3, "%02x", (unsigned int)digest[i]);
		//printf("value md:%s\n", mdString);
		//printf("value i:%d\n", i);
	}		

	//printf("HMAC digest: %s\n", mdString);
	
	return std::string(mdString);	
}

std::wstring cryptsy_api::map2query(const query_params m){
	wstring res=L"",part;
	typedef query_params::const_iterator MapIterator;
	for (MapIterator iter = m.begin(); iter != m.end(); iter++)
	{
		if (iter != m.begin()) res += L"&";		 
		res += iter->first + L"=" + iter->second;
	}
	return res;
}

pplx::task<web::json::value>  cryptsy_api::query_crptsy_api(std::wstring method_name,query_params postdata)
{
	auto ptr_json_val = make_shared<value>();
	auto ptr_method_name = make_shared<wstring>(method_name);
	auto ptr_post_data = make_shared<query_params>(postdata);

	//return create_task([ptr_json_val, ptr_method_name, ptr_post_data](){		
		
	(*ptr_post_data)[U("method")] = (*ptr_method_name);

	time_t nonce = std::time(NULL);
	(*ptr_post_data)[U("nonce")] = std::to_wstring(nonce);

	//conversion to query string
	wstring postdata_str = cryptsy_api::map2query(*ptr_post_data);

	wstring post_digest = cryptsy_api::s2ws(cryptsy_api::sign_hash_hmac(cryptsy_api::ws2s(postdata_str), cryptsy_api::get_user_secret()));

	//std::wcout<<"postdata params: " << postdata_str<<endl;
		
	http_client client(cryptsy_api::s2ws(cryptsy_api::get_api_url()));

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	request.headers().add(L"Sign", post_digest);
	request.headers().add(L"Key", cryptsy_api::s2ws(cryptsy_api::get_user_api_key()));
	request.set_body(postdata_str);
	request.headers().set_content_type(L"application/json");		

	//std::wcout << "request params: " << request.to_string() << endl;
	//std::wcout << "request body: " << request. << endl;

	return client.request(request)
			.then([](http_response response)
		{
			std::wcout << response.status_code() << std::endl;

			if (response.status_code() == status_codes::OK)
			{
				//auto body = response.extract_string();

				//std::wcout << body.get().c_str();

				return response.extract_json();
			}
			else{
				// return an empty JSON value
				return response.extract_json();
			}
	});	
	
}

void cryptsy_api::get_markets()
{	
	query_params param;

	auto res = query_crptsy_api(L"getmarkets", param);

	auto res_json = res.get();

	//In order to see the results
	//Commented out when not needed
	wofstream myfile(L"C:\\Users\\Mafura\\Dropbox\\cryptsy_price.csv");
	if (myfile.is_open())
	{
		//myfile << res_json.serialize().c_str();		
		//myfile.close();
	}
	else {
		cout << "Unable to open file";
		return;
	}

	if (!res_json.is_null())
	{
		//clear markets dictionary
		markets.clear();
		//auto response = 
		auto results = res_json[L"return"];
		for (auto const & p : results.as_array())
		{
			auto o = p;
			auto _marketid = o[L"marketid"];
			auto _label = o[L"label"];
			auto _primary_currency_code = o[L"primary_currency_code"];
			auto _primary_currency_name = o[L"primary_currency_name"];
			auto _secondary_currency_code = o[L"secondary_currency_code"];
			auto _secondary_currency_name = o[L"secondary_currency_name"];
			auto _current_volume = o[L"current_volume"];
			auto _last_trade = o[L"last_trade"];
			auto _high_trade = o[L"high_trade"];
			auto _low_trade = o[L"low_trade"];
			auto _created = o[L"created"];

			//_label.
			market m;
			
			//std::cout << "filling dictionary..."<<endl;
			try{
				m.label = _label.as_string();
				//std::wcout << "m.label ...OK :" <<m.label<< endl;
				m.marketid = boost::lexical_cast<int>(_marketid.as_string());
				//std::cout << "m.marketid ...OK" << endl;
				m.primary_currency_code = _primary_currency_code.as_string();
				//std::cout << "m.primary_currency_code ...OK" << endl;
				m.primary_currency_name = _primary_currency_name.as_string();
				//std::cout << "m.primary_currency_name ...OK" << endl;
				m.secondary_currency_code = _secondary_currency_code.as_string();
				//std::cout << "m.secondary_currency_code ...OK" << endl;
				m.secondary_currency_name = _secondary_currency_name.as_string();
				//std::cout << "m.secondary_currency_name ...OK" << endl;
				m.current_volume = boost::lexical_cast<double>(_current_volume.as_string());
				//std::cout << "m.current_volume ...OK " << m.current_volume<<endl;
				m.last_trade = boost::lexical_cast<long double>(_last_trade.as_string());
				//std::cout << "m.last_trade ...OK " <<m .last_trade<<endl;
				m.high_trade = boost::lexical_cast<double>(_high_trade.as_string());
				//std::cout << "m.high_trade ...OK " <<m.high_trade<< endl;
				m.low_trade = boost::lexical_cast<double>(_low_trade.as_string());
				//std::cout << "m.low_trade ...OK " <<m.low_trade<< endl;
				//m.created = _created.as_object();

				markets[m.label] = m;
		
				//myfile << m.label << " ; " << boost::lexical_cast<wstring>(m.last_trade) << " ; " << boost::lexical_cast<wstring>(m.current_volume) << " ; " << endl;
				myfile << m.label << " ; " << to_wstring(m.last_trade) << " ; " << to_wstring(m.current_volume) << " ; " << 
					to_wstring(m.last_trade * m.current_volume)<< endl;
				
			}
			catch (json_exception const & e)
			{
				wcout << e.what() << endl;				
			}			
		}
	}

	myfile.close();

	/*marketid	Integer value representing a market
	label	Name for this market, for example: AMC / BTC
	primary_currency_code	Primary currency code, for example : AMC
	primary_currency_name	Primary currency name, for example : AmericanCoin
	secondary_currency_code	Secondary currency code, for example : BTC
	secondary_currency_name	Secondary currency name, for example : BitCoin
	current_volume	24 hour trading volume in this market
	last_trade	Last trade price for this market
	high_trade	24 hour highest trade price in this market
	low_trade	24 hour lowest trade price in this market
	created	Datetime(EST) the market was created*/
}

void cryptsy_api::get_winning_market_LTC()
{
	get_markets();
	long double ltc_min_volume = 1000.0;
	long double btc_min_volume = 30.0;
	wcout << "Number of markets" << markets.size() << endl;

	typedef std::unordered_map<wstring, int> passed_vect;
	passed_vect passed;
	wstring key,rev_key,prim_coin,sec_coin;
	market prim_m,rev_m,best_m;
	long double best_price = 0;
	for (auto kv:markets){
		key = kv.first;
		//wcout << "key =>" << key << endl;
		prim_coin = key.substr(0, key.length() - 4);		
		sec_coin = key.substr(key.length() - 3, 3);

		if (sec_coin == L"BTC")
		{
			rev_key = prim_coin + L"/" + L"LTC";
			if ((kv.second.current_volume*kv.second.last_trade) < btc_min_volume) continue;
		}
		else 
		{ 
			rev_key = prim_coin + L"/" + L"BTC";
			if ((kv.second.current_volume*kv.second.last_trade)< ltc_min_volume) continue;
		}

		//wcout << L"prim_coin:" << key << L" sec_coin " << rev_key << endl;
		//Markets with active trading will be considerered		

		if (!passed.count(key) && !passed.count(rev_key)) 
		{/*key not passed*/
			//add markets to passed
			passed[key] = 1;
			passed[rev_key] = 1;

			//Get trade prices
			long double prim_trade_price, rev_trade_price,ltc_coins;
			prim_m = kv.second;
			rev_m = markets[rev_key];
			prim_trade_price = prim_m.last_trade;
			rev_trade_price = rev_m.last_trade;
			//How many ltc coins are gotten by 
			//BTC -> AltCoin -> LTC
			if (sec_coin == L"BTC")
				ltc_coins = rev_trade_price / prim_trade_price;
			else
				ltc_coins = prim_trade_price / rev_trade_price;
			if (best_price < ltc_coins)
			{
				best_price = ltc_coins;
				best_m = prim_m;
			}
		}
	}

	wcout << "Best Market: " << best_m.label << " => " << best_price << " LTC. " << "Price of trade" << to_wstring(best_m.last_trade) 
		<<"  Volume of trade =>"<<to_wstring(best_m.last_trade*best_m.current_volume)<< endl;
	int k;
	cin >> k;
	
}

void cryptsy_api::test_hash_hmac()
{
	cryptsy_api::user_api_key = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"; //user api key
	cryptsy_api::user_secret = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";//your secret shhhh
	std:string user_message = "1dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd";
	std::string ctrl_digest = "e2a065eb8d3d58321803902f72a525e246ee765c88339796fa6cdab7c66fffd3f427884302598a2f1b60721e0366c795dd82c5fe1b88b5de3aa0dc7951e8b4a3";
	

	std::string digest = sign_hash_hmac(user_message,user_secret);

	if (ctrl_digest == digest)
		std::cout << "Test passed OK"<< "\n";
	else
		std::cout << "Test failed STOP"<<"\n";

	//std::cout << digest<<"\n";
}

//declare static members

std::string cryptsy_api::user_api_key="";
std::string cryptsy_api::user_api_url = "";
std::string cryptsy_api::user_secret = "";
cyptsy_markets cryptsy_api::markets;

