/*
 * reference https://www.postgresql.org/docs/current/libpq-example.html
 */

#include <iostream>

#include "libpq-fe.h"

int main(int argc, char *argv[])
{

  std::string conninfo = "dbname=test user=test password=admin_123";

  PGconn *conn = PQconnectdb(conninfo.c_str());
  int status = PQstatus(conn);
  if (status != CONNECTION_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    return 1;
  } else {
    std::cout << "connect pg server success" << std::endl;
  }

  PGresult *res = PQexec(conn, "SELECT pg_catalog.set_config('search_path', '', false)");
  if (PQresultStatus(res) !=PGRES_TUPLES_OK) {
    std::cerr << "SET failed: " << PQerrorMessage(conn);
    PQfinish(conn);
    return 1;
  }

  PQclear(res);

  res = PQexec(conn, "BEGIN");
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    std::cerr << "Begin command failed: " << PQerrorMessage(conn);
    PQclear(res);
    PQfinish(conn);
    return 1;
  }

  res = PQexec(conn, "DECLARE myportal CURSOR FOR select * from public.item_info");
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    std::cerr << "DECLARE CURSOR failed: " << PQerrorMessage(conn);
    PQclear(res);
    PQfinish(conn);
    return 1;
  }
  PQclear(res);

  res = PQexec(conn, "FETCH ALL in myportal");
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    std::cerr << "FETCH All failed: " << PQerrorMessage(conn);
    PQclear(res);
    PQfinish(conn);
    return 1;
  }

  int nFields = PQnfields(res);
  for (int i = 0; i < nFields; i++) {
    std::cout << PQfname(res, i);
  }
  std::cout << "\n\n";

  for (int i = 0; i < PQntuples(res); i++) {
    for (int j = 0; j < nFields; j++) {
      std::cout << PQgetvalue(res, i, j);
    }
    std::cout << "\n";
  }

  PQclear(res);

  res = PQexec(conn, "Close myportal");
  PQclear(res);
  
  res = PQexec(conn, "END");
  PQclear(res);
  
  PQfinish(conn);
  return 0;
}
