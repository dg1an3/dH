# Branches

Notes on the local/remote branches in this repository. `main` is the integration
branch; feature and bugfix branches are merged via PR and then deleted. The
branches below are **unmerged and stale** — they are kept only for reference and
should not be assumed to contain wanted work.

Last reviewed: 2026-07-17.

## Active

- **`main`** — the integration branch. All shipped work lands here via pull
  request (recent examples: the optimizer crash fixes, the variational
  free-energy / entropy regularizer, and the WebView2 convergence chart).

## Unmerged, stale branches (~5 months old, do not merge as-is)

These predate a large amount of subsequent work on `main`. None of them need a
PR: their useful content is already in `main`, and their unique content was
either not adopted or is now far too far behind to merge cleanly.

- **`local-main-changes`** (tip `269175a`) — adds a `CVectorN` class /
  `VectorOps` and XML-logging classes (`CXMLLogFile`, `CXMLElement`).
  **Superseded**: both `RtModel/include/VectorN.h` and the `XMLLogging/`
  project are already present in `main` (they arrived via other history). No
  unique content. Local-only (no upstream).

- **`reorganize-classic-libs`** (tip `cd2e9e9`, also on `origin`) — the same
  CVectorN/XML-logging commits plus a repository reorganization that moves the
  "classic" libraries into `lib_classic/` and introduces a `src/` layout.
  **Not adopted**: `main` has no `lib_classic/` or `src/` structure; the reorg
  was abandoned. The remaining commits are already in `main`.

- **`modernize-build-deps`** (tip `1c178c0`) — "replace IPP with a C++
  implementation, use vcpkg ITK 5.4". **Dangerously stale**: `main` already uses
  vcpkg ITK 5.4, and this branch's diff against `main` is roughly
  **+256 / −21,315 lines across 269 files** — merging it would *delete* large
  amounts of current work (including the Python test suite and recently added
  gradient tests). If the IPP removal is ever wanted, redo it fresh against
  today's `main` rather than resurrecting this branch. Local-only (no upstream).

### Recommendation

Delete these branches (and the `origin/reorganize-classic-libs` remote) once
their reference value has been captured here. Should any idea from them be
worth pursuing (e.g. dropping the IPP dependency), start a new branch from
current `main` instead of reviving these.
