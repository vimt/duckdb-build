// Embedded DuckDB shell library.
// Provides duckdb_start_shell() which starts the full interactive CLI
// on an existing duckdb_database handle.

// Must be compiled with -DHAVE_LINENOISE=1 -DSQLITE_SHELL_IS_UTF8=1
// -DUSE_DUCKDB_SHELL_WRAPPER=1

#include "duckdb_shell_lib.h"
#include "duckdb.hpp"
#include "duckdb/main/capi/capi_internal.hpp"

// sqlite3 API wrapper headers
#include "sqlite3.h"
#include "udf_struct_sqlite3.h"

// Shell headers
#include "shell_state.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>

#if HAVE_LINENOISE
#include "linenoise.hpp"
#endif

using namespace duckdb;

// Shell uses these globals — declared in shell.cpp (which we link against)
extern sqlite3 *duckdb_shell_sqlite3_globalDb;
extern bool duckdb_shell_sqlite3_stdin_is_interactive;
extern bool duckdb_shell_sqlite3_stdout_is_console;
extern bool duckdb_shell_sqlite3_stderr_is_console;

// Forward declarations for functions defined in sqlite3_api_wrapper
extern "C" {
int duckdb_shell_sqlite3_open_v2(const char *, sqlite3 **, int, const char *);
int duckdb_shell_sqlite3_close(sqlite3 *);
int duckdb_shell_sqlite3_errcode(sqlite3 *);
const char *duckdb_shell_sqlite3_errmsg(sqlite3 *);
int duckdb_shell_sqlite3_exec(sqlite3 *, const char *, int (*)(void *, int, char **, char **),
                              void *, char **);
}

extern "C" {

int duckdb_start_shell(duckdb_database database) {
	if (!database) {
		fprintf(stderr, "Error: NULL database handle\n");
		return 1;
	}

	auto wrapper = reinterpret_cast<DatabaseWrapper *>(database);
	DuckDB &duckdb_instance = *wrapper->database;

	// Create a sqlite3 handle wrapping the existing DuckDB instance.
	// We create a Connection from the existing DuckDB, then build a sqlite3
	// struct around it. The sqlite3 struct normally owns its DuckDB via
	// unique_ptr, but we'll manage cleanup manually to avoid destroying
	// the caller's database.
	auto *shell_db = new sqlite3();
	// db stays nullptr — we set con directly
	shell_db->con = make_uniq<Connection>(duckdb_instance);
	shell_db->errCode = 0;

	// Set up terminal detection
	duckdb_shell_sqlite3_stdin_is_interactive = isatty(0);
	duckdb_shell_sqlite3_stdout_is_console = isatty(1);
	duckdb_shell_sqlite3_stderr_is_console = isatty(2);

	// Set up the global pointer used by signal handler and autocomplete
	duckdb_shell_sqlite3_globalDb = shell_db;

	// Initialize shell state
	duckdb_shell::ShellState data;
	data.db = shell_db;
	data.normalMode = data.cMode = data.mode = duckdb_shell::RenderMode::DUCKBOX;
	data.max_rows = 40;
	data.showHeader = true;
	data.out = stdout;

	// Enable progress bar for interactive sessions
	duckdb_shell_sqlite3_exec(shell_db, "PRAGMA enable_progress_bar", nullptr, nullptr, nullptr);
	duckdb_shell_sqlite3_exec(shell_db, "PRAGMA enable_print_progress_bar", nullptr, nullptr, nullptr);

	int rc = 0;

	if (duckdb_shell_sqlite3_stdin_is_interactive) {
		printf("DuckDB %s (%s) %.19s\n"
		       "Enter \".help\" for usage hints.\n",
		       DuckDB::LibraryVersion(), DuckDB::ReleaseCodename(), DuckDB::SourceID());

		// Set up command history
		char *zHistory = nullptr;
		char *zHistoryEnv = getenv("DUCKDB_HISTORY");
		if (zHistoryEnv) {
			zHistory = strdup(zHistoryEnv);
		} else {
			const char *home = getenv("HOME");
			if (home) {
				size_t len = strlen(home) + 30;
				zHistory = (char *)malloc(len);
				if (zHistory) {
					snprintf(zHistory, len, "%s/.duckdb_history", home);
				}
			}
		}

#if HAVE_LINENOISE
		if (zHistory) {
			linenoiseHistoryLoad(zHistory);
		}
		linenoiseSetCompletionCallback(nullptr);
#endif

		data.in = nullptr;
		rc = data.ProcessInput(duckdb_shell::InputMode::STANDARD);

#if HAVE_LINENOISE
		if (zHistory) {
			linenoiseHistorySetMaxLen(2000);
			linenoiseHistorySave(zHistory);
		}
#endif
		if (zHistory) {
			free(zHistory);
		}
	} else {
		data.in = stdin;
		rc = data.ProcessInput(duckdb_shell::InputMode::STANDARD);
	}

	data.SetTableName(nullptr);
	data.ResetOutput();
	data.doXdgOpen = 0;
	data.ClearTempFile();

	// Cleanup: DON'T close the sqlite3 handle normally because it would
	// try to destroy the DuckDB instance. Instead, release the connection
	// and free the struct manually.
	duckdb_shell_sqlite3_globalDb = nullptr;
	data.db = nullptr;
	shell_db->con.reset();
	delete shell_db;

	return rc;
}

} // extern "C"
