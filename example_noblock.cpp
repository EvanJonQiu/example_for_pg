/*
 * reference https://www.postgresql.org/docs/current/libpq-example.html
 */

#include <iostream>
#include <pthread.h>
#include <unistd.h>

#include "libpq-fe.h"

static void *work_func(void *params);
void displayStatus(ConnStatusType status);
void displayPollingStatusType(PostgresPollingStatusType type);

int main(int argc, char *argv[])
{
  pthread_t thread;
  PGconn *conn;

  std::string conninfo = "dbname=test user=test password=admin_123 sslmode=disable";

  conn = PQconnectStart(conninfo.c_str());

  displayStatus(PQstatus(conn));
  
  if (PQsetnonblocking(conn, 1) == -1) {
    std::cerr << "set non block failed: " << PQerrorMessage(conn) << std::endl;
    PQfinish(conn);
    return 1;
  }
  displayStatus(PQstatus(conn));

  pthread_create(&thread, NULL, work_func, conn);

  pthread_join(thread, NULL);

  displayStatus(PQstatus(conn));

  PQfinish(conn);
  
  return 0;
}

static void *work_func(void *params) {
  PGconn *conn = (PGconn *)params;
  
  std::cout << "in thread" << std::endl;

  PostgresPollingStatusType statusType = PQconnectPoll(conn);
  while (statusType != PGRES_POLLING_OK) {
    displayPollingStatusType(statusType);
    statusType = PQconnectPoll(conn);
  }

  int ret = PQsendQuery(conn, "SELECT * FROM public.item_info");
  if (ret == 0) {
    std::cerr << "send query failed: " << PQerrorMessage(conn) << std::endl;
    return NULL;
  }

  int sock = PQsocket(conn);
  fd_set input_mask;

  FD_ZERO(&input_mask);
  FD_SET(sock, &input_mask);

  while (1) {
    int n = select(sock + 1, &input_mask, NULL, NULL, NULL);
    if (n == 0) {
      continue;
    } else if (n == -1) {
      break;
    } else {
      if (!PQconsumeInput(conn)) {
        std::cerr << "consume input failed" << std::endl;
        break;
      }
      while (PQisBusy(conn)) {
        std::cout << "still receiving data" << std::endl;
        sleep(2);
      }
      PGresult *result = PQgetResult(conn);
      if (result != NULL) {
        std::cout << "get data" << std::endl;

        int nFields = PQnfields(result);
        for (int i = 0; i < nFields; i++) {
          std::cout << PQfname(result, i) << std::endl;
        }

        for (int i = 0; i < PQntuples(result); ++i) {
          for (int j = 0; j < nFields; ++j) {
            std::cout << PQgetvalue(result, i, j) << std::endl;
          }
        }
      }
      /*
      while ((result = PQgetResult(conn)) != NULL) {
        std::cout << "still result" << std::endl;
        sleep(2);
      }
      */
      PQclear(result);
      
      std::cout << "should read data" << std::endl;
      break;
    }
  }

  std::cout << "end thread" << std::endl;

  return NULL;
}

void displayStatus(ConnStatusType status) {
  switch (status) {
  case CONNECTION_OK:
    std::cout << "OK" << "\n";
    break;
  case CONNECTION_BAD:
    std::cout << "BAD" << "\n";
    break;
  case CONNECTION_STARTED:
    std::cout << "Waiting for connection to be made." << "\n";
    break;
  case CONNECTION_MADE:
    std::cout << "Connection OK; waiting to send." << "\n";
    break;
  case CONNECTION_AWAITING_RESPONSE:
    std::cout << "Waiting for a response from the postmaster." << "\n";
    break;
  case CONNECTION_AUTH_OK:
    std::cout << "Received authentication; waiting for backend startup." << "\n";
    break;
  case CONNECTION_SETENV:
    std::cout << "This state is no longer used." << "\n";
    break;
  case CONNECTION_SSL_STARTUP:
    std::cout << "Negotiating SSL." << "\n";
    break;
  case CONNECTION_NEEDED:
    std::cout << "Internal state: connect() needed" << "\n";
    break;
  case CONNECTION_CHECK_WRITABLE:
    std::cout << "Checking if session is read-write." << "\n";
    break;
  case CONNECTION_CONSUME:
    std::cout << "Consuming any extra messages." << "\n";
    break;
  case CONNECTION_GSS_STARTUP:
    std::cout << "Negotiating GSSAPI." << "\n";
    break;
  case CONNECTION_CHECK_TARGET:
    std::cout << "Checking target server properties." << "\n";
    break;
  default:
    std::cout << "unknown status: " << (int)status << "\n";
    break;
  }
}

void displayPollingStatusType(PostgresPollingStatusType type) {
  switch (type) {
  case PGRES_POLLING_FAILED:
    std::cout << "pool status: failed" << "\n";
    break;
  case PGRES_POLLING_READING:
    std::cout << "pool status: read" << "\n";
    break;
  case PGRES_POLLING_WRITING:
    std::cout << "pool status: write" << "\n";
    break;
  case PGRES_POLLING_OK:
    std::cout << "pool status: polling ok" << "\n";
    break;
  case PGRES_POLLING_ACTIVE:
    std::cout << "pool status: polling active" << "\n";
    break;
  default:
    std::cout << "pool status: unknown polling status" << "\n";
    break;
  }
}
