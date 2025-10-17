import { MCPServer } from "@modelcontextprotocol/sdk/server/index.js";
import { StdioServerTransport } from "@modelcontextprotocol/sdk/server/stdio.js";
import { Tool } from "@modelcontextprotocol/sdk/types.js";
import { spawn } from "node:child_process";
import path from "node:path";
import { fileURLToPath } from "node:url";
import process from "node:process";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const repoRoot = path.resolve(__dirname, "../../..");

function runCmd(cmd, args, options = {}) {
  return new Promise((resolve, reject) => {
    const child = spawn(cmd, args, { cwd: repoRoot, stdio: ["ignore", "pipe", "pipe"], shell: false, ...options });
    let stdout = "";
    let stderr = "";
    child.stdout.on("data", (d) => (stdout += d.toString()));
    child.stderr.on("data", (d) => (stderr += d.toString()));
    child.on("error", (err) => reject(err));
    child.on("close", (code) => {
      if (code === 0) resolve({ code, stdout, stderr });
      else reject(Object.assign(new Error(`Command failed: ${cmd} ${args.join(" ")}`), { code, stdout, stderr }));
    });
  });
}

const composeFile = path.join(repoRoot, "docker", "docker-compose.yml");
const compose = (...args) => runCmd("docker", ["compose", "-f", composeFile, ...args]);

async function upTool() {
  const res = await compose("up", "-d");
  return { content: [{ type: "text", text: res.stdout || "Compose up done" }] };
}

async function downTool() {
  const res = await compose("down", "-v");
  return { content: [{ type: "text", text: res.stdout || "Compose down done" }] };
}

async function statusTool() {
  const res = await compose("ps");
  return { content: [{ type: "text", text: res.stdout }] };
}

async function logsTool({ service = "dovecot", tail = 200 }) {
  const res = await compose("logs", "--tail", String(tail), service);
  return { content: [{ type: "text", text: res.stdout || res.stderr }] };
}

const buildContainer = "build"; // name from docker-compose.yml (container_name: 'build')

async function execInBuild(argsArray, opts = {}) {
  // Mounts repo at /repo in build container per docker-compose.yml
  const res = await runCmd("docker", ["exec", "-i", buildContainer, ...argsArray], { ...opts });
  return res;
}

async function buildTool({ clean = false, configureArgs = [] } = {}) {
  const steps = [];
  // Ensure services up
  await compose("up", "-d");
  if (clean) {
    steps.push(await execInBuild(["bash", "-lc", "cd /repo && git clean -fdx || true"]));
  }
  // Always ensure submodules (e.g., googletest) are initialized and updated
  steps.push(await execInBuild(["bash", "-lc", "cd /repo && git submodule update --init --recursive || true"]));
  // Fallback: if googletest sources still missing, force shallow clone (build-only convenience)
  steps.push(await execInBuild(["bash", "-lc", "cd /repo && if [ ! -f src/tests/googletest/googletest/src/gtest-all.cc ]; then echo 'googletest submodule incomplete; performing fallback shallow clone'; rm -rf src/tests/googletest; git clone --depth=1 https://github.com/google/googletest.git src/tests/googletest; fi"]));
  steps.push(await execInBuild(["bash", "-lc", "cd /repo && ./autogen.sh"]));
  // Default to workflow-like configure if none supplied
  const defaultArgs = [
    "--with-dovecot=/usr/local/lib/dovecot",
    "--enable-maintainer-mode",
    "--enable-debug",
    "--with-integration-tests",
    "--enable-valgrind",
    "--enable-debug"
  ];
  const finalArgs = configureArgs.length ? configureArgs : defaultArgs;
  const cfg = ["./configure", ...finalArgs].join(" ");
  steps.push(await execInBuild(["bash", "-lc", `cd /repo && ${cfg}`]));
  steps.push(await execInBuild(["bash", "-lc", "cd /repo && make -j$(nproc)"]));
  const text = steps.map((s) => s.stdout).join("\n");
  return { content: [{ type: "text", text }] };
}

