//
// Created by samuel on 29/01/25.
//

#ifndef EVO_MOTION_FRAME_BUFFER_H
#define EVO_MOTION_FRAME_BUFFER_H

class FrameBuffer final {
public:
    FrameBuffer(float width, float height);
    virtual ~FrameBuffer();

    unsigned int get_frame_texture() const;
    void rescale_frame_buffer(float width, float height) const;
    void bind() const;
    static void unbind();

private:
    unsigned int fbo;
    unsigned int texture;
    unsigned int rbo;
};

#endif//EVO_MOTION_FRAME_BUFFER_H
