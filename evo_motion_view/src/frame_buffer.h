//
// Created by samuel on 07/05/24.
//

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

class FrameBuffer {
public:
    FrameBuffer(const int width, const int height);

    void bind() const;
    static void unbind();

    void rescale(const int width, const int height) const;
    unsigned long get_texture() const;

    ~FrameBuffer();

private:
    unsigned int fbo;
    unsigned int texture;
    unsigned int rbo;
};

#endif //FRAME_BUFFER_H
