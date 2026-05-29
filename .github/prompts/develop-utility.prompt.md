---
description: "Develop a new utility in the opcua-server framework from a short specification"
name: "Develop Utility"
argument-hint: "Utility goal, inputs/outputs, and constraints"
agent: "agent"
---
Create a production-ready utility inside this `opcua-server` framework.

Input:
- User request: ${input}

Instructions:
1. Infer the best location for the utility from the repository structure and existing patterns.
2. Implement the utility with minimal, targeted changes and preserve current architecture and style.
3. Reuse existing helpers/modules before introducing new abstractions.
4. Add or update tests when there is an existing testing pattern for the affected area.
5. Validate by building or running relevant checks, then report what was verified.
6. If requirements are ambiguous, ask only the smallest set of clarifying questions.

Output format:
- `Summary`: what utility was implemented and why.
- `Files changed`: list each file and purpose.
- `Validation`: commands/checks run and key results.
- `Follow-ups`: optional next improvements.
