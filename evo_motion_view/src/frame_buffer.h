//
// Created by samuel on 07/05/24.
//

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

class FrameBuffer {
public:
    FrameBuffer(int width, int height);

    void bind() const;

    static void unbind();

    void rescale(int width, int height) const;

    [[nodiscard]] unsigned long get_texture() const;

    ~FrameBuffer();

private:
    unsigned int fbo;
    unsigned int texture;
    unsigned int rbo;
};

#endif//FRAME_BUFFER_H