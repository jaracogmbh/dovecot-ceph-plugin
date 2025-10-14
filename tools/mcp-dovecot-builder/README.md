# MCP Dovecot Builder

An MCP server that exposes tools to build and test the dovecot-ceph-plugin using the provided Docker Compose setup.

## Prerequisites

- Docker and Docker Compose (uses `docker compose` CLI)
- The repository’s `docker/docker-compose.yml` is unchanged regarding service names and volumes
- Node.js 18+

## Install

From this directory:

```sh
npm install
```

## Run (as MCP server)

This server speaks over stdio. Your MCP client should launch it via:

```json
{
  "mcpServers": {
    "dovecot-builder": {
      "command": "node",
      "args": ["tools/mcp-dovecot-builder/src/server.js"],
      "cwd": "${workspaceFolder}"
    }
  }
}
```

## Tools

- compose_up: start the Ceph and build containers
- compose_down: stop and remove containers and named volumes
- compose_status: list compose services
- compose_logs: show logs for a service (defaults to `dovecot`, container named `build`)
- plugin_build: run autogen/configure/make inside the build container (optionally `clean` and pass `configureArgs`)
- plugin_test: run make check, optionally enabling integration tests with `withIntegration: true`
- build_exec: run any command inside the build container

## Notes

- The build container is named `build` in `docker-compose.yml`. The repo is mounted at `/repo` inside that container.
- The Ceph service uses `ceph/daemon:latest` with a demo configuration; adapt as needed for local testing.
- If configure needs custom flags, use `configureArgs`, e.g. `{ "configureArgs": ["--with-tests=yes","--with-storage=yes"] }`.
