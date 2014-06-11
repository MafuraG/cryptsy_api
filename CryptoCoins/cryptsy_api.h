#pragma once
#include <string>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>
#include "market.h"

typedef std::unordered_map<std::wstring, std::wstring> query_params;
typedef std::unordered_map<std::wstring, market> cyptsy_markets;
class cryptsy_api
{
public:	
	cryptsy_api();
	
	//sha512 hmac implementation
	static std::string sign_hash_hmac(std::string data, std::string secret_key);
	
	//getters
	static std::string get_user_api_key(){ return user_api_key; }
	static std::string get_user_secret(){ return user_secret; }
	static std::string get_api_url(){ return user_api_url; }

	//setters
	static void set_user_api_key(std::string _api_key){ user_api_key = _api_key; }
	static void set_user_secret(std::string _user_secret){ user_secret = _user_secret; }
	static void set_user_api_url(std::string _user_api_url){ user_api_url = _user_api_url; }	

	//Asynchrounous query to cryptsy api with method_name and post data
	pplx::task<web::json::value>  query_crptsy_api(std::wstring method_name, query_params  postdata);

	//get available markets info
	void get_markets();

	//find the best exchange price
	void get_winning_market_LTC();
	
	//converters
	static std::wstring s2ws(const std::string& str);
	static std::string  ws2s(const std::wstring& wstr);
	static std::wstring map2query(const query_params m);

	//tests
	void test_hash_hmac();
	
	~cryptsy_api();
private:	
	static std::string user_api_key;
	static std::string user_secret;
	/*std::string user_message;*/
	static std::string user_api_url;
	static cyptsy_markets markets; //active markets
};

