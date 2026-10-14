/**
 * @file BallTexture.h
 * @brief ボール（球体）の見た目となるテクスチャを生成する関数の宣言。
 */
#pragma once

#include <SDL3/SDL.h>

/**
 * @brief 陰影付きの球体テクスチャを生成する。
 *
 * 左上から光が当たっているように明暗とハイライトを付け、縁はアンチエイリアス
 * （半透明でぼかす処理）して滑らかに見せる。描画サイズより大きな解像度で作り、
 * 縮小表示することで小さなボールでもきれいに表示できる。
 *
 * @param r テクスチャを作成するレンダラー
 * @return 生成したテクスチャ。失敗時は nullptr。
 *         不要になったら呼び出し側で SDL_DestroyTexture() すること。
 */
SDL_Texture* MakeBallTexture(SDL_Renderer* r);
