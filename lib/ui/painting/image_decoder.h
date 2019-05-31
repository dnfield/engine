// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_IMAGE_DECODER_H_
#define FLUTTER_LIB_UI_PAINTING_IMAGE_DECODER_H_

#include <memory>
#include <optional>

#include "flutter/common/task_runners.h"
#include "flutter/flow/skia_gpu_object.h"
#include "flutter/fml/concurrent_message_loop.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/mapping.h"
#include "flutter/lib/ui/io_manager.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkSize.h"

namespace flutter {

class ImageDecoder {
 public:
  ImageDecoder(
      TaskRunners runners,
      std::shared_ptr<fml::ConcurrentTaskRunner> concurrent_task_runner,
      fml::WeakPtr<IOManager> io_manager);

  ~ImageDecoder();

  bool IsValid() const;

  struct ImageInfo {
    SkImageInfo sk_info = {};
    size_t row_bytes = 0;
  };

  struct ImageDescriptor {
    sk_sp<SkData> data;
    std::optional<ImageInfo> decompressed_image_info;
    std::optional<uint32_t> target_width;
    std::optional<uint32_t> target_height;
  };

  using ImageResult = std::function<void(SkiaGPUObject<SkImage>)>;

  void Decode(ImageDescriptor descriptor, ImageResult result);

  fml::WeakPtr<ImageDecoder> GetWeakPtr() const;

 private:
  TaskRunners runners_;
  std::shared_ptr<fml::ConcurrentTaskRunner> concurrent_task_runner_;
  fml::WeakPtr<IOManager> io_manager_;
  bool is_valid_ = false;
  fml::WeakPtrFactory<ImageDecoder> weak_factory_;

  FML_DISALLOW_COPY_AND_ASSIGN(ImageDecoder);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_IMAGE_DECODER_H_