async function testTool({ withIntegration = false } = {}) {
  await compose("up", "-d");
  const cfgArg = withIntegration ? "--with-integration-tests=yes" : "";
  const commands = [
    "set -e",
    "cd /repo",
    // Ensure submodules before building tests
    "git submodule update --init --recursive || true",
    // Fallback shallow clone if needed (tests require real sources)
    "if [ ! -f src/tests/googletest/googletest/src/gtest-all.cc ]; then echo 'googletest submodule incomplete; performing fallback shallow clone'; rm -rf src/tests/googletest; git clone --depth=1 https://github.com/google/googletest.git src/tests/googletest; fi",
    "./autogen.sh",
    `./configure ${cfgArg}`.trim(),
    "make -j$(nproc)",
    "make check -j$(nproc)"
  ].join(" && ");
  const res = await execInBuild(["bash", "-lc", commands]);
  return { content: [{ type: "text", text: res.stdout || "Tests finished" }] };
}

async function dovecotBuildTool({ branch = "2.3.15" } = {}) {
  await compose("up", "-d");
  const cmds = [
    "set -e",
    "cd /usr/local/src/dovecot",
    "git fetch origin",
    `git checkout ${branch}`,
    "./autogen.sh",
    "./configure --enable-maintainer-mode --enable-devel-checks --with-zlib",
    "make install"
  ].join(" && ");
  const res = await execInBuild(["bash", "-lc", cmds]);
  return { content: [{ type: "text", text: res.stdout || "Dovecot built and installed" }] };
}

async function valgrindTestsTool() {
  await compose("up", "-d");
  const cmds = [
    "set -e",
    "cd /repo",
    "valgrind src/tests/test_storage_mock_rbox_bugs",
    "valgrind src/tests/test_librmb_utils"
  ].join(" && ");
  const res = await execInBuild(["bash", "-lc", cmds]);
  return { content: [{ type: "text", text: res.stdout || "Valgrind tests completed" }] };
}

async function execTool({ cmd = "bash", args = [] } = {}) {
  const res = await execInBuild([cmd, ...args]);
  return { content: [{ type: "text", text: (res.stdout || "").toString() }] };
}

const tools = [
  {
    name: "compose_up",
    description: "Start Ceph and build containers via docker compose",
    inputSchema: { type: "object", properties: {}, additionalProperties: false }
  },
  {
    name: "compose_down",
    description: "Stop and remove compose services and volumes",
    inputSchema: { type: "object", properties: {}, additionalProperties: false }
  },
  {
    name: "compose_status",
    description: "Show compose services status",
    inputSchema: { type: "object", properties: {}, additionalProperties: false }
  },
  {
    name: "compose_logs",
    description: "Show logs for a compose service (default dovecot/build)",
    inputSchema: {
      type: "object",
      properties: { service: { type: "string" }, tail: { type: "integer" } },
      additionalProperties: false
    }
  },
  {
    name: "plugin_build",
    description: "Run autogen, configure, and make inside the build container",
    inputSchema: {
      type: "object",
      properties: {
        clean: { type: "boolean" },
        configureArgs: { type: "array", items: { type: "string" } }
      },
      additionalProperties: false
    }
  },
  {
    name: "plugin_test",
    description: "Build and run tests (make check); set withIntegration to enable integration tests",
    inputSchema: {
      type: "object",
      properties: { withIntegration: { type: "boolean" } },
      additionalProperties: false
    }
  },
  {
    name: "dovecot_build",
    description: "Build and install Dovecot inside the build container (default branch 2.3.15)",
    inputSchema: {
      type: "object",
      properties: { branch: { type: "string" } },
      additionalProperties: false
    }
  },
  {
    name: "plugin_valgrind_tests",
    description: "Run valgrind on specific tests similar to CI workflow",
    inputSchema: { type: "object", properties: {}, additionalProperties: false }
  },
  {
    name: "build_exec",
    description: "Execute an arbitrary command inside the build container",
    inputSchema: {
      type: "object",
      properties: { cmd: { type: "string" }, args: { type: "array", items: { type: "string" } } },
      additionalProperties: false
    }
  }
];

const server = new MCPServer({ name: "mcp-dovecot-builder", version: "0.1.0" }, { tools });

server.tool("compose_up", async () => upTool());
server.tool("compose_down", async () => downTool());
server.tool("compose_status", async () => statusTool());
server.tool("compose_logs", async (args) => logsTool(args));
server.tool("plugin_build", async (args) => buildTool(args));
server.tool("plugin_test", async (args) => testTool(args));
server.tool("build_exec", async (args) => execTool(args));
server.tool("dovecot_build", async (args) => dovecotBuildTool(args));
server.tool("plugin_valgrind_tests", async () => valgrindTestsTool());

const transport = new StdioServerTransport();
await server.connect(transport);
