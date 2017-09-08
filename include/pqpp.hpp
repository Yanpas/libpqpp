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
	Connection(const std::string& conn_str);
	Connection(const char* user, const char* dbname, const char* password, const char* address, uint16_t port = 5432);
	~Connection();
	void exec(const char* query);
private:
	PGconn* conn_handle_;
	void connect(const std::string& conn_str);
};

#include <type_traits>

/*
 * Statement with parameter bindings support. Executes in constructor
 */
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
	
	class Val {
	public:
		template <typename Int, class = typename std::enable_if<std::is_integral<Int>::value>::type>
		Val(Int val) {
			repr_ = std::to_string(val);
		}
		
		Val(std::string val) : repr_(std::move(val)) {}
		Val(const char* val) : repr_(val) {}
		Val(char val) : repr_{val} {}
		const char* data() const {
			return repr_.c_str();
		}
	private:
		std::string repr_;
	};

	Statement(Connection& conn, const std::string& query, std::initializer_list<Val> params = {});
	~Statement();
	RowIterator begin();
	RowIterator end();
	std::size_t getRowsN() const {
		return rows_n;
	}
private:
	Connection& conn_;
	PGresult* res_ = nullptr;
	std::size_t rows_n;
};

/*
 * Simple RAII wrapper
 */
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
