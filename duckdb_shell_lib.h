#ifndef DUCKDB_SHELL_LIB_H
#define DUCKDB_SHELL_LIB_H

#include "duckdb.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Start the full interactive DuckDB shell on an existing database instance.
 * This provides the complete official shell experience: duckbox rendering,
 * linenoise line editing, syntax highlighting, autocomplete, command history,
 * and all meta-commands (.mode, .timer, .tables, etc.)
 *
 * The database is NOT closed when the shell exits.
 * Returns 0 on success.
 */
int duckdb_start_shell(duckdb_database database);

#ifdef __cplusplus
}
#endif

#endif
