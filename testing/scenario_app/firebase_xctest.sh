#!/bin/bash

set -e

"${BASH_SOURCE%/*}/compile_ios_aot.sh" $1 $2

GIT_REVISION=${3:-$(git rev-parse HEAD)}
BUILD_ID=${4:-$CIRRUS_BUILD_ID}

pushd "${BASH_SOURCE%/*}/ios/Scenarios"
xcodebuild -project Scenarios.xcodeproj -scheme Scenarios -configuration Debug \
    -sdk iphoneos \
    -derivedDataPath DerivedData/Scenarios \
    -allowProvisioningUpdates \
    build-for-testing

pushd DerivedData/Scenarios/Build/Products

if [[ $CODE_SIGNING_ALLOWED == "NO ]]; then
  /usr/bin/codesign -s $CODE_SIGN_IDENTITY -f --deep Debug-iphoneos/Scenarios.app
  /usr/bin/codesign -s $CODE_SIGN_IDENTITY -f --deep Debug-iphoneos/ScenariosUITests-Runner.app
fi

zip -r scenarios.zip Debug-iphoneos Scenarios*.xctestrun

gcloud firebase test ios run --test ./scenarios.zip \
    --device model=iphone8plus,version=12.0,locale=en_US,orientation=portrait \
    --xcode-version=10.2 \
    --results-bucket=gs://flutter_firebase_testlab \
    --results-dir=engine_scenario_test/$GIT_REVISION/$BUILD_ID \

popd
popd
