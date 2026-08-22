# ReggaeWave Offline Desktop MVP Architecture Design

| Field | Value |
| --- | --- |
| Date | 2026-08-18 |
| Status | Approved in design discussion; awaiting review of this written specification |
| Platforms | macOS 13+ and Windows 11 x64 |
| Product form | Installed native desktop application with a shared UI codebase |
| Processing | Fully offline after first-time model-pack installation |

## 1. Decision and PRD relationship

ReggaeWave will develop an installed macOS and Windows desktop application before web or mobile editions. Conversion runs entirely on the user's computer. The application may access the network only for explicit application-update and model-pack operations; audio processing does not require or use a network connection.

This is an approved product pivot from the current responsive-web and asynchronous-cloud decisions in `docs/PRD.md`. It also replaces fixed cloud retention with user-managed local retention. The PRD and repository instructions must be amended and reviewed before implementation begins. Until that amendment is merged, the current PRD remains the source of truth and no conflicting scaffold or product code may be added.

## 2. Goals

- Ship one shared desktop codebase for macOS and Windows.
- Complete the entire conversion pipeline offline after model setup.
- Support Apple Silicon and Windows NVIDIA acceleration when an approved model produces equivalent results on those runtimes.
- Provide a CPU compatibility path on Apple Silicon, Intel Macs, and Windows x64 machines.
- Preserve durable, restart-safe, idempotent processing and explicit job states without cloud infrastructure.
- Keep projects and media private on the user's computer and under user-managed retention.
- Preserve all fixed ReggaeWave product, rights, cultural, musical-quality, accessibility, and export requirements not explicitly changed by this design.

## 3. Non-goals

- A web/PWA client for the desktop MVP
- Cloud conversion, cloud storage, hosted accounts, or tenant collaboration
- Native iOS or Android clients in the desktop MVP
- Windows on ARM in the desktop MVP
- Background telemetry or automatic media upload
- A multitrack editor, advanced production controls, or a target-genre selector
- Voice cloning, singer replacement, impersonation, or model training on user media
- Guaranteed forensic erasure from SSDs, operating-system backups, or user-created copies

## 4. Product invariants retained

- The product name is ReggaeWave, and the genre is spelled Reggae.
- Authorized input may come from any source genre; output is Reggae only.
- Rights attestation is mandatory and cannot be preselected, bypassed, or hidden.
- The conversion flow exposes exactly three creative controls: Reggae intensity, dub-effects amount, and vocal level.
- Subtitle generation is optional and disabled for every new project.
- Every conversion creates two duration-aligned variations.
- The separated lead vocal is preserved; it is never cloned or replaced.
- MP3 and WAV exports remain independent of subtitle state.
- SRT, VTT, and the MP4 lyric visualizer are available only when subtitles are enabled.
- Subtitle or video failure never invalidates successful audio output.
- Musical quality remains subject to the approved PRD rubric and qualified Reggae-practitioner review.

## 5. Architecture

### 5.1 Component boundaries

```text
React/TypeScript UI
        │ generated commands and events
        ▼
Tauri 2 / Rust desktop host
  ├─ capability and filesystem mediation
  ├─ durable local job coordinator
  ├─ process supervision and cancellation
  ├─ SQLite persistence
  └─ model-pack and application updates
        │ versioned private IPC
        ▼
Python offline engine
  ├─ media validation and normalization
  ├─ source separation and analysis
  ├─ optional transcription
  ├─ Reggae arrangement
  ├─ mixing and mastering
  └─ MP3/WAV/SRT/VTT/MP4 rendering
```

The UI cannot read arbitrary files, spawn processes, or access model storage directly. The Rust host exposes a narrow capability allowlist and validates every command. The Python engine exposes only versioned pipeline operations and events. Vendor- or runtime-specific values never cross into domain objects.

### 5.2 Toolchain

- **Desktop UI:** TypeScript, React, and Vite
- **Desktop host:** Tauri 2 and Rust
- **Audio/ML engine:** Python, FFmpeg, and approved music/DSP libraries
- **Local metadata:** SQLite with explicit transactional state transitions
- **Contracts:** versioned schemas with generated TypeScript, Rust, and Python bindings
- **Inference:** a repository-owned adapter over approved CPU, Apple, and Windows/NVIDIA runtimes
- **Distribution:** signed Windows installers and signed/notarized macOS application bundles

