#include "MySQLApi.h"


#include <algorithm>

// 所有构造函数这儿的MYSQL指针初始化似乎无用
CMySQL::CMySQL()
{
	_ptrMysql = mysql_init(NULL);
}

CMySQL::CMySQL(const std::string& host, const std::string& user = "", const std::string& pass = "", const std::string& databd = "", const std::string& charset = "", int port = 0, int flag = 0)
{
	init(host, user, pass, databd, charset, port, flag);
	_ptrMysql = mysql_init(NULL);
}

CMySQL::CMySQL(const DBConf& dbconf)
{
	init(dbconf);
	_ptrMysql = mysql_init(NULL);
}

CMySQL::~CMySQL()
{
	if (_ptrMysql)
	{
		mysql_close(_ptrMysql);
		_ptrMysql = nullptr;
	}
}

void CMySQL::init(const std::string& host, const std::string& user, const std::string& pass, const std::string& databd, const std::string& charset, int port, int flag)
{
	_dbConf._host = host;
	_dbConf._user = user;
	_dbConf._password = pass;
	_dbConf._database = databd;
	_dbConf._charset = charset;
	_dbConf._port = port;
	_dbConf._flag = flag;
}

void CMySQL::init(const DBConf& dbconf)
{
	_dbConf = dbconf;
}

// 确保在连接数据库时，有且仅有一个可用的MYSQL指针
void CMySQL::connect()
{
	disconnect();

	if (nullptr == _ptrMysql)
	{
		_ptrMysql = mysql_init(NULL);
	}

	// 连接建立后，设置字符集
	if (!_dbConf._charset.empty())
	{
		if (mysql_options(_ptrMysql, MYSQL_SET_CHARSET_NAME, _dbConf._charset.c_str()))
		{
			// error
		}
	}

	if (NULL == mysql_real_connect(_ptrMysql, _dbConf._host.c_str(), _dbConf._user.c_str(), _dbConf._password.c_str(), _dbConf._database.c_str(), _dbConf._port, NULL, _dbConf._flag))
	{
		// error
	}

	_isConnected = true;
}

// 确保在连接数据库时，始终都存在一个可用的MYSQL指针
void CMySQL::disconnect()
{
	if (_ptrMysql)
	{
		mysql_close(_ptrMysql);
		_ptrMysql = mysql_init(NULL);
	}

	_isConnected = false;
}

std::string CMySQL::escape_string_nosafe(const std::string& from)
{
	// 最大假设每个字符都需要转义
	std::string::size_type len = from.length() * 2 + 1;

	char* pTo = new char[len];
	memset(pTo, 0x00, len);
	mysql_escape_string(pTo, from.c_str(), from.length());
	std::string to = pTo;
	delete[] pTo;

	return to;
}

void CMySQL::build_ostringstream(const RECORD_DATA& columns, std::ostringstream& column_names, std::ostringstream& column_values)
{
	bool is_beg = true;
	std::for_each(columns.begin(), columns.end(), [&is_beg, &column_names, &column_values](RECORD_DATA::const_reference record)
	{
		if (is_beg)
		{
			column_names << "`" << record.first << "`";
			if (DB_INT == record.second.first)
			{
				column_values << record.second.second;
			}
			else
			{
				column_values << "'" << escape_string_nosafe(record.second.second) << "'";
			}

			is_beg = false;
		}
		else
		{
			column_names << ",`" << record.first << "`";
			if (DB_INT == record.second.first)
			{
				column_values << "," << record.second.second;
			}
			else
			{
				column_values << ",'" << escape_string_nosafe(record.second.second) << "'";
			}
		}
	});
}

std::string CMySQL::build_insert_sql_nosafe(const std::string& table_name, const RECORD_DATA& columns)
{
	std::ostringstream column_names;
	std::ostringstream column_values;

	build_ostringstream(columns, column_names, column_values);

	std::ostringstream os;
	os << "insert into " << table_name << " (" << column_names.str() << ") values (" << column_values.str() << ")";
	return os.str();
}

std::string CMySQL::build_replace_sql_nosafe(const std::string& table_name, const RECORD_DATA& columns)
{
	std::ostringstream column_names;
	std::ostringstream column_values;

	build_ostringstream(columns, column_names, column_values);

	std::ostringstream os;
	os << "replace into " << table_name << " (" << column_names.str() << ") values (" << column_values.str() << ")";
	return os.str();
}

std::string CMySQL::build_update_sql_nosafe(const std::string& table_name, const RECORD_DATA& columns, const std::string& condition)
{
	std::ostringstream column_nameValue_set;

	bool is_beg = true;
	std::for_each(columns.begin(), columns.end(), [&is_beg, &column_nameValue_set](RECORD_DATA::const_reference record)
	{
		if (is_beg)
		{
			column_nameValue_set << "`" << record.first << "`";
			is_beg = false;
		}
		else
		{
			column_nameValue_set << ",`" << record.first << "`";
		}

		if (DB_INT == record.second.first)
		{
			column_nameValue_set << "= " << record.second.second;
		}
		else
		{
			column_nameValue_set << "= '" << escape_string_nosafe(record.second.second) << "'";
		}
	});

	std::ostringstream os;
	os << "update " << table_name << " set " << column_nameValue_set.str() << " " << condition;
	return os.str();
}