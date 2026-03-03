# duckdb-build

Build DuckDB static libraries with custom extensions for use with [duckdb-go-bindings](https://github.com/duckdb/duckdb-go-bindings).

## Included Extensions

**Core** (built-in by DuckDB):
- autocomplete, core_functions, icu, json, parquet, jemalloc, tpch, tpcds

**Custom**:
- [mysql_sharding](https://github.com/vimt/duckdb-mysql-sharding) - MySQL sharding extension with multi-host support

## Build Platforms

| Platform | Architecture |
|----------|-------------|
| linux-amd64 | x86_64 glibc |
| linux-arm64 | aarch64 glibc |
| darwin-arm64 | Apple Silicon |
| darwin-amd64 | Intel Mac |

## Usage

### GitHub Actions

Push to trigger a build, or use **Actions → Run workflow** to build manually.

Set `create_release: true` to create a GitHub release with downloadable zip files.

### Using with Go

Replace the `.a` files in your local `duckdb-go-bindings/lib/<platform>/` directory with the ones from the build artifacts, and update the `prebuilt.go` CGO LDFLAGS to include the additional libraries.
