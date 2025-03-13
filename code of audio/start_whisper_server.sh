#se.en-q5_1.bin!/bin/bash

MODEL=models/ggml-base.en-q5_1.bin

cd whisper.cpp-1.7.4 && ./build/bin/whisper-server --port 8090 -m $MODEL
