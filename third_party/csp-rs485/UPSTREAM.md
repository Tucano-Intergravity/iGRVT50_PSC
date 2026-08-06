# csp-rs485 provenance

## Source identity

- Source checkout used for this intake: `C:\PSC\csp-rs485`
- Upstream repository: `https://github.com/yjsong-intergravity/csp-rs485.git`
- Upstream tag: `1.0.0`
- Upstream commit: `56addf6e936e78e3090b43ef5c3c8d60542f3b94`
- Nested libcsp observation: the source commit records
  `third_party/libcsp` as gitlink
  `87006959696c78f70535ab382b0bcd4cb5a6558d` (`v1.6`). The nested checkout
  is not copied or built here; this repository owns its independent official
  top-level libcsp submodule.

The authorization gate and its exact eight-path scope are recorded in
[`docs/evidence/samv71-csp-rs485-verification.md`](../../docs/evidence/samv71-csp-rs485-verification.md).
That evidence records user-confirmed authorization for copying, using, and
redistributing these eight files inside this firmware repository. It does not
assert an unstated upstream license or ownership claim.

## Authorized file manifest

Hashes are SHA-256 over the copied file bytes. Target paths below are relative
to this directory; source paths are relative to the source checkout.

| Target path | Source path | SHA-256 |
| --- | --- | --- |
| `include/csp_rs485_link.h` | `csp_rs485/include/csp_rs485_link.h` | `31d742c857467afcfc7c1a78698b50266cb9a75fb804b4ab17b5cb6cfab2c88e` |
| `include/csp_rs485_port.h` | `csp_rs485/include/csp_rs485_port.h` | `5ba347a46b3657e6b063c91b449fdf3ef2cac218b3495bc5758401f979cd4c58` |
| `include/csp_rs485_profile.h` | `csp_rs485/include/csp_rs485_profile.h` | `6230fcfb0d2da73a03181ec31fdd456c125b9f60d0d97c708ba1127d02434e5a` |
| `src/csp_rs485_internal.h` | `csp_rs485/src/csp_rs485_internal.h` | `0b4ccc41672c3d1bf35cab95b3690a2653db593d1720328e0033e0daac98cd66` |
| `src/csp_rs485_freertos.c` | `csp_rs485/src/csp_rs485_freertos.c` | `3908669622668b477883f398e35409d6c8abe48666e816ad363f895f12ea6c8b` |
| `src/csp_rs485_kiss.c` | `csp_rs485/src/csp_rs485_kiss.c` | `3c68200de98f7afbf2f08d90e975da71e367027e09785d90da32f8133a9702ed` |
| `src/csp_rs485_link.c` | `csp_rs485/src/csp_rs485_link.c` | `d532dda4e430f47479ca9d638b16ef649f2dc93299a8149eecc4748a622544e6` |
| `src/csp_rs485_supervisor.c` | `csp_rs485/src/csp_rs485_supervisor.c` | `0c332a16b444c80cdd33d1bac42eb0a6034618a16b1176527b9415580782129f` |

STM32 ports, CubeMX files, tests, configuration files, and the nested libcsp
checkout are outside this manifest and are not authorized by this intake.

## Upstream verification

The upstream host-test command is:

```sh
make -C tests/host clean test
```

Run it from a writable checkout of the recorded upstream commit. It is not run
against the read-only source checkout during dependency intake because the
command creates and removes build artifacts.

## Synchronization procedure

1. Use a clean, writable checkout of the upstream repository and verify that
   tag `1.0.0` resolves to commit
   `56addf6e936e78e3090b43ef5c3c8d60542f3b94`. If selecting a newer revision,
   record that deliberate change and re-establish authorization before copying.
2. Confirm that the authorization evidence still covers every file to be
   copied. Do not expand this eight-file manifest implicitly.
3. Run `make -C tests/host clean test` in the writable upstream checkout.
4. Copy only the eight source paths in the manifest to their corresponding
   target paths, preserving the file bytes.
5. Recompute each target SHA-256 (for example, with PowerShell
   `Get-FileHash -Algorithm SHA256`) and update the manifest hashes.
6. Confirm that no other `.c` or `.h` file exists below this directory.
7. Stage the dependency snapshot, inspect the staged diff, and confirm that no
   target port, generated file, test fixture, configuration, or nested
   dependency entered the intake. Commit the reviewed snapshot.
8. Run `python tools/verify_csp_vendor.py` from the firmware repository root.
   The verifier intentionally requires `.gitmodules` and `third_party` vendor
   state to agree across committed `HEAD`, the index, and the worktree. If the
   source checkout is elsewhere, pass `--source-root PATH`; when the checkout
   exists, the verifier also requires an empty `git diff --no-index` for every
   source/target pair. Correct any failure and amend or add a follow-up commit
   before treating the synchronized snapshot as verified.
