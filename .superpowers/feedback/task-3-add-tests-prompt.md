- 测试体系里增加 regression cases 的说明. 具体使用方法参考 tests/regression/README.md
- Unit tests 的具体内容参考 tests/unit/README.md，
- Spec tests 的具体内容参考 tests/wamr-test-suites/README.md

- review tests/regression/README.md, 修正有歧义以及含糊的地方，确保用户和AI都能理解如何使用 regression cases 进行测试，并增加新的cases
- review tests/unit/README.md, 修正有歧义以及含糊的地方， 确保用户和AI都能理解如何使用 unit tests 进行测试，并增加新的cases
- review tests/wamr-test-suites/README.md, 修正有歧义以及含糊的地方， 确保用户和AI都能理解如何使用 spec tests 进行测试。以及如何新增针对新 proposal 的 spec tests

- 正式提交到 upstream 前，需要执行以下步骤。建议将这些步骤添加到 doc/linting.md 中
  - code format check pass
  - unit tests pass
  - spec tests pass
  - regression cases pass
- CONTRIBUTING.md 增加到 linting.md 的链接，确保所有贡献者都能遵守这些检查步骤，以保持代码质量和稳定性。
- AGENTS.md 增加到 linting.md 的链接，确保所有使用 agents 进行测试的用户都能遵守这些检查步骤，以保持代码质量和稳定性。
