#ifndef DUCKDB_SHELL_LIB_H
#define DUCKDB_SHELL_LIB_H

#include "duckdb.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Start an interactive DuckDB shell on an existing database instance.
 * The database is NOT closed when the shell exits.
 * Returns 0 on success.
 */
int duckdb_start_shell(duckdb_database database);

#ifdef __cplusplus
}
#endif

#endif
