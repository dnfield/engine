// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <jni.h>

#include "flutter/fml/macros.h"
#include "third_party/skia/include/core/SkImageGenerator.h"
#include "third_party/skia/include/core/SkTypes.h"

namespace flutter {

class ImageGeneratorAndroid : public SkImageGenerator {
 public:
  ImageGeneratorAndroid(const SkImageInfo& info,
                        sk_sp<SkData> data,
                        SkEncodedOrigin origin);

 protected:
  sk_sp<SkData> onRefEncodedData() override;

  bool onGetPixels(const SkImageInfo&,
                   void* pixels,
                   size_t rowBytes,
                   const Options&) override;

 private:
  const sk_sp<SkData> data_;
  const SkEncodedOrigin origin_;
};

class ImageDecoderAndroid final {
 public:
  static bool Register(JNIEnv* env);

  // Decodes an image using Android API.
  //
  // This is used for fallback decoding if Flutter/Skia does not have an
  // inbuilt codec for the particular image format.
  // This method will always return null:
  //  - If there is no FlutterJNI reference set from SetFlutterJni
  //  - Below API level 28, where the image decoding functions we use are not
  //    supported.
  //  - For invalid image data or image data that Android does not know how to
  //    decode.
  sk_sp<SkData> DecodeImage(sk_sp<SkData> data,
                            int target_width,
                            int target_height);

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(ImageDecoderAndroid);
};
}  // namespace flutter
