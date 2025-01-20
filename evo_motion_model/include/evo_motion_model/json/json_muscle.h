//
// Created by samuel on 15/01/25.
//

#ifndef EVO_MOTION_JSON_MUSCLE_H
#define EVO_MOTION_JSON_MUSCLE_H

#include <evo_motion_model/muscle.h>
#include <evo_motion_model/skeleton.h>

class JsonMuscularSystem final : public AbstractMuscularSystem {
public:
    JsonMuscularSystem(Skeleton skeleton, const std::string &json_path);

    std::vector<Muscle> get_muscles() override;

private:
    std::vector<Muscle> muscles;
};

#endif//EVO_MOTION_JSON_MUSCLE_H