Next.js, FastAPI, Celery, Redis, PostgreSQL, and S3-compatible storage are not part of the offline desktop MVP.

### 5.3 Intended repository shape

```text
apps/
  desktop/              # React UI and Tauri/Rust host
services/
  engine/               # Python offline media and ML engine
packages/
  contracts/            # Versioned schemas and generated bindings
  model-manifests/      # Signed-pack schemas, licenses, and compatibility data
  audio-fixtures/       # Synthetic/owned/CC0 generators and license manifests
infra/
  packaging/            # Signing, installer, and release configuration
tests/
  e2e/                  # Installed desktop flows and offline verification
docs/
  adr/                  # Approved architecture decisions and rollback paths
```

## 6. Local data model and storage

ReggaeWave uses operating-system application-data directories for its database, verified model packs, managed projects, logs, and temporary work. User-selected exports may be written outside the managed project directory only after an explicit save action.

Each managed project contains:

- An immutable copy of the imported source
- A versioned rights-attestation record
- A versioned musical-analysis manifest
- Stage artifacts addressed by immutable source checksum, pipeline version, stage version, model-pack version, seed, and relevant parameters
- Two variation manifests and their previews
- Machine transcript and user revisions as distinct records when subtitles are enabled
- Validated export metadata and checksums

SQLite is the source of truth for project, conversion-job, stage-run, transcript, variation-selection, export-job, deletion, and optional-cleanup state. File presence never determines state.

Rights attestation stores the selected basis, policy version, project identifier, and UTC timestamp. The offline MVP has no hosted user identity or coarse request-region field.

## 7. State machines and pipeline execution

### 7.1 Conversion jobs

The offline conversion state machine is:

`created → importing → validating → queued → normalizing → separating → analyzing → arranging → mixing → transcribing → preview_ready → completed`

Subtitle-disabled jobs skip `transcribing`. Terminal alternatives remain `failed`, `cancelled`, `expired`, and `deleted`. `expired` is used only when the user explicitly enables an automatic-cleanup policy.

Renaming cloud state `uploading` to local state `importing` is a breaking contract change and requires the PRD and contract review before implementation.

### 7.2 Export jobs

Each export remains a separate job:

`created → queued → rendering → validating → completed`

Terminal alternatives are `failed`, `cancelled`, `expired`, and `deleted`. MP3 and WAV exports do not depend on subtitle state. SRT, VTT, and MP4 jobs require subtitles to have been enabled and a valid transcript revision to exist.

### 7.3 Local execution lanes

The coordinator provides separate bounded lanes for CPU inference, accelerated inference, and media rendering. It starts no stage before committing intent to SQLite and checks cancellation before each stage. A restart resumes from verified prior artifacts rather than restarting the whole job.

Only one accelerated stage runs by default. CPU concurrency and FFmpeg concurrency are bounded using detected memory and processor capacity. The user may reduce resource use in application settings; performance settings are operational controls and do not add creative controls to the conversion flow.

## 8. Hardware and runtime policy

The supported release matrix is:

| Platform | Architecture | Required path | Optional acceleration |
| --- | --- | --- | --- |
| macOS 13+ | Apple Silicon | CPU | Approved Apple GPU/Neural Engine runtime |
| macOS 13+ | Intel x64 | CPU | None required |
| Windows 11 | x64 | CPU | Approved NVIDIA runtime |

CPU-only mode may take real-time or multiple times track duration. It must retain the same product features and measurable output targets; it does not receive the accelerated performance target.

At startup and before a job, a hardware probe selects the fastest validated provider. Provider initialization failure or GPU-memory exhaustion triggers one clearly reported CPU fallback. The fallback does not create a second user-visible variation or restart valid earlier stages.

Each approved model records its license, commercial-use terms, training-data statement, hardware support, expected memory, benchmark results, and output-equivalence results. A model is not approved merely because it performs well in a demonstration.

## 9. Model-pack lifecycle

The application installer does not contain the large model payload. First-time setup performs an explicit model-pack installation:

