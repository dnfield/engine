// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <jni.h>
#include <optional>

#include "flutter/fml/macros.h"
#include "third_party/skia/include/core/SkImageGenerator.h"
#include "third_party/skia/include/core/SkTypes.h"

namespace flutter {

class ImageGeneratorAndroid : public SkImageGenerator {
 public:
  static std::unique_ptr<SkImageGenerator> MakeFromEncodedAndroid(
      sk_sp<SkData> data);

 protected:
  sk_sp<SkData> onRefEncodedData() override;

  bool onGetPixels(const SkImageInfo&,
                   void* pixels,
                   size_t rowBytes,
                   const Options&) override;

 private:
  ImageGeneratorAndroid(const SkImageInfo& info,
                        sk_sp<SkData> data,
                        sk_sp<SkData> encoded_data);
  const sk_sp<SkData> data_;
  const sk_sp<SkData> decoded_data_;
};

class ImageDecoderAndroid final {
 public:
  ImageDecoderAndroid() {}

  static bool Register(JNIEnv* env);

  struct Descriptor {
    SkImageInfo info;
    sk_sp<SkData> data;
  };

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
  std::optional<Descriptor> DecodeImage(sk_sp<SkData> data,
                                        int target_width,
                                        int target_height);

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(ImageDecoderAndroid);
};
}  // namespace flutter
