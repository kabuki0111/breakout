cmake_minimum_required(VERSION 3.20)
project(Breakout CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(SDL3 REQUIRED CONFIG)

add_executable(breakout
    src/main.cpp
    src/Game.cpp
    src/GameRender.cpp
    src/Sfx.cpp
    src/BallTexture.cpp
    src/Level.cpp
)
target_link_libraries(breakout PRIVATE SDL3::SDL3)

# ステージファイル（levels/）をビルドのたびに実行ファイルの隣へコピーする。
# POST_BUILD だとソースに変更が無い時は実行されず、ステージだけ編集しても反映されないため、
# 毎回必ず動く独立したターゲットにしている。
add_custom_target(copy_levels ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/levels $<TARGET_FILE_DIR:breakout>/levels
    COMMENT "Copying levels/ next to the executable"
)
add_dependencies(breakout copy_levels)

# ソースは BOM なし UTF-8。MSVC は既定で Shift-JIS として読むため、日本語コメントで壊れないよう指定する
if(MSVC)
    target_compile_options(breakout PRIVATE /utf-8)
endif()
