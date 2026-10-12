#!/bin/bash
# Ad-hoc signs the Maya Chamber bundles and packs them into MayaChamber-macOS.dmg.
set -euo pipefail

OUT="build/MayaChamber_artefacts/Release"
STAGE="dmg-stage"
VST3="$OUT/VST3/Maya Chamber.vst3"
AU="$OUT/AU/Maya Chamber.component"
APP="$OUT/Standalone/Maya Chamber.app"

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

cp packaging/README-install.txt "$STAGE/READ ME FIRST.txt"

hdiutil create -volname "Maya Chamber" -srcfolder "$STAGE" -ov -format UDZO MayaChamber-macOS.dmg
echo "Done: MayaChamber-macOS.dmg"