1. Fetch a signed manifest and the platform-appropriate pack.
2. Support resumable download into a non-active staging directory.
3. Verify the manifest signature, every artifact checksum, platform, architecture, runtime compatibility, and license metadata.
4. Probe the complete installed pack before activation.
5. Activate it atomically while retaining the previous verified pack for rollback.
6. Delete superseded packs only with user approval or an enabled cleanup policy.

Pack variants are macOS Apple Silicon accelerated, macOS Intel CPU, Windows x64 CPU, and an optional Windows x64 NVIDIA acceleration add-on. A project records the exact active pack and model versions needed to explain or reproduce its output.

Model checks and application updates are user-initiated network operations. Conversion and project browsing remain functional without a network connection. Failure to update never invalidates the current verified pack.

## 10. Primary user flow

1. The user installs and launches ReggaeWave.
2. First-time setup installs and verifies the required model pack.
3. The user creates a local project and imports an audio file.
4. ReggaeWave creates an immutable managed source copy and validates decoded media limits.
5. The user completes the mandatory rights attestation.
6. The user leaves the three creative controls at their defaults or adjusts them.
7. The user leaves subtitles disabled or explicitly enables them.
8. The coordinator persists and runs the conversion locally with stage-level progress and cancellation.
9. The user compares two synchronized variations while playback position is preserved.
10. The user explicitly selects one variation.
11. The user exports MP3 or WAV and, when enabled, edits lyrics and exports SRT, VTT, or MP4.
12. The user keeps the project, deletes selected intermediates or exports, or deletes the entire project.

The application shows stage and activity information without false remaining-time precision. Reduced-confidence warnings identify rhythm, harmony, vocal separation, or structure in plain language.

## 11. Retention and deletion

Projects remain on the user's computer until the user deletes them. Automatic cleanup is optional and disabled by default. A storage dashboard reports source, intermediate, model, preview, and export usage separately.

Deleting a project immediately makes it unavailable in the application, records the deletion transition, terminates eligible work, and removes managed files through a restart-safe deletion queue. Deletion errors remain visible and retryable. ReggaeWave does not promise forensic erasure from SSD wear-leveling, operating-system backups, snapshots, or copies created outside its managed directories.

Users may independently remove regenerable intermediates without deleting source, project metadata, selected variation, or final exports. Regeneration uses the recorded pipeline and model-pack identities when those versions remain installed.

## 12. Privacy and security

- No sign-in, hosted account, tenant, cloud media store, or background telemetry exists in the MVP.
- The original external file is never overwritten.
- Processing code has no update or download responsibility.
- All update and model-pack networking is isolated in the Rust host behind explicit user actions.
- IPC accepts only schema-validated commands and never arbitrary shell text.
- Tauri capabilities grant the minimum filesystem and process access required for the current window.
- Logs exclude audio, lyrics, user filenames, full paths, rights evidence, credentials, and model-provider payloads.
- Secrets used for release signing stay in platform signing services or CI secret management and are never shipped in the application.
- Exported media has unnecessary embedded metadata removed.
- Diagnostic bundles are generated only on explicit request, are sanitized, and never include media or lyrics.
- Release tests execute complete conversions with networking disabled and fail if the processing path attempts network access.

Operating-system account permissions protect local application data in the MVP. Application-level project passwords and custom at-rest encryption are outside this MVP; the product documentation must recommend the platform's full-disk encryption for users requiring device-loss protection.

## 13. Error handling

Stable local error categories are validation, transient-local, model-pack, capacity, deterministic-media, cancelled, and internal. Messages are sanitized and actionable.

- Invalid or unsupported media fails before expensive processing.
- Insufficient disk or memory fails during preflight when it can be predicted.
- Deterministic media failures do not retry indefinitely.
- Interrupted local I/O and restart-safe cleanup may retry within documented bounds.
- Corrupt or incomplete artifacts never become playable or exportable.
- GPU failure retries the affected stage once on CPU after warning the user.
- Subtitle and lyric-video errors never invalidate successful audio.
- Stack traces and internal payloads remain in sanitized local diagnostics, not normal UI copy.

## 14. Testing and acceptance

Development remains behavior-first: establish a failing test, observe the expected failure, implement the minimum behavior, and rerun focused and relevant suites.

