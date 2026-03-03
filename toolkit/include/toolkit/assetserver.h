#pragma once

#include <toolkit/thread_pool.h>

#include <atomic>
#include <cstdint>
#include <flat_map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace toolkit {

class Renderer;

enum class AssetStatus { Loading, Ready, Failed };

struct Image {
    int width = 0;
    int height = 0;
    int channels = 0;  // always 4 (RGBA) after loading
    std::vector<uint8_t> data;
};

template<typename T>
struct Handle {
    size_t id = 0;
};

class AssetServer {
  public:
    void init(Renderer& r) { m_renderer.store(&r, std::memory_order_release); }

    template<typename T>
    Handle<T> load(const std::string& path);

    template<typename T>
    AssetStatus status(Handle<T> handle) const;

    template<typename T>
    const T* get(Handle<T> handle) const;

  private:
    struct ImageEntry {
        Image value;
        std::atomic<AssetStatus> status {AssetStatus::Loading};
    };

    std::atomic<Renderer*> m_renderer {nullptr};
    std::flat_map<std::string, size_t> m_path_to_id;
    std::flat_map<size_t, std::unique_ptr<ImageEntry>> m_images;
    mutable std::mutex m_mutex;
    std::atomic<size_t> m_next_id{1};
    // Declared last so it is destroyed first, joining threads before maps are torn down.
    ThreadPool m_pool;
};

template<>
Handle<Image> AssetServer::load(const std::string& path);
template<>
AssetStatus AssetServer::status(Handle<Image> handle) const;
template<>
const Image* AssetServer::get(Handle<Image> handle) const;

}  // namespace toolkit
