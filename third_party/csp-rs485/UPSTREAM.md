# csp-rs485 Upstream Provenance

## Source

- Source checkout: `C:\PSC\csp-rs485`
- Source tag: `1.0.0`
- Source commit: `56addf6e936e78e3090b43ef5c3c8d60542f3b94`
- Nested libcsp gitlink observed in source: `87006959696c78f70535ab382b0bcd4cb5a6558d`
- Target libcsp submodule: `third_party/libcsp`
- Target libcsp commit: `87006959696c78f70535ab382b0bcd4cb5a6558d`
- Target libcsp tag: `v1.6`

## Authorization

The user approved using `C:\PSC\csp-rs485` in this PSC firmware repository on 2026-08-14. The approval covers copying and using the reusable csp-rs485 core files listed below for the SAMV71 RS485 libcsp port.

The source checkout does not contain a top-level `LICENSE` file. Do not import additional csp-rs485 files beyond this manifest unless the authorization/provenance evidence is updated first.

## Imported Manifest

| SHA-256 | Target path | Source path |
|---|---|---|
| `31d742c857467afcfc7c1a78698b50266cb9a75fb804b4ab17b5cb6cfab2c88e` | `third_party/csp-rs485/include/csp_rs485_link.h` | `csp_rs485/include/csp_rs485_link.h` |
| `5ba347a46b3657e6b063c91b449fdf3ef2cac218b3495bc5758401f979cd4c58` | `third_party/csp-rs485/include/csp_rs485_port.h` | `csp_rs485/include/csp_rs485_port.h` |
| `6230fcfb0d2da73a03181ec31fdd456c125b9f60d0d97c708ba1127d02434e5a` | `third_party/csp-rs485/include/csp_rs485_profile.h` | `csp_rs485/include/csp_rs485_profile.h` |
| `3908669622668b477883f398e35409d6c8abe48666e816ad363f895f12ea6c8b` | `third_party/csp-rs485/src/csp_rs485_freertos.c` | `csp_rs485/src/csp_rs485_freertos.c` |
| `0b4ccc41672c3d1bf35cab95b3690a2653db593d1720328e0033e0daac98cd66` | `third_party/csp-rs485/src/csp_rs485_internal.h` | `csp_rs485/src/csp_rs485_internal.h` |
| `3c68200de98f7afbf2f08d90e975da71e367027e09785d90da32f8133a9702ed` | `third_party/csp-rs485/src/csp_rs485_kiss.c` | `csp_rs485/src/csp_rs485_kiss.c` |
| `d532dda4e430f47479ca9d638b16ef649f2dc93299a8149eecc4748a622544e6` | `third_party/csp-rs485/src/csp_rs485_link.c` | `csp_rs485/src/csp_rs485_link.c` |
| `0c332a16b444c80cdd33d1bac42eb0a6034618a16b1176527b9415580782129f` | `third_party/csp-rs485/src/csp_rs485_supervisor.c` | `csp_rs485/src/csp_rs485_supervisor.c` |

## Verification

Run from repository root:

```powershell
python tools/verify_csp_vendor.py
```

Expected:

```text
PASS libcsp=87006959696c78f70535ab382b0bcd4cb5a6558d csp-rs485=8 files
```

The verifier checks:

- `third_party/libcsp` worktree is at the exact expected commit.
- `.gitmodules` points `third_party/libcsp` to the official libcsp repository.
- Every imported csp-rs485 file exists and matches the manifest hash.
- No extra `.c` or `.h` files exist below `third_party/csp-rs485`.
- If `C:\PSC\csp-rs485` exists, each imported file is byte-identical to the source checkout.

## Sync Procedure

1. Confirm updated csp-rs485 authorization before copying any new source.
2. Update only the manifest-listed reusable core files unless a new design explicitly expands the manifest.
3. Recompute SHA-256 hashes.
4. Run `python tools/verify_csp_vendor.py`.
5. Record upstream commit, file list, and verification result in this document.
