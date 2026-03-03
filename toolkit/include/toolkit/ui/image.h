#pragma once

#include <toolkit/assetserver.h>
#include <toolkit/render/commands.h>
#include <toolkit/ui.h>

namespace toolkit {

class UiImage : public UiNode {
  public:
    UiImage(Handle<Image> handle) : m_handle(handle) {}

    UiImage& set_fit(ImageFit fit) {
        m_fit = fit;
        return *this;
    }

  protected:
    void paint(Painter& painter) override {
        UiNode::paint(painter);
        painter.draw_image(computed_position(), computed_size(), m_handle.id, m_fit);
    }

  private:
    Handle<Image> m_handle;
    ImageFit m_fit = ImageFit::Fill;
};

}  // namespace toolkit
