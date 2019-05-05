#ifndef OGLW_FRAMEBUFFER_H_190427
#define OGLW_FRAMEBUFFER_H_190427

#include <memory>

#include <oglw/image.h>

namespace oglw {

// =============================== Frame Buffer ================================
class FrameBuffer {
public:
    template <typename... Args>
    static auto Create(Args... args) {
        return std::make_shared<FrameBuffer>(args...);
    }

    FrameBuffer();
    FrameBuffer(size_t w, size_t h);
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer(FrameBuffer&&);
    FrameBuffer& operator=(const FrameBuffer&) = delete;  // non-copyable
    FrameBuffer& operator=(FrameBuffer&&);
    virtual ~FrameBuffer();

    void init(size_t w, size_t h);
    GpuImagePtr<uint8_t> getImage();

    void bind();
    static void Unbind();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ------------------------------ Pointer Aliases ------------------------------
using FrameBufferPtr = std::shared_ptr<FrameBuffer>;

}  // namespace oglw

#endif
