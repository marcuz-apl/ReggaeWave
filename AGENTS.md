# ReggaeWave Repository Instructions

These instructions apply to the entire repository. Product requirements in `docs/PRD.md` are the source of truth. When implementation and the PRD disagree, stop, document the conflict, and obtain product approval before changing a locked product decision.

## Product contract

- The product name is **ReggaeWave**.
- Spell the music genre **Reggae** in product copy, code comments, documentation, fixtures, and identifiers.
- Accept authorized musical input from any source genre.
- Output Reggae only. Do not add a target-genre selector.
- Permit exactly three creative controls in the MVP flow:
  1. Reggae intensity
  2. Dub-effects amount
  3. Vocal level
- Generate two variations per conversion.
- Preserve the separated lead vocal. Do not implement voice cloning, celebrity voices, singer replacement, or impersonation.
- Lyrics/subtitles are optional and disabled by default.
- The initial product is a responsive web app/PWA backed by asynchronous cloud processing.
- Export MP3 and WAV. Export SRT, VTT, and the MP4 lyric visualizer only when subtitles are enabled.

## Rights and cultural safeguards

- Process only user-owned, explicitly licensed, or public-domain material with an authorized recording.
- Never bypass, weaken, preselect, or hide the rights attestation.
- Never use unlicensed commercial recordings in source code, fixtures, demos, screenshots, benchmarks, or documentation.
- Every committed audio fixture must have an adjacent machine-readable license manifest or be programmatically synthesized by the test.
- Do not imply that private use, AI processing, or a mechanical license automatically grants transformation rights.
- Do not train, fine-tune, evaluate, or market with user media unless a separately approved opt-in workflow exists. The MVP has no such workflow.
- Treat Reggae as Jamaican living cultural heritage. Avoid caricature, reductive language, flags/colors as superficial authenticity, and claims that automated output is culturally authoritative.
- Musical-quality changes must be evaluated against the PRD rubric with qualified Reggae practitioners.

## Architecture invariants

- Keep long-running media work out of request/response handlers.
- Upload directly to private object storage with user- and object-scoped signed credentials.
- Treat the original upload as immutable.
- Make every processing stage idempotent, versioned, observable, independently retryable, and safe after worker restart.
- Check cancellation before starting every stage.
- Address reusable artifacts by immutable input identity, pipeline version, stage version, and relevant parameters.
- Keep external AI and media vendors behind repository-owned interfaces. Domain objects must not expose vendor response types.
- Record the provider, provider model/version, stage version, seed, and material parameters needed to explain or reproduce an output.
- A subtitle or video failure must not invalidate successful audio output.
- A low-confidence musical analysis warns and continues; input genre alone never causes rejection.
- Keep CPU, GPU, and external-provider tasks on independently scalable queues.
- Never expose stable public media URLs.

## Intended repository shape

The repository is initially documentation-only. When scaffolding is approved, prefer these boundaries unless an approved architecture decision record changes them:

```text
apps/
  web/                 # Next.js responsive PWA
services/
  api/                 # FastAPI application and domain orchestration
  worker/              # Celery media and pipeline workers
packages/
  contracts/           # OpenAPI-derived and event-schema contracts
  audio-fixtures/      # Manifests and generators; no unlicensed media
infra/                 # Local and deployment configuration
tests/
  e2e/                 # Cross-service product flows
docs/
  PRD.md                # Product source of truth
  adr/                  # Approved architecture decisions
```

- Prefer small files with one responsibility.
- Keep features together by domain responsibility, not in generic dumping grounds such as `utils` or `helpers`.
- Define cross-service interfaces in versioned schemas before implementing consumers.
- Generate client types from schemas; do not maintain handwritten duplicates.
- Do not add large media binaries to normal Git history.

## Domain and state rules

- Use explicit project, conversion-job, and export-job state machines. Do not infer state from the presence of files.
- Conversion and export jobs have separate state machines as defined in `docs/PRD.md`; adding or reordering states requires a migration and contract review.
- MP3/WAV exports are independent of subtitle state. Subtitle and MP4 failures must not block or invalidate audio exports.
- Use stable public error codes and sanitized messages. Keep stack traces and provider payloads internal.
- Store musical analysis in a versioned manifest. Include timing units and coordinate origins in the schema.
- Store tuning values as validated numeric values using the PRD ranges and defaults.
- Record the selected variation explicitly before export.
- Keep the machine transcript distinct from user revisions.
- Use UTC timestamps in storage and APIs; localize only in clients.
- Make deletion and expiry first-class states with auditable events.

## Privacy and security

