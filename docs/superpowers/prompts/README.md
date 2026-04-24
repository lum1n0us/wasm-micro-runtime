# Prompt Templates for Real-World Tasks

这些 prompt 模板用于验证我们的 AI-friendly 基础设施。

## 三个任务模板

### 1. `task-1-bugfix-prompt.md` 🐛
**用途：** 修复真实的 bug
**验证：** 文档完整性、调试工作流、容器集成
**适合：** 有明确 issue 的 bug

### 2. `task-2-pr-review-prompt.md` 📋  
**用途：** 审查真实的 PR
**验证：** 架构理解、审查工作流、测试流程
**适合：** GitHub 上的开放 PR

### 3. `task-3-add-tests-prompt.md` 🧪
**用途：** 为代码添加测试
**验证：** 测试基础设施、文档覆盖、工具链
**适合：** 测试覆盖率低的组件

## 使用方法

### Step 1: 准备任务描述

在使用模板前，你需要：

**对于 Bug Fix:**
- 找到一个真实的 issue
- 复制 issue 链接或描述

**对于 PR Review:**
- 找到一个开放的 PR  
- 复制 PR 链接（格式：https://github.com/bytecodealliance/wasm-micro-runtime/pull/XXXX）

**对于 Add Tests:**
- 确定要测试的组件/函数
- 说明为什么这个组件需要测试

### Step 2: 启动新 Session

为每个任务创建独立的 session：

```bash
# 使用 Claude Code CLI
claude code  # 然后命名 session：
# "WAMR Bug Fix - Issue #1234"
# "WAMR PR Review - PR #5678"  
# "WAMR Add Tests - Memory Allocator"
```

### Step 3: 复制并定制 Prompt

1. 打开对应的 prompt 模板文件
2. 复制整个 "Initial Prompt for New Session" 部分
3. 替换占位符：
   - `[PASTE ISSUE LINK OR DESCRIPTION HERE]`
   - `[PASTE PR LINK HERE]`
   - `[PASTE COMPONENT/FUNCTION/FEATURE HERE]`
4. 粘贴到新 session 中

### Step 4: 让 AI 工作

AI 会：
- 自动读取 AGENTS.md 和相关文档
- 使用 `./scripts/in-container.sh` 执行所有命令
- 跟随文档中的工作流
- 记录 feedback

### Step 5: 收集 Feedback

任务完成后，查看：
- `.superpowers/feedback/task-X-*-feedback.md` (如果 AI 创建了)
- 或者回到 "Make WAMR be friendly to AI" session 报告

## 示例：启动 Bug Fix 任务

```markdown
# 在新 session 中粘贴：

You are helping fix a bug in the WAMR (WebAssembly Micro Runtime) project.

## ⚠️ CRITICAL: Development Environment
This project uses devcontainer for ALL development activities.
ALL commands MUST use: ./scripts/in-container.sh "<command>"

## Your Mission
Fix this bug: https://github.com/bytecodealliance/wasm-micro-runtime/issues/1234

Memory leak in wasm_runtime_malloc when allocation fails

## Getting Started
1. Read `AGENTS.md` completely
2. Follow "For Bug Fixes" section
...
```

## Feedback 收集

### Option A: 在新 session 中创建 feedback 文件

```bash
# AI 在任务结束时创建：
.superpowers/feedback/task-1-bugfix-feedback.md
```

内容包括：
- 哪些文档有用
- 哪些文档缺失
- 容器工具的体验
- 遇到的问题
- 改进建议

### Option B: 口头报告

回到 "Make WAMR be friendly to AI" session，说：

```
Task 1 (Bug Fix) 完成了！Feedback:

Documentation that helped:
- doc/debugging.md 的 GDB 工作流很清晰
- doc/building.md 的 debug build 选项完整

Documentation gaps:
- 缺少常见 bug 模式的说明
- Valgrind 输出的解读不够详细

Container wrapper:
- --verbose 模式对调试很有帮助
- 自动检测和启动容器很流畅

Issues encountered:
- 第一次构建很慢（~2分钟）
- clang-format 的配置文件位置不明确

Suggestions:
1. 添加 "Common Bug Patterns" 文档
2. 在 doc/debugging.md 加入 Valgrind 输出解析
3. 在 doc/building.md 提到首次构建时间
```

## 并行执行

这三个任务可以并行进行：
- 它们相互独立
- 每个在独立的 session 中
- 可以同时验证不同方面

## 成功标准

每个任务完成后应该有：
- ✅ 实际的工作成果（修复/审查/测试）
- ✅ 详细的 feedback 记录
- ✅ 对文档/工具的改进建议

## 下一步

收集完所有 feedback 后：
1. 回到 "Make WAMR be friendly to AI" session
2. 根据 feedback 改进文档和工具
3. 迭代优化系统

---

**记住：** 这些任务的目的不仅仅是完成工作，更重要的是验证和改进我们的 AI-friendly 基础设施。

**祝你好运！** 🚀
