#include"my_sql.h"

cgi_sql::cgi_sql(const std::string &_host,\
				const std::string &_user,\
				const std::string _passwd,\
				const std::string _db,\
				const int _port);
	:host(_host)
	,user(_user)
	,passwd(_passwd)
	,db(_db)
	,port(_port)
{
	conn = my_sql_init(NULL);
}

cgi_sql::~cgi_sql()
{
	if(conn){
		mysql_close(conn);
		conn = NULL;
	}
	if(res){
		free(res);
		res = NULL;
	}
}
int cgi::my_connect()
{
	if(!mysql_real_connect(conn,host.c_str(),user.c_str(),passwd.c_str(),db.c_str(),port)){
		std::cout<<"connect error"<<std::endl;
		return 1;
	}else{
		std::cout<<"connect success"<<std::endl;
		return 0;
	}
}
int cgi_sql::my_insert(std::string &table,std::string &_field,std::string &_data)
{
	std::string _sql = "INSERT INTO ";
	_sql += _table;
	_sql += " ";
	_sql += _field;
	_sql += " ";
	_sql += values;
	_sql += " ";
	_sql += _data;
	int ret = mysql_query(conn,_sql.c_str());
	return ret;
}
void cgi_sql::my_select(std::string _table,std::string options = "")
{
	std::string _sql = "select * from ";
	_sql += _table;
	_sql += options;
	int ret = mysql_query(conn,_sql.c_str());
	if(ret == 0){
		res = mysql_store_result(conn);
		if(res){
		int res_lines = mysql_num_rows(res);
			int res_cols = mysql_num_cols(res);
		}
	}
	MYSQL_FIELD *fn = NULL;
	for(;fn = mysql_fetch_field(res);){
		std::cout<<fn->name<<"\t";
		std::cout<<std::endl;
	}:
}



