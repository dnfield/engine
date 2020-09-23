// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <jni.h>

#include "third_party/skia/include/core/SkImageGenerator.h"
#include "third_party/skia/include/core/SkTypes.h"

namespace flutter {

class ImageDecoderAndroid final {
 public:
  static bool Register(JNIEnv* env);

  ImageDecoderAndroid(fml::jni::JavaObjectWeakGlobalRef java_object);

  sk_sp<SkData> DecodeImage(sk_sp<SkData> data,
                            int target_width,
                            int target_height);

 private:
  // Reference to FlutterJNI object.
  fml::jni::JavaObjectWeakGlobalRef java_object_;
};
}  // namespace flutter
