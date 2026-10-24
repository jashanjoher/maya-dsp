#!/bin/bash
# Ad-hoc signs the Maya Echo ME-24 bundles and packs them into MayaEcho-macOS.dmg.
set -euo pipefail

OUT="build/MayaEcho_artefacts/Release"
STAGE="dmg-stage"
VST3="$OUT/VST3/Maya Echo ME-24.vst3"
AU="$OUT/AU/Maya Echo ME-24.component"
APP="$OUT/Standalone/Maya Echo ME-24.app"

for p in "$VST3" "$AU" "$APP"; do
  if [ ! -e "$p" ]; then
    echo "Missing: $p"
    echo "What the build produced:"
    find build -maxdepth 4 -print
    exit 1
  fi
done

# Ad-hoc signing (identity "-"): no Developer ID, so this is not notarized.
for p in "$VST3" "$AU" "$APP"; do
  codesign --force --deep --sign - "$p"
done

rm -rf "$STAGE"
mkdir "$STAGE"

cp -R "$VST3" "$STAGE/"
cp -R "$AU" "$STAGE/"
cp -R "$APP" "$STAGE/"

ln -s /Library/Audio/Plug-Ins/VST3 "$STAGE/1-VST3-folder"
ln -s /Library/Audio/Plug-Ins/Components "$STAGE/2-AU-folder"
ln -s /Applications "$STAGE/3-Applications"

cp packaging/README-echo-install.txt "$STAGE/READ ME FIRST.txt"

hdiutil create -volname "Maya Echo ME-24" -srcfolder "$STAGE" -ov -format UDZO MayaEcho-macOS.dmg
echo "Done: MayaEcho-macOS.dmg"
