#include "duckdb_shell_lib.h"
#include "duckdb.hpp"
#include "duckdb/main/capi/capi_internal.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace duckdb;

static void shell_print_version() {
	printf("DuckDB %s (embedded shell)\n"
	       "Type \".quit\" to exit, \".help\" for usage hints.\n",
	       DuckDB::LibraryVersion());
}

static void shell_print_help() {
	printf("Meta-commands:\n"
	       "  .quit / .exit    Exit this shell\n"
	       "  .tables          List all tables\n"
	       "  .schema [TABLE]  Show CREATE statements\n"
	       "  .databases       List attached databases\n"
	       "  .timer on|off    Toggle query timer\n"
	       "  .help            Show this help\n"
	       "\n"
	       "SQL statements end with a semicolon (;).\n"
	       "Multi-line input is supported.\n");
}

static bool shell_handle_meta(Connection &con, const char *line, bool &show_timer) {
	std::string cmd(line);

	if (cmd == ".quit" || cmd == ".exit") {
		return false;
	}

	if (cmd == ".help") {
		shell_print_help();
		return true;
	}

	if (cmd == ".tables") {
		auto result = con.Query(
		    "SELECT table_name FROM information_schema.tables "
		    "WHERE table_schema NOT IN ('information_schema', 'pg_catalog') "
		    "ORDER BY table_catalog, table_schema, table_name");
		if (result->HasError()) {
			fprintf(stderr, "Error: %s\n", result->GetError().c_str());
		} else {
			result->Print();
		}
		return true;
	}

	if (cmd == ".databases") {
		auto result = con.Query("PRAGMA database_list");
		if (result->HasError()) {
			fprintf(stderr, "Error: %s\n", result->GetError().c_str());
		} else {
			result->Print();
		}
		return true;
	}

	if (cmd.rfind(".schema", 0) == 0) {
		std::string sql;
		if (cmd.size() > 8) {
			std::string table_name = cmd.substr(8);
			sql = "SELECT sql FROM sqlite_master WHERE name = '" + table_name + "'";
		} else {
			sql = "SELECT sql FROM sqlite_master ORDER BY name";
		}
		auto result = con.Query(sql);
		if (result->HasError()) {
			fprintf(stderr, "Error: %s\n", result->GetError().c_str());
		} else {
			result->Print();
		}
		return true;
	}

	if (cmd == ".timer on") {
		show_timer = true;
		printf("Timer enabled.\n");
		return true;
	}
	if (cmd == ".timer off") {
		show_timer = false;
		printf("Timer disabled.\n");
		return true;
	}

	fprintf(stderr, "Unknown command: %s\nUse \".help\" for help.\n", line);
	return true;
}

extern "C" {

int duckdb_start_shell(duckdb_database database) {
	if (!database) {
		fprintf(stderr, "Error: NULL database handle\n");
		return 1;
	}

	auto wrapper = reinterpret_cast<DatabaseWrapper *>(database);
	DuckDB &db = *wrapper->database;
	Connection con(db);

	shell_print_version();

	char line_buf[65536];
	std::string sql_buffer;
	bool show_timer = false;

	while (true) {
		const char *prompt = sql_buffer.empty() ? "D " : "  ";
		fprintf(stdout, "%s", prompt);
		fflush(stdout);

		if (!fgets(line_buf, sizeof(line_buf), stdin)) {
			if (!sql_buffer.empty()) {
				fprintf(stderr, "\nIncomplete SQL statement discarded.\n");
			}
			break;
		}

		size_t len = strlen(line_buf);
		while (len > 0 && (line_buf[len - 1] == '\n' || line_buf[len - 1] == '\r')) {
			line_buf[--len] = '\0';
		}

		if (len == 0 && sql_buffer.empty()) {
			continue;
		}

		if (sql_buffer.empty() && line_buf[0] == '.') {
			if (!shell_handle_meta(con, line_buf, show_timer)) {
				break;
			}
			continue;
		}

		if (!sql_buffer.empty()) {
			sql_buffer += "\n";
		}
		sql_buffer += line_buf;

		// Check if SQL ends with semicolon
		size_t end = sql_buffer.size();
		while (end > 0 && (sql_buffer[end - 1] == ' ' || sql_buffer[end - 1] == '\t')) {
			end--;
		}
		if (end == 0 || sql_buffer[end - 1] != ';') {
			continue;
		}

		try {
			auto t0 = std::chrono::steady_clock::now();
			auto result = con.Query(sql_buffer);
			auto t1 = std::chrono::steady_clock::now();

			if (result->HasError()) {
				fprintf(stderr, "Error: %s\n", result->GetError().c_str());
			} else {
				result->Print();
				if (show_timer) {
					auto ms = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
					printf("Run Time: %.3f s\n", ms / 1e6);
				}
			}
		} catch (std::exception &e) {
			fprintf(stderr, "Error: %s\n", e.what());
		}

		sql_buffer.clear();
	}

	return 0;
}

} // extern "C"
