# duckdb-build

Build DuckDB as a single static library (`libduckdb_bundle.a`) with custom extensions baked in.

## Included Extensions

| Extension | Source |
|-----------|--------|
| autocomplete, core_functions, icu, json, parquet, jemalloc, tpch, tpcds | DuckDB core |
| httpfs | DuckDB core |
| ui | [duckdb/duckdb-ui](https://github.com/duckdb/duckdb-ui) |
| mysql_sharding | [vimt/duckdb-mysql-sharding](https://github.com/vimt/duckdb-mysql-sharding) |

## Build Platforms

| Platform | Runner |
|----------|--------|
| linux-amd64 | ubuntu-24.04 |
| linux-arm64 | ubuntu-24.04-arm |
| darwin-arm64 | macos-latest (Apple Silicon) |
| darwin-amd64 | macos-latest (cross-compile) |

## Output

Each platform produces a zip containing:
- `libduckdb_bundle.a` — single static library with DuckDB + all extensions + all dependencies
- `duckdb.h` — C API header

## Usage

### Trigger Build

- **Auto**: push to main
- **Manual**: Actions → Run workflow (can specify `duckdb_version`, enable `create_release`)

### Use with duckdb-go-bindings

1. Download the zip for your platform from Actions artifacts or Releases
2. Replace files in `duckdb-go-bindings/lib/<platform>/`:
   - Remove all `*.a` files
   - Copy `libduckdb_bundle.a` and `duckdb.h` into the directory
3. The modified `prebuilt.go` links only `-lduckdb_bundle`
