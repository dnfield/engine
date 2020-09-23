// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/image_decoder_android.h"

#include <android/bitmap.h>
#include <dlfcn.h>

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
}  // namespace

static fml::jni::ScopedJavaGlobalRef<jclass>* g_flutter_jni_class = nullptr;
static jmethodID g_decode_image_method = nullptr;
static jmethodID g_bitmap_recycle_method = nullptr;

bool ImageDecoderAndroid::Register(JNIEnv* env) {
  g_flutter_jni_class = new fml::jni::ScopedJavaGlobalRef<jclass>(
      env, env->FindClass("io/flutter/embedding/engine/FlutterJNI"));
  if (g_flutter_jni_class->is_null()) {
    FML_LOG(ERROR) << "Failed to find FlutterJNI Class.";
    return false;
  }

  g_decode_image_method =
      env->GetMethodID(g_flutter_jni_class->obj(), "decodeImage",
                       "(java/nio/ByteBuffer;II)Landroid/graphics/Bitmap;");

  if (g_decode_image_method == nullptr) {
    FML_LOG(ERROR) << "Could not locate decodeImage method";
    return false;
  }

  fml::jni::ScopedJavaLocalRef<jclass> bitmap_class(
      env, env->FindClass("android/graphics/Bitmap"));
  if (bitmap_class.is_null()) {
    FML_LOG(ERROR) << "Failed to find android.graphics.Bitmap Class.";
    return false;
  }

  g_bitmap_recycle_method =
      env->GetMethodID(bitmap_class.obj(), "recycle", "()V");
  if (g_bitmap_recycle_method == nullptr) {
    FML_LOG(ERROR) << "Failed to locate android.graphics.Bitmap.recycle()";
    return false;
  }
  return true;
}

sk_sp<SkData> ImageDecoderAndroid::DecodeImage(sk_sp<SkData> data,
                                               int target_width,
                                               int target_height) {
  JNIEnv* env = fml::jni::AttachCurrentThread();

  auto java_object = java_object_.get(env);
  if (java_object.is_null()) {
    return nullptr;
  }

  fml::jni::ScopedJavaLocalRef<jobject> direct_buffer(
      env,
      env->NewDirectByteBuffer(const_cast<void*>(data->data()), data->size()));
  fml::jni::ScopedJavaLocalRef<jobject> bitmap(
      env,
      env->CallObjectMethod(java_object.obj(), g_decode_image_method,
                            direct_buffer.obj(), target_width, target_height));
  FML_CHECK(CheckException(env));

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

  env->CallVoidMethod(bitmap.obj(), g_bitmap_recycle_method);

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

// struct AImageDecoder;

// typedef int (*AImageDecoder_createFromBuffer)(const void* buffer,
//                                               size_t length,
//                                               AImageDecoder** outDecoder);

// typedef void (*AImageDecoder_delete)(AImageDecoder* decoder);

// std::unique_ptr<SkImageGeneartor>
// AndroidImageDecoder::CreateSkImageGenerator(
//     SkData* data) {
//   void* handle_ = dlopen("libjnigraphics.so", RTLD_NOW | RTLD_NODELETE);
//   if (!handle_) {
//     FML_LOG(ERROR) << "Failed to open libjnigraphics";
//     return nullptr;
//   }

//   auto create = (AImageDecoder_createFromBuffer)dlsym(
//       handle_, "AImageDecoder_createFromBuffer");
//   if (!create) {
//     FML_LOG(ERROR) << "Failed to dlsym create function";
//     return nullptr;
//   }

//   auto destroy = (AImageDecoder_delete)dlsym(handle_,
//   "AImageDecoder_delete"); if (!destroy) {
//     FML_LOG(ERROR) << "Failed to dlsym delete function";
//     return nullptr;
//   }

//   AImageDecoder* decoder;
//   int result = create(data->data(), data->size(), &decoder);
//   if (result != 0) {
//     ERRORF(r, "Unexpected result %i");
//     return;
//   }

//   destroy(decoder);
// }

}  // namespace flutter