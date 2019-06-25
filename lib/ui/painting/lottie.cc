// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/lottie.h"

#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"

namespace flutter {

static void LottieAnimation_constructor(Dart_NativeArguments args) {
  DartCallConstructor(&LottieAnimation::Create, args);
}

IMPLEMENT_WRAPPERTYPEINFO(ui, LottieAnimation);

#define FOR_EACH_BINDING(V)       \
  V(LottieAnimation, drawFrame)  \
  V(LottieAnimation, duration)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void LottieAnimation::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register(
      {{"LottieAnimation_constructor", LottieAnimation_constructor, 2, true},
       FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

LottieAnimation::LottieAnimation() {}

LottieAnimation::~LottieAnimation() {}

void LottieAnimation::drawFrame(const Canvas* canvas,
                                 double progress,
                                 double height,
                                 double width) {
  FML_DCHECK(canvas);
  FML_DCHECK(progress >= 0);
  FML_DCHECK(height > 0);
  FML_DCHECK(width > 0);
  auto skia_canvas = canvas->canvas();
  FML_DCHECK(skia_canvas);
  animation_->seek(progress);
  SkRect bounds = SkRect::MakeWH(width, height);
  animation_->render(skia_canvas, &bounds);
}

double LottieAnimation::duration() {
  return animation_->duration();
}

}  // namespace blink
