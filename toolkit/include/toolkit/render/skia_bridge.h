#pragma once

#include <core/SkData.h>
#include <core/SkImage.h>
#include <core/SkImageInfo.h>
#include <toolkit/assetserver.h>

namespace toolkit {

// Creates a raster sk_sp<SkImage> from a loaded Image.
// When drawn to a GPU-backed Skia canvas, the texture upload is handled automatically.
// Call once when the image becomes ready and hold onto the returned sk_sp.
inline sk_sp<SkImage> to_sk_image(const Image& img) {
    auto info = SkImageInfo::Make(
        img.width, img.height, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType
    );
    auto data = SkData::MakeWithCopy(img.data.data(), img.data.size());
    return SkImages::RasterFromData(info, data, img.width * 4);
}

}  // namespace toolkit