### 14.1 Engine feasibility gate

Before product scaffolding proceeds beyond a disposable engine probe, test at least ten authorized tracks from five source genres. At least eight must be recognized as Reggae and at least six judged usable after no more than one automated rerender, using the existing qualified-practitioner rubric.

The probe must demonstrate useful vocal separation, stable beat/chord/section manifests, aligned Reggae arrangement, two coherent variations, deterministic WAV rendering, CPU execution, and at least one accelerated path on each supported accelerated platform.

### 14.2 Automated coverage

- Unit tests for rights rules, tuning bounds, schemas, state transitions, idempotency, retention, deletion, subtitle normalization, and export profiles
- Contract tests for TypeScript/Rust/Python commands, events, manifests, model packs, stages, and artifacts
- Integration tests for import, validation, queueing, cancellation, restart, retry, deletion, disk-full behavior, corrupt packs, provider fallback, and export probing
- Golden-audio assertions for duration, timing, sample rate, bit depth, channels, loudness, true peak, and codecs
- End-to-end tests for subtitles-disabled and subtitles-enabled flows on installed macOS and Windows builds
- Accessibility tests for keyboard operation, focus, screen-reader announcements, contrast, reduced motion, and responsive window sizes
- Security tests for capability scope, IPC validation, log redaction, signature rejection, path traversal, model rollback, and offline processing

Fixtures must be programmatically synthesized, owned, or CC0 and must include adjacent machine-readable provenance.

### 14.3 Release matrix

Both macOS and Windows must pass before general availability. Release candidates are tested on Apple Silicon acceleration, Apple Silicon CPU, Intel Mac CPU, Windows x64 CPU, and supported Windows NVIDIA acceleration.

Signed/notarized macOS installers and signed Windows installers must pass clean installation, first model download, interrupted-download recovery, offline conversion, update, rollback, project deletion, and uninstall testing on real machines.

## 15. Delivery sequence

1. Amend `docs/PRD.md` and `AGENTS.md` to record the approved desktop/offline pivot and user-managed local retention.
2. Add an ADR for Tauri/React/Rust/Python, local SQLite orchestration, signed model packs, and the rollback path.
3. Complete the offline engine feasibility gate.
4. Define versioned cross-language contracts and the durable local state machines.
5. Build the Python engine and local orchestration behavior test-first.
6. Add the Tauri host, signed model-pack lifecycle, permissions, and secure IPC.
7. Build the accessible primary interface.
8. Validate packaging, signing, installation, updates, rollback, and uninstall.
9. Run automated audio checks and qualified Reggae-practitioner release review.

No implementation step may bypass the PRD amendment, model/license review, or feasibility gate.

## 16. Mobile relationship

iOS and Android are separate post-MVP products. They may reuse schemas, product rules, design tokens, and model metadata, but they do not constrain the desktop UI or runtime architecture. Their model formats, acceleration providers, storage, background execution, and packaging require separate product and architecture approval.

## 17. Rollback path

If the feasibility gate shows that acceptable Reggae output cannot run on the required CPU tier, stop before building the product UI and installers. Product review may narrow supported hardware, approve a hybrid/cloud design, or restore the web/cloud direction.

The engine, manifests, and state contracts remain independent of Tauri so a later approved client or hosted coordinator can consume them. No desktop-only type may enter the musical domain model.

## 18. Required documentation changes

The PRD amendment must update its executive summary, goals and non-goals, user flow, accounts, intake, retention, architecture, state machines, privacy, reliability, performance, compatibility, analytics, delivery strategy, risk table, and locked product decisions. Repository instructions must replace web/cloud-specific invariants and intended directories without weakening rights, cultural, testing, or media-safety requirements.

The ADR must document context, considered alternatives, the selected architecture, operational and packaging consequences, security boundaries, and this rollback path.

## 19. References

- [Tauri: embedding external binaries](https://v2.tauri.app/develop/sidecar/)
- [Tauri: WebView versions](https://v2.tauri.app/reference/webview-versions/)
- [Tauri: distribution and signing](https://v2.tauri.app/distribute/)
- [ONNX Runtime execution providers](https://onnxruntime.ai/docs/execution-providers/)
