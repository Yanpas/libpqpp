/*
 * Connection.cpp
 *
 *  Created on: Aug 28, 2017
 */

#include <sstream>
#include <vector>
#include "pqpp.hpp"

namespace pqpp {

Connection::Connection(const char* user, const char* dbname, const char* password, const char* address, uint16_t port) {
	std::ostringstream conninfo;
	conninfo << "hostaddr=" << address << " port=" << port
			<< " user=" << user << " password=" << password
			<< " dbname=" << dbname;
	conn_handle_ = PQconnectdb(conninfo.str().c_str());
	if(PQstatus(conn_handle_) != CONNECTION_OK) {
		throw IOError(PQerrorMessage(conn_handle_));
	}
}

Connection::~Connection() {
	PQfinish(conn_handle_);
}

void Connection::exec(const char* query) {
	auto res = PQexec(conn_handle_, query);
	auto status = PQresultStatus(res);
	PQclear(res);
	if(status != PGRES_COMMAND_OK)
		throw InputError(PQerrorMessage(conn_handle_));
}

Statement::Statement(Connection& conn, const std::string& query, std::initializer_list<std::string> params) : conn_(conn) {
	std::vector<const char*> paramvec(params.size());
	{
		size_t i = 0;
		for(auto& el : params)
			paramvec[i++] = el.c_str();
	}
	res_ = PQexecParams(conn_.conn_handle_, query.c_str(), paramvec.size(), nullptr, paramvec.data(), nullptr, nullptr, 0);
	auto status = PQresultStatus(res_);
	if(status == PGRES_BAD_RESPONSE) {
		throw IOError(PQerrorMessage(conn.conn_handle_));
	} else if (status == PGRES_FATAL_ERROR) {
		throw InputError(PQerrorMessage(conn.conn_handle_));
	}
}

Statement::~Statement() {
	if(res_)
		PQclear(res_);
}

Statement::RowIterator::RowIterator(PGresult*& res, std::size_t pos) : pos_(pos), res_(res) {
}

Statement::RowIterator& Statement::RowIterator::operator ++() {
	pos_++;
	return *this;
}

Statement::RowIterator& Statement::RowIterator::operator *() {
	return *this;
}

bool Statement::RowIterator::operator !=(const RowIterator& rhs) const {
	return pos_ != rhs.pos_ || res_ != rhs.res_;
}

const char* Statement::RowIterator::operator [](const char* colname) {
	return this->operator[](PQfnumber(res_, colname));
}

const char* Statement::RowIterator::operator [](int colindex) {
	return PQgetvalue(res_, pos_, colindex);
}

Statement::RowIterator Statement::begin() {
	return RowIterator(res_, 0);
}

Statement::RowIterator Statement::end() {
	return RowIterator(res_, PQntuples(res_));
}

Transaction::Transaction(Connection& conn) : conn_(conn) {
	conn_.exec("BEGIN;");
}

Transaction::~Transaction() {
	if(!commited_)
		conn_.exec("ROLLBACK;");
}

void Transaction::commit() {
	conn_.exec("COMMIT;");
	commited_ = true;
}

} /* namespace pqpp */

