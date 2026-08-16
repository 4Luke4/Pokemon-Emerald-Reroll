# Integration patches

These patches are intentionally small adapters between the current upstream
decompilation and the standalone code under `overlay/`.

| Patch | Hook surface |
| --- | --- |
| `battle.patch` | Party generation, AI/items, battle completion, EXP lock |
| `challenge.patch` | Set style, indoor running, invariant loop, permadeath |
| `hms.patch` | Virtual HM party lookup and Surf availability |
| `items.patch` | Bag and mart capture-item exclusion |
| `scripts.patch` | Pickup, Repel, HM prompt, and event-special hooks |

Each patch must pass `git apply --check`, contain no new source file, and remain
below the audit-size threshold enforced by `scripts/verify-source.py`.
