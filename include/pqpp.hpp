/*
 *  pqpp
 *
 *  Created on: Aug 28, 2017
 */

#ifndef PQPP_HPP_
#define PQPP_HPP_

#include <cinttypes>
#include <exception>
#include <string>
#include <initializer_list>

#include <postgresql/libpq-fe.h>

namespace pqpp {

class Exception : public std::exception {
public:
	virtual const char* what() const noexcept override {
		return description_.c_str();
	}
protected:
	Exception(std::string description) : description_(std::move(description)) {}
private:
	std::string description_;	
};

class IOError : public Exception {
public:
	IOError(std::string description = {}) : Exception(std::move(description)) {}
};

class InputError : public Exception {
public:
	InputError(std::string description = {}) : Exception(std::move(description)) {}
};

class Statement;

class Connection {
	friend class Statement;
public:
	Connection(const char* user, const char* dbname, const char* password, const char* address, uint16_t port = 5432);
	~Connection();
	void exec(const char* query);
private:
	PGconn* conn_handle_;
};

class Statement {
public:
	class RowIterator {
	public:
		RowIterator(PGresult*& res, std::size_t pos);
		RowIterator& operator++();
		RowIterator& operator*();
		bool operator!=(const RowIterator& rhs) const;
		const char* operator[](const char* colname);
		const char* operator[](int colindex);
	private:
		std::size_t pos_;
		PGresult*& res_;
	};

	Statement(Connection& conn, const std::string& query, std::initializer_list<std::string> params = {});
	~Statement();
	RowIterator begin();
	RowIterator end();
private:
	Connection& conn_;
	PGresult* res_ = nullptr;
};

class Transaction {
public:
	Transaction(Connection& conn);
	~Transaction();
	void commit();
private:
	Connection& conn_;
	bool commited_ = false;
};

} /* namespace pqpp */

#endif /* PQPP_HPP_ */
