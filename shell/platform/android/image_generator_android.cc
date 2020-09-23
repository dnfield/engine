// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/image_generator_android.h"

#include <android/bitmap.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/platform/android/jni_util.h"

namespace flutter {

namespace {
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

static fml::jni::ScopedJavaGlobalRef<jclass>* g_flutter_jni_class = nullptr;
static jmethodID g_decode_image_method = nullptr;
}  // namespace

bool ImageDecoderAndroid::Register(JNIEnv* env) {
  g_flutter_jni_class = new fml::jni::ScopedJavaGlobalRef<jclass>(
      env, env->FindClass("io/flutter/embedding/engine/FlutterJNI"));
  if (g_flutter_jni_class->is_null()) {
    FML_LOG(ERROR) << "Failed to find FlutterJNI Class.";
    return false;
  }

  g_decode_image_method = env->GetStaticMethodID(
      g_flutter_jni_class->obj(), "decodeImage",
      "(java/nio/ByteBuffer;II)Landroid/graphics/Bitmap;");

  if (g_decode_image_method == nullptr) {
    FML_LOG(ERROR) << "Could not locate decodeImage method";
    return false;
  }

  return true;
}

sk_sp<SkData> ImageDecoderAndroid::DecodeImage(sk_sp<SkData> data,
                                               int target_width,
                                               int target_height) {
  if (!g_flutter_jni_class || !g_decode_image_method) {
    return nullptr;
  }

  concurrent_task_runner_->PostTask() JNIEnv* env =
      fml::jni::AttachCurrentThread();

  fml::jni::ScopedJavaLocalRef<jobject> direct_buffer(
      env,
      env->NewDirectByteBuffer(const_cast<void*>(data->data()), data->size()));
  fml::jni::ScopedJavaLocalRef<jobject> bitmap(
      env, env->CallStaticObjectMethod(
               g_flutter_jni_class->obj(), g_decode_image_method,
               direct_buffer.obj(), target_width, target_height));
  FML_CHECK(fml::jni::CheckException(env));

  AndroidBitmapInfo info;
  int status;
  if ((status = AndroidBitmap_getInfo(env, bitmap.obj(), &info)) < 0) {
    FML_DLOG(ERROR) << "Failed to get bitmap info, status=" << status;
    return nullptr;
  }
  FML_DCHECK(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888);

  void* pixel_lock;
  if ((status = AndroidBitmap_lockPixels(env, bitmap.obj(), &pixel_lock)) < 0) {
    FML_DLOG(ERROR) << "Failed to lock pixels, error=" << status;
    return nullptr;
  }
  sk_sp<SkData> pixel_data = SkData::MakeWithCopy(
      pixel_lock, info.width * info.height * sizeof(uint32_t));
  AndroidBitmap_unlockPixels(env, bitmap.obj());

  return pixel_data;
}

ImageGeneratorAndroid::ImageGeneratorAndroid(const SkImageInfo& info,
                                             sk_sp<SkData> data,
                                             SkEncodedOrigin origin)
    : SkImageGenerator(info), data_(std::move(data)), origin_(origin) {}

sk_sp<SkData> ImageGeneratorAndroid::onRefEncodedData() {
  return data_;
}

bool ImageGeneratorAndroid::onGetPixels(const SkImageInfo& info,
                                        void* pixels,
                                        size_t rowBytes,
                                        const Options&) {
  if (kRGBA_8888_SkColorType != info.colorType()) {
    return false;
  }

  switch (info.alphaType()) {
    case kOpaque_SkAlphaType:
      if (kOpaque_SkAlphaType != this->getInfo().alphaType()) {
        return false;
      }
      break;
    case kPremul_SkAlphaType:
      break;
    default:
      return false;
  }
  FML_DLOG(ERROR) << origin_;
  // SkUniqueCFRef<CGImageRef> image(
  //     CGImageSourceCreateImageAtIndex(fImageSrc.get(), 0, nullptr));
  // if (!image) {
  //   return false;
  // }

  // SkPixmap dst(info, pixels, rowBytes);
  // auto decode = [&image](const SkPixmap& pm) {
  //   return SkCopyPixelsFromCGImage(pm, image.get());
  // };
  // return SkPixmapPriv::Orient(dst, fOrigin, decode);
  return true;
}

}  // namespace flutter
