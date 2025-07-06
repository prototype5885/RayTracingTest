FROM archlinux:latest

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
    git \
    base-devel \
    cmake \
    mingw-w64-gcc

ARG USERNAME=aurbuilder
RUN useradd -m $USERNAME && echo "$USERNAME ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER $USERNAME
WORKDIR /home/$USERNAME

RUN git clone https://aur.archlinux.org/yay-bin.git \
    && cd yay-bin \
    && makepkg -si --noconfirm \
    && cd .. \
    && rm -rf yay-bin

RUN yay -S --noconfirm mingw-w64-sdl2

ENV CC=x86_64-w64-mingw32-gcc
ENV CXX=x86_64-w64-mingw32-g++

COPY --chown=$USERNAME:$USERNAME . /app
WORKDIR /app

RUN mkdir build
WORKDIR /app/build

RUN cmake ..

RUN make
