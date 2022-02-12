cmake_minimum_required(VERSION 3.0.0)
project(ffmpeg_music_sample VERSION 0.1.0)



add_executable(${PROJECT_NAME} main.cpp miniaudio.h)
target_link_libraries(${PROJECT_NAME} avformat avcodec avutil swresample pthread m dl)

