/**
 * @file GameRender.cpp
 * @brief Game クラスのうち、画面描画（Draw）の実装。
 *
 * 描画は「背景 → ブロック → パドル → ボール → 文字（HUD）」の順に重ねていく。
 * 後から描いたものほど手前に表示される。
 */
#include "Game.h"

#include <string>

#include "Config.h"

void Game::Draw(SDL_Renderer* r) const {
    // 背景: 暗い紺色で画面全体を塗りつぶす（前フレームの描画内容を消す役割もある）
    SDL_SetRenderDrawColor(r, 20, 20, 30, 255);
    SDL_RenderClear(r);

    // ブロック: 種類ごとの色（ステージファイルの '1'〜'6' = 赤・橙・黄・緑・青・紫）
    static const SDL_Color kindColor[kBrickKinds] = {
        {235,  80,  80, 255}, {240, 150,  60, 255}, {240, 220,  70, 255},
        { 90, 210, 110, 255}, { 80, 160, 240, 255}, {170, 110, 240, 255}};
    for (const auto& b : bricks_) {
        if (!b.alive) continue;  // 壊れたブロックは描かない
        const SDL_Color c = kindColor[b.kind];
        SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 255);
        SDL_RenderFillRect(r, &b.r);
    }

    // パドル: 明るい灰色の四角
    SDL_SetRenderDrawColor(r, 230, 230, 230, 255);
    SDL_RenderFillRect(r, &paddle_);

    // ボール: 球体テクスチャを当たり判定の四角にぴったり収まるよう引き伸ばして描く。
    // テクスチャの生成に失敗していた場合は、代わりにパドルと同じ色の四角で描く。
    if (ballTex) SDL_RenderTexture(r, ballTex, nullptr, &ball_);
    else         SDL_RenderFillRect(r, &ball_);

    // HUD（スコアとライフ）: SDL 内蔵の簡易フォント（1文字 8×8px）で左上に表示
    const std::string hud = "SCORE " + std::to_string(score_) + "   LIFE " + std::to_string(lives_);
    SDL_RenderDebugText(r, 10, 10, hud.c_str());

    // 状態ごとの案内メッセージを画面中央付近に表示。
    // x 座標は「画面中央 − 文字列の幅の半分（文字数 × 8px ÷ 2）」でおおよそ中央揃えにしている。
    if (state == State::Ready)    SDL_RenderDebugText(r, kW / 2 - 100, kH / 2, "PRESS SPACE TO LAUNCH");
    if (state == State::GameOver) SDL_RenderDebugText(r, kW / 2 - 80,  kH / 2, "GAME OVER - PRESS R");
    if (state == State::Clear)    SDL_RenderDebugText(r, kW / 2 - 60,  kH / 2, "CLEAR! - PRESS R");

    // ここまで裏側のバッファに描いた内容を、まとめて画面に表示する
    SDL_RenderPresent(r);
}
