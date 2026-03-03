# Extensions to build into DuckDB static library
# mysql_sharding: MySQL sharding extension with multi-host support
duckdb_extension_load(mysql_sharding
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/duckdb-mysql-sharding
)
