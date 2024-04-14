//
// Created by samuel on 25/12/22.
//

#ifndef EVO_MOTION_SLIDER_H
#define EVO_MOTION_SLIDER_H

#include <btBulletDynamicsCommon.h>
#include <evo_motion_model/controller.h>
#include <torch/torch.h>

class SliderController : public Controller {
public:
    explicit SliderController(int action_index, btSliderConstraint *slider, float slider_speed);

    void on_input(torch::Tensor action) override;

private:
    int action_index;
    btSliderConstraint *slider;
    float slider_speed;
};

#endif//EVO_MOTION_SLIDER_H
