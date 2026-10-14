/**
 * @file BallTexture.cpp
 * @brief 球体テクスチャの生成処理の実装。
 *
 * 各ピクセルについて「球の表面がどちらを向いているか（法線）」を計算し、
 * 光の向きとの関係から明るさを決める、簡易的な 3D ライティングを行っている。
 */
#include "BallTexture.h"

#include <algorithm>
#include <cmath>
#include <vector>

SDL_Texture* MakeBallTexture(SDL_Renderer* r) {
    // テクスチャの一辺のピクセル数。実際のボール(10px)より大きく作り、縮小表示で滑らかにする。
    constexpr int S = 64;

    // RGBA 各 1byte × S × S のピクセルバッファ
    std::vector<Uint8> px(S * S * 4);

    // 光源の方向ベクトル（長さ 1 に正規化済み）。
    // x が負 = 左から、y が負 = 上から、z が正 = 手前（画面のこちら側）から光が来る。
    const float lx = -0.5f, ly = -0.6f, lz = 0.62f;

    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            // ピクセル中心の座標を -1〜+1 に変換（テクスチャ中央が原点、円の半径が 1）
            const float nx = (x + 0.5f) / S * 2 - 1;
            const float ny = (y + 0.5f) / S * 2 - 1;

            // 中心からの距離。d <= 1 の範囲が球（円）の内側。
            const float d = std::sqrt(nx * nx + ny * ny);

            // 不透明度: 円の内側は 1、外側は 0。
            // 境界付近の約 1 ピクセル幅だけ 0〜1 の中間値にして、縁のギザギザを消す（アンチエイリアス）。
            const float alpha = std::clamp((1 - d) * S / 2 + 0.5f, 0.0f, 1.0f);

            // 球の表面の手前方向の成分 z。x² + y² + z² = 1 （単位球）から求める。
            // これで (nx, ny, nz) が表面の向き（法線ベクトル）になる。
            const float nz = std::sqrt(std::max(0.0f, 1 - d * d));

            // 拡散反射（ランバート反射）: 法線と光の向きの内積。光に正面を向いている所ほど明るい。
            const float diffuse = std::max(0.0f, nx * lx + ny * ly + nz * lz);

            // 鏡面反射（ハイライト）: diffuse を大きく累乗すると、最も明るい一点付近だけが残る。
            const float spec = std::pow(diffuse, 24.0f);

            // 最終的な明るさ:
            //  基本色 230 に対して、影の部分でも 30% は残す（環境光）＋ 拡散光 70%、
            //  そこへ白いハイライトを加算し、255 を超えないよう制限する。
            const float v = std::min(255.0f, 230 * (0.3f + 0.7f * diffuse) + 255 * 0.6f * spec);

            // RGBA32 はメモリ上で R, G, B, A の順に並ぶ。白〜灰色なので RGB は同じ値。
            Uint8* p = &px[(y * S + x) * 4];
            p[0] = p[1] = p[2] = Uint8(v);
            p[3] = Uint8(alpha * 255);
        }
    }

    // GPU 側にテクスチャを作成（STATIC = 一度書き込んだら以後は変更しない用途）
    SDL_Texture* tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, S, S);
    if (!tex) {
        SDL_Log("CreateTexture: %s", SDL_GetError());
        return nullptr;
    }

    // 作ったピクセルデータを転送（第4引数は 1行あたりのバイト数 = 幅 × 4byte）
    SDL_UpdateTexture(tex, nullptr, px.data(), S * 4);

    // アルファ値を使って背景と合成する（円の外側が透明になる）
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    // 縮小表示時に周囲のピクセルを補間して滑らかにする
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
    return tex;
}
