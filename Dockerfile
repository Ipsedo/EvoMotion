FROM nvidia/cuda:12.4.1-cudnn-devel-ubuntu22.04

ENV EVO_MOTION_ROOT="/opt/evo_motion"

# Install dependencies
RUN apt-get update --fix-missing
RUN apt-get -y install ca-certificates cmake gpg wget git libglm-dev libglew-dev libglfw3-dev curl unzip libgtest-dev

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

# bullet
RUN git clone https://github.com/bulletphysics/bullet3.git -b 3.25 /opt/bullet3
RUN mkdir /opt/bullet3/build
RUN cmake -DBULLET2_MULTITHREADING=ON -DBUILD_SHARED_LIBS=ON -S /opt/bullet3/ -B /opt/bullet3/build
RUN make -j 8 -C /opt/bullet3/build
RUN make install -C /opt/bullet3/build
RUN ldconfig /opt/bullet3/build

# ImGui
RUN git clone https://github.com/ocornut/imgui.git /opt/imgui
RUN mkdir /opt/imgui/build
RUN curl https://raw.githubusercontent.com/microsoft/vcpkg/16601c6e7ee15aeccac771185916cd6f6fe1ba50/ports/imgui/CMakeLists.txt -o /opt/imgui/CMakeLists.txt
RUN curl https://raw.githubusercontent.com/microsoft/vcpkg/16601c6e7ee15aeccac771185916cd6f6fe1ba50/ports/imgui/imgui-config.cmake.in -o /opt/imgui/imgui-config.cmake.in
RUN cmake -DIMGUI_BUILD_GLFW_BINDING=ON -DBUILD_SHARED_LIBS=ON -DIMGUI_BUILD_OPENGL3_BINDING=ON -S /opt/imgui/ -B /opt/imgui/build
RUN make -j 8 -C /opt/imgui/build
RUN make install -C /opt/imgui/build
RUN ldconfig /opt/imgui/build

RUN echo "Will build evo_motion"

# create EvoMotion folders
RUN cd /opt/
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
RUN make -j 8 -C /opt/evo_motion/build

ENTRYPOINT ["/opt/evo_motion/build/evo_motion"]
