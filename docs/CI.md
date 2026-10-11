# Cross-platform CI matrix

Build workflow: sonicwaveshaper/.github/workflows/build-vst.yml
Triggers: push to main, version tags (v*), pull requests, manual run.

| Job | Runner | Output | Artifact name |
|---|---|---|---|
| macOS | macos-14 (Apple Silicon host, builds arm64 and x86_64 universal) | VST3, AU, Standalone packed into SonicWaveshaper-macOS.dmg | SonicWaveshaper-macOS-dmg |
| Windows | windows-latest (Visual Studio 2022, x64) | VST3 and Standalone .exe packed into SonicWaveshaper-Windows.zip | SonicWaveshaper-Windows-zip |
| Release | ubuntu-latest | Runs only on tags v*. Attaches the DMG and Windows zip to a GitHub Release | none |

Shared build: CMakeLists.txt is the same for both platforms. JUCE 8.0.6 is cloned at configure time, and NeuralAmpModelerCore, Eigen, and nlohmann/json are fetched with FetchContent.

Platform-specific code: only the macOS job passes the universal architecture flag and runs packaging/make-dmg.sh. Only the Windows job runs the PowerShell zip step.

Status: the Windows and macOS jobs have not been confirmed green since the latest changes. Check the Actions tab before relying on a build.

Not yet done: code signing and notarization for macOS, code signing for Windows, and a Windows installer (currently a zip).
