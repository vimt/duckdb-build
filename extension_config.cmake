# Custom DuckDB build with additional extensions
#
# Core extensions (autocomplete, core_functions, icu, json, parquet, jemalloc,
# tpch, tpcds) are included by DuckDB by default.

duckdb_extension_load(httpfs)

duckdb_extension_load(ui
    GIT_URL https://github.com/duckdb/duckdb-ui
    GIT_TAG main
)

duckdb_extension_load(mysql_sharding
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/duckdb-mysql-sharding
)
