/*
 * test.cpp
 *
 *  Created on: Aug 28, 2017
 */

#include "pqpp.hpp"
#include <iostream>

int main() {
	using namespace pqpp;
	Connection con("yan", "yan", "", "127.0.0.1");
	con.exec("DROP TABLE IF EXISTS sample;");
	con.exec("CREATE TABLE sample (num INT, txt TEXT);");
	Statement s1(con, "INSERT INTO sample (num, txt) VALUES ($1, $2), ($3, $4);", {"1", "First", "2", "Second"});
	Statement s2(con, "SELECT num, txt FROM sample;");
	for(auto& row : s2) {
		std::cout << "num=" << row[0] << " txt=" << row["txt"] << '\n';
	}
	//Statement s3(con, "SELECT (1/0);");
}

