#pragma once 
#include<mysql.h>

class cgi_sql{

	public:
		cgi_sql(const std::string &_host = "127.0.0.1",\
				const std::string &_user = "root",\
				const std::string _passwd = "",\
				const std::string _db = "bigdata"
				const int port = 3306);
		~cgi_sql();
		my_connect();

	private:
		MYSQL *conn;

		std::string host;
		std::string user;
		std::string passwd;
		std::string db;
		int port;
}
