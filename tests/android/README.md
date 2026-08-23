# Android Gradle project contract

`test-gradle-project.sh` is a portable Bash contract for the generated Android
project at `apps/mobile/Builds/Android/`. It checks that the Gradle wrapper,
app module, manifest, application id, minimum API level, and required
`arm64-v8a` and `x86_64` ABI declarations are present.

From Git Bash at the repository root, run:

```bash
bash tests/android/test-gradle-project.sh
```

The contract is expected to fail until the Android Studio project is generated.
