#/bin/bash

function install_dep() 
{
    sudo apt update
    sudo apt upgrade
    sudo apt install build-essential
    sudo apt install -y cmake
    sudo apt install -y libsdl2-dev
}

function setup_whisper_fuzzy()
{
    cp ./cmake/CMakeLists.txt ./whisper.cpp/
    cp ./cmake/examples/CMakeLists.txt ./whisper.cpp/examples
}

function build_whisper_cpp() 
{
    cmake -B build -DWHISPER_SDL2=ON -DENABLE_GDB=OFF -S whisper.cpp
    cmake --build build --config Release

    ./build/bin/whisper-cli -m ./whisper.cpp/models/ggml-base.en-q5_1.bin ./whisper.cpp/samples/jfk.wav 
    if [[ 0 -ne $? ]]; then
        echo "test whisper.cpp err code: $?"
        exit 1
    fi
}

install_dep
setup_whisper_fuzzy

if [[ x"build" == x"$1" ]]; then
    build_whisper_cpp
fi
