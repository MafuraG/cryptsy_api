// CryptoCoins.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <string.h>
#include <iostream>
#include "cryptsy_api.h"

#include "stdafx.h"
#include <string>
#include "cpprest/containerstream.h"
#include "cpprest/filestream.h"
#include "cpprest/http_client.h"
#include "cpprest/json.h"
#include "cpprest/producerconsumerstream.h"



//#include <curl/curl.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams
using namespace concurrency;
using namespace std;
using namespace web::json;


int main() {	

	cryptsy_api::set_user_api_key("3b8e794787cfc4790e1ab0ebd3ee62e1c70cc151");
	cryptsy_api::set_user_api_url("https://api.cryptsy.com/api");
	cryptsy_api::set_user_secret("008313e0ad6a195bfea1e639e9987725959e85137312a0cf9fa906b0c729fad67998c41852fd6c91");

	cryptsy_api c_api;
	query_params param;

	/*auto res = c_api.query_crptsy_api(L"getmarkets", param);*/

	//auto res_json = res.get();
	c_api.get_winning_market_LTC();

	return 0;
}