- Authorize every project, job, stage, transcript, preview, and export lookup by tenant ownership.
- Keep secrets in server-side secret management. Never commit credentials or expose them to the browser.
- Never log audio, lyrics, user filenames, signed URLs, authorization headers, secrets, or full vendor payloads.
- Validate actual uploaded bytes and decoded streams; do not trust extensions or client MIME types.
- Apply file-size, duration, rate, decode-time, and resource limits before expensive work.
- Use short-lived signed URLs and private storage buckets.
- Delete sources, stems, and analysis-only audio within 24 hours after a terminal job state.
- Expire final exports after 30 days and support immediate user deletion.
- Revoke access immediately when deletion begins, even if physical object deletion is asynchronous.
- Any new subprocessor requires privacy, retention, licensing, security, and data-training review.

## Audio and AI requirements

- Normalize into a documented canonical format without overwriting the source.
- Preserve timing alignment across the vocal, generated stems, previews, and exports.
- Keep source separation, music analysis, transcription, arrangement, mixing, mastering, and rendering as separate stages.
- Do not label a model or algorithm “AI” unless that distinction helps users understand behavior or risk.
- Do not introduce a model based only on demo quality. Record its license, allowed commercial use, training-data statement, supported hardware, latency, cost, and benchmark result.
- MusicGen and similar research systems are references, not approved production dependencies.
- Licensed samples and instruments must have documented rights permitting server-side rendering and distribution in user exports.
- Default mastering targets are -14 LUFS integrated with a -1 dBTP ceiling and the tolerances in the PRD.
- Keep two generated variations aligned to identical duration and start time.

## Frontend requirements

- Keep the default flow one-click. Place the three optional tuning controls behind a compact disclosure.
- Do not add advanced production controls, a multitrack editor, or a target-genre choice to the MVP.
- Keep subtitles off on every new project until the user explicitly enables them.
- Make upload, progress, A/B playback, lyric editing, cancellation, export, and deletion keyboard accessible.
- Preserve the playback timestamp when switching between variations.
- Do not display false precision for remaining processing time.
- Identify reduced confidence by musical dimension—rhythm, harmony, vocal separation, or structure—in plain language.
- Meet WCAG 2.2 AA and support responsive widths from 320 px upward.

## Backend and worker requirements

- Keep API handlers thin: validate, authorize, persist intent, enqueue, and return.
- Use database transactions for state transitions and an outbox or equivalent pattern when publishing work from committed state.
- Use deterministic idempotency keys for every stage and export.
- Bound retries and classify errors as validation, transient, provider, capacity, deterministic-media, cancelled, or internal.
- A retry must resume from valid prior artifacts rather than restart the entire job.
- Probe and checksum every media export before marking it downloadable.
- Reconcile object-storage lifecycle state with database retention state on a scheduled job.
- Emit structured metrics for queue delay, stage duration, retries, provider latency, artifact size, failure category, and compute cost.

## Testing rules

- Develop behavior test-first: add a failing test, observe the expected failure, implement the minimum change, and rerun the focused and relevant suites.
- Unit-test rights rules, tuning bounds, state transitions, idempotency, retention, schema validation, arrangement rules, subtitles, and export profiles.
- Contract-test API schemas, worker events, provider adapters, analysis manifests, and artifact metadata.
- Integration-test upload, validation, queueing, retries, cancellation, deletion, provider failure mapping, and export probing.
- End-to-end test the primary flow with subtitles disabled and enabled.
- Add cross-tenant authorization tests for every new media or metadata endpoint.
- Use short synthetic, owned, or CC0 audio fixtures. Record fixture provenance in a license manifest.
- Audio tests must assert measurable properties such as duration, sample rate, bit depth, channel count, loudness, true peak, timing, and codec—not only file existence.
- Never update golden outputs merely to make a failing test pass; explain and review the audible or measurable change.
- Run accessibility checks and keyboard tests for every changed primary-flow screen.

## Documentation and decisions

- Update `docs/PRD.md` only for approved product changes.
- Record material technical choices in `docs/adr/` with context, decision, consequences, and rollback path.
- Document every external model, service, sample library, and media codec with its license and operational constraints.
- Keep diagrams synchronized with actual service and data flow.
- Do not leave ambiguous placeholders such as “handle errors” or “add tests”; state the exact behavior and verification.

## Change discipline

- Preserve unrelated user changes in a dirty worktree.
- Make focused changes and avoid unrelated refactoring.
- Do not perform destructive Git or filesystem operations without explicit authorization.
- Do not commit generated media, local caches, model weights, secrets, or provider responses.
- When a change affects rights, retention, public API contracts, job states, tuning, exports, or the fixed Reggae destination, stop for product review before implementation.

## Definition of done

A change is complete only when:

- Its behavior matches the PRD and these repository instructions.
- Relevant unit, contract, integration, end-to-end, audio, security, and accessibility tests pass.
- New media fixtures have verified provenance.
- Schemas, migrations, retention behavior, observability, and failure behavior are included where applicable.
- No secrets, private media, lyrics, signed URLs, or unlicensed assets appear in code, logs, tests, or documentation.
- User-visible copy consistently says ReggaeWave and Reggae.
- Documentation and diagrams reflect the implemented behavior.
- Verification output has been reviewed before claiming success.
