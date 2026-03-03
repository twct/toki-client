#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <toolkit/assetserver.h>
#include <toolkit/log.h>
#include <toolkit/render/renderer.h>

namespace toolkit {

template<>
Handle<Image> AssetServer::load(const std::string& path) {
    size_t id;
    ImageEntry* entry;

    {
        std::lock_guard lock(m_mutex);

        if (auto it = m_path_to_id.find(path); it != m_path_to_id.end())
            return Handle<Image>{it->second};

        id = m_next_id.fetch_add(1, std::memory_order_relaxed);
        m_path_to_id.emplace(path, id);
        auto owned = std::make_unique<ImageEntry>();
        entry = owned.get();
        m_images.emplace(id, std::move(owned));
    }

    // entry is a raw borrow — safe because m_pool is destroyed before m_images,
    // so all tasks finish before any entry is freed.
    m_pool.enqueue([this, entry, path, id]() {
        int w, h, original_channels;
        // Force RGBA: Vulkan doesn't guarantee VK_FORMAT_R8G8B8_UNORM support.
        unsigned char* raw =
            stbi_load(path.c_str(), &w, &h, &original_channels, 4);
        if (!raw) {
            log::error("AssetServer: failed to load '{}': {}", path, stbi_failure_reason());
            entry->status.store(AssetStatus::Failed, std::memory_order_release);
            return;
        }

        entry->value.width = w;
        entry->value.height = h;
        entry->value.channels = 4;
        entry->value.data.assign(raw, raw + w * h * 4);
        stbi_image_free(raw);

        // Release ensures the value write is visible to any acquire on this status.
        entry->status.store(AssetStatus::Ready, std::memory_order_release);

        if (auto* r = m_renderer.load(std::memory_order_acquire)) {
            log::info("AssetServer: uploading '{}' ({}x{}) to renderer", path, w, h);
            r->create_image(id, entry->value);
        } else {
            log::warn("AssetServer: '{}' loaded but renderer not set, image will not display", path);
        }
    });

    return Handle<Image> {id};
}

template<>
AssetStatus AssetServer::status(Handle<Image> handle) const {
    std::lock_guard lock(m_mutex);
    auto it = m_images.find(handle.id);
    if (it == m_images.end())
        return AssetStatus::Failed;
    return it->second->status.load(std::memory_order_acquire);
}

template<>
const Image* AssetServer::get(Handle<Image> handle) const {
    std::lock_guard lock(m_mutex);
    auto it = m_images.find(handle.id);
    if (it == m_images.end())
        return nullptr;
    auto& entry = *it->second;
    if (entry.status.load(std::memory_order_acquire) != AssetStatus::Ready)
        return nullptr;
    // entry.value is written before status -> Ready and never modified again.
    return &entry.value;
}

}  // namespace toolkit
