#pragma once

#pragma comment (lib, "mysqlcppconn.lib")

#include <memory>
#include <string>
#include <vector>

#include "cppconn/driver.h"
#include "cppconn/connection.h"
#include "cppconn/exception.h"
#include "cppconn/resultset.h"
#include "cppconn/statement.h"
#include "cppconn/prepared_statement.h"

#include "pool/ConnectionPool.h"
#include "pool/drivers/MySQLDriver.h"
#include "pool/Policies.h"

#include "SafePrint.h"

class MySQLServer
{
	std::unique_ptr<ember::drivers::MySQL> driver;
	std::unique_ptr<ember::connection_pool::Pool<ember::drivers::MySQL, ember::connection_pool::CheckinClean, ember::connection_pool::ExponentialGrowth>> pool;
	
	void PrintExceptionMessage(const sql::SQLException &e);

	Safe_Print* SafePrint;

public:
	MySQLServer(Safe_Print* SafePrint);
	~MySQLServer();

	bool Query(const std::string &query_str);
	bool QueryResult(const std::string &query_str, std::vector<std::string> &res, size_t res_count);
	bool PreparedStatement(const std::string &query_str, const std::vector<std::string> &vec);
	bool QueryResultPreparedStatement(const std::string &query_str, const std::vector<std::string> &vec, std::vector<std::string> &res, size_t res_count);
};