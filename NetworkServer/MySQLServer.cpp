#include "MySQLServer.h"

#include "TextUtilities.h"

using namespace std;
using namespace sql;
using namespace ember;
namespace pool = ember::connection_pool;

MySQLServer::MySQLServer(Safe_Print* SafePrint) : SafePrint(SafePrint),
	driver(std::make_unique<drivers::MySQL>("sapp", "Btcc22!", "localhost", 3306, "sapp", "latin1")),
	pool(std::make_unique<pool::Pool<drivers::MySQL, pool::CheckinClean, pool::ExponentialGrowth>>(*driver, 4, 128, std::chrono::seconds(10)))
{
}

MySQLServer::~MySQLServer()
{
}

void MySQLServer::PrintExceptionMessage(const SQLException &e)
{
	SafePrint->Print(FormatArgs("Error: %hs, MySQL error code: 0x%08X\n", e.what(), e.getErrorCode()));
}

bool MySQLServer::Query(const string &query_str) try
{
	auto connection = pool->wait_connection();
	std::unique_ptr<Statement> stmt(connection->createStatement());
	stmt->execute(query_str);
	return true;
}
catch (const SQLException &e)
{
	PrintExceptionMessage(e);
	return false;
}

//ULONGLONG MySQLServer::QueryID(const char* query_str)
//{
//	ULONGLONG id = 0;
//
//	if(connection)
//	{
//		MySQLLock.Lock();
//
//		Statement *stmt = nullptr;
//
//		try
//		{
//			stmt = connection->createStatement();
//			stmt->execute(query_str);
//			//id = mysql_insert_id(connection);
//			//SELECT LAST_INSERT_ID();
//		}
//		catch (const SQLException &e)
//		{
//			PrintExceptionMessage(e);
//		}
//
//		delete stmt;
//
//		MySQLLock.Unlock();
//	}
//
//	return id;
//}
bool MySQLServer::QueryResult(const string &query_str, vector<string> &res, size_t res_count) try
{
	auto connection = pool->wait_connection();
	std::unique_ptr<Statement> stmt(connection->createStatement());
	std::unique_ptr<ResultSet> result(stmt->executeQuery(query_str));

	if (result->next())
	{
		for (size_t i = 1; i <= res_count; i++)
		{
			res.emplace_back(result->getString(i));
		}
	}
	return true;
}
catch (const SQLException &e)
{
	PrintExceptionMessage(e);
	return false;
}

bool MySQLServer::PreparedStatement(const string &query_str, const vector<string> &vec) try
{
	auto connection = pool->wait_connection();
	sql::PreparedStatement* prep_stmt = driver->prepare_cached(*connection, query_str);

	for (size_t i = 0; i < vec.size(); i++)
	{
		prep_stmt->setString(i + 1, vec[i].c_str());
	}

	prep_stmt->execute();
	return true;
}
catch (const SQLException &e)
{
	PrintExceptionMessage(e);
	return false;
}

bool MySQLServer::QueryResultPreparedStatement(const string &query_str, const vector<string> &vec, vector<string> &res, size_t res_count) try
{
	auto connection = pool->wait_connection();
	sql::PreparedStatement* prep_stmt = driver->prepare_cached(*connection, query_str);

	for (size_t i = 0; i < vec.size(); i++)
	{
		prep_stmt->setString(i + 1, vec[i].c_str());
	}

	std::unique_ptr<ResultSet> result(prep_stmt->executeQuery());

	if (result->next())
	{
		for (size_t i = 1; i <= res_count; i++)
		{
			res.emplace_back(result->getString(i));
		}
	}
	return true;
}
catch (const SQLException &e)
{
	PrintExceptionMessage(e);
	return false;
}