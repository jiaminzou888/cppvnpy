#pragma once

#include <WinSock2.h>

#include "mysql/include/mysql.h"
#pragma comment(lib,"mysql/lib/libmysql.lib")

#include <map>
#include <vector>
#include <sstream>

// 数据库配置
struct DBConf
{
	std::string _host;
	std::string _user;
	std::string _password;
	std::string _database;
	std::string _charset;
	int _port{ 0 };
	// 客户端标识
	int _flag{ 0 };

	// 读取数据库配置
	void loadFromMap(const std::map<std::string, std::string>& mpParam)
	{
		// 不做拷贝构造时，编译报错?
		std::map<std::string, std::string> mpTmp = mpParam;

		_host = mpTmp["dbhost"];
		_user = mpTmp["dbuser"];
		_password	= mpTmp["dbpass"];
		_database	= mpTmp["dbname"];
		_charset	= mpTmp["charset"];
		_port = atoi(mpTmp["dbport"].c_str());
		_flag = 0;

		if (0 == mpTmp["dbport"].compare(""))
		{
			_port = 3306;
		}
	}
};


/*
* MySQL数据库操作类
* 非线程安全，通常一个线程一个MySQL对象
* 对于insert/update可以有更好的函数封装，保证SQL注入
* DB_INT表示组装sql语句时，不加""和转义
* DB_STR表示组装sql语句时，  加""和转义
*/
class CMySQL
{
public:
	CMySQL();
	CMySQL(const std::string& host, const std::string& user, const std::string& pass, const std::string& databd, const std::string& charset, int port, int flag);
	CMySQL(const DBConf& dbconf);
	~CMySQL();

	// 屏蔽拷贝构造和赋值构造
	CMySQL(const CMySQL& mysql) = delete;
	CMySQL& operator = (const CMySQL& mysql) = delete;

	void init(const std::string& host, const std::string& user = "", const std::string& pass = "", const std::string& databd = "", const std::string& charset = "", int port = 0, int flag = 0);
	void init(const DBConf& dbconf);

	void connect();
	void disconnect();

	// 获取数据库指针
	MYSQL* get_mysql();
	// 获取数据库变量
	std::string get_variables(const std::string& name);
	// 字符转义，需要连接到数据库，考虑到了字符集
	std::string real_escape_string(const std::string& from);
	// 更新和插入数据
	void execute(const std::string& sql);

	// mysql的一条记录
	class MySqlRecord
	{
	public:
		MySqlRecord(const std::map<std::string, std::string>& record);
		// 获取记录中的某个字段
		const std::string& operator[](const std::string& field);
	protected:
		const std::map<std::string, std::string>& _record;
	};
	// 查询出来的mysql数据
	class MySqlData
	{
	public:
		// 返回所有数据
		std::vector<std::map<std::string, std::string>>& data();
		// 获取记录条数
		size_t size();
		// 获取某一条记录
		MySqlRecord operator[](size_t idx);

	protected:
		std::vector<std::map<std::string, std::string>> _data;
	};

	// sql语句
	MySqlData query_record(std::string& sql);

	// 数据字段类型
	enum FT
	{
		DB_INT,
		DB_STR
	};

	// 数据记录
	// map本身是一个红黑树，内部对pair节点的key默认排序，且key唯一
	// 而pair只是内部表示键值对的元素<字段名，类型，值>
	using RECORD_DATA = std::map<std::string, const std::pair<FT, std::string>>;

	// 获取查询结果集个数
	size_t get_record_count(const std::string& table_name, const std::string& condition = "");
	// 获取sql返回结果集个数
	size_t get_record_count(const std::string& condition = "");
	// 存在记录
	bool exist_record(const std::string& condition);
	// 获取字段最大值
	int get_max_value(const std::string& table_name, const std::string& field_name, const std::string& condition);
	// 获取最后插入ID
	long last_insert_id();

	// 操作函数
	size_t update_record(const std::string& table_name, const RECORD_DATA& columns, const std::string& condition);
	size_t insert_record(const std::string& table_name, const RECORD_DATA& columns);
	size_t replace_record(const std::string& table_name, const RECORD_DATA& columns);
	size_t update_record(const std::string& table_name, const std::string& condition = "");

	// 构建sql语句
	std::string build_insert_sql(const std::string& table_name, const RECORD_DATA& columns);
	std::string build_replace_sql(const std::string& table_name, const RECORD_DATA& columns);
	std::string build_update_sql(const std::string& table_name, const RECORD_DATA& columns, const std::string& condition);

	std::string get_last_sql();
	size_t get_affected_rows();

	//////////////////////////////////////////////////////////////////////////
	// 字符转义，不考虑字符集（有一定风险）
	static void build_ostringstream(const RECORD_DATA& columns, std::ostringstream& column_names, std::ostringstream& column_values);
	static std::string escape_string_nosafe(const std::string& from);
	static std::string build_insert_sql_nosafe(const std::string& table_name, const RECORD_DATA& columns);
	static std::string build_replace_sql_nosafe(const std::string& table_name, const RECORD_DATA& columns);
	static std::string build_update_sql_nosafe(const std::string& table_name, const RECORD_DATA& columns, const std::string& condition);

private:
	// 数据库指针
	MYSQL* _ptrMysql{ nullptr };
	// 数据库配置
	DBConf	_dbConf;
	// 是否连接
	bool _isConnected{ false };
	// 最后执行的sql
	std::string _lastSql;
};