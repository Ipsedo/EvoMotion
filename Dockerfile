FROM nvidia/cuda:12.4.1-cudnn-devel-ubuntu22.04

ENV EVO_MOTION_ROOT="/opt/evo_motion"

# Install dependencies
RUN apt-get update
RUN apt-get -y install ca-certificates gpg wget git libbullet-dev libglm-dev libglew-dev libglfw3-dev libimgui-dev curl unzip

# CMake last version
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null
RUN apt-get update
RUN rm /usr/share/keyrings/kitware-archive-keyring.gpg
RUN apt-get -y install kitware-archive-keyring
RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy-rc main' | tee -a /etc/apt/sources.list.d/kitware.list >/dev/null
RUN apt-get update
RUN apt-get -y install cmake

# libtorch
RUN curl --output libtorch.zip "https://download.pytorch.org/libtorch/cu124/libtorch-cxx11-abi-shared-with-deps-2.4.0%2Bcu124.zip"
RUN unzip libtorch.zip -d /opt/
RUN rm -f libtorch.zip

RUN echo "Will build evo_motion"

# create EvoMotion folders
RUN mkdir $EVO_MOTION_ROOT
RUN mkdir $EVO_MOTION_ROOT/evo_motion_model
RUN mkdir $EVO_MOTION_ROOT/evo_motion_networks
RUN mkdir $EVO_MOTION_ROOT/evo_motion_view
RUN mkdir $EVO_MOTION_ROOT/src
RUN mkdir $EVO_MOTION_ROOT/resources

# copy EvoMotion stuff
ADD ./evo_motion_model/ $EVO_MOTION_ROOT/evo_motion_model/
ADD ./evo_motion_networks/ $EVO_MOTION_ROOT/evo_motion_networks/
ADD ./evo_motion_view/ $EVO_MOTION_ROOT/evo_motion_view/
ADD ./resources/ $EVO_MOTION_ROOT/resources/
ADD ./src/ $EVO_MOTION_ROOT/src/
ADD ./CMakeLists.txt $EVO_MOTION_ROOT/CMakeLists.txt

# build EvoMotion
RUN cmake -DCAFFE2_USE_CUDNN=1 -DTORCH_CUDA_ARCH_LIST="8.0 8.6 8.9 9.0" -S /opt/evo_motion/ -B /opt/evo_motion/build
RUN make -C /opt/evo_motion/build

ENTRYPOINT ["/opt/evo_motion/build/evo_motion"]
