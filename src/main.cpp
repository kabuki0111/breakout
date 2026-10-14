/**
 * @file main.cpp
 * @brief SDL3 ブロック崩しのエントリーポイント。
 *
 * SDL の初期化、ウィンドウの作成、メインループ（入力 → 更新 → 描画）、終了処理を行う。
 *
 * 操作: ←→ でパドル移動 / Space で発射 / R でリスタート / Esc で終了
 */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>  // Windows などで main() を各 OS の正しい入口に繋ぐために必要
#include <algorithm>
#include <string>

#include "BallTexture.h"
#include "Config.h"
#include "Game.h"
#include "Level.h"
#include "Sfx.h"

/**
 * @brief キーが押された時の処理（発射・リスタート・終了）を行う。
 * @param key     押されたキー
 * @param game    ゲーム本体
 * @param running メインループを続けるかどうか（Esc で false にする）
 *
 * パドル移動は押しっぱなしで動かしたいので、ここではなく Game::Update() 内で
 * キーの状態を直接調べている。ここでは「押した瞬間に1回だけ」行う操作を扱う。
 */
static void HandleKeyDown(SDL_Keycode key, Game& game, bool& running) {
    if (key == SDLK_ESCAPE) running = false;

    // 発射は発射待ちの時だけ
    if (key == SDLK_SPACE && game.state == State::Ready) game.Launch();

    // リスタートはゲームオーバー・クリアの時だけ（プレイ中に誤って押しても何も起きない）
    if (key == SDLK_R && (game.state == State::GameOver || game.state == State::Clear))
        game.ResetAll();
}

/**
 * @brief 遊ぶステージのテキストを用意する。
 * @param argc コマンドライン引数の数
 * @param argv コマンドライン引数。1 つ目にステージファイルのパスを渡すとそれを使う
 * @return ステージのテキスト
 *
 * 読み込むファイルの優先順位:
 *  1. コマンドライン引数で指定されたファイル（例: breakout.exe levels\\level2.txt）
 *  2. 実行ファイルと同じフォルダにある levels/level1.txt（ビルド時に自動でコピーされる）
 *  3. どちらも読めなければ、プログラムに組み込んだ既定のステージ
 */
static std::string LoadLevelText(int argc, char** argv) {
    std::string path;
    if (argc >= 2) {
        path = argv[1];
    } else {
        // SDL_GetBasePath は実行ファイルのあるフォルダ（末尾に区切り文字付き）を返す。
        // 作業フォルダ（どこから起動したか）に左右されずにファイルを見つけられる。
        const char* base = SDL_GetBasePath();
        path = std::string(base ? base : "") + "levels/level1.txt";
    }

    std::string text;
    if (ReadTextFile(path, text)) {
        SDL_Log("Level: '%s' を読み込みました", path.c_str());
        return text;
    }
    SDL_Log("Level: 組み込みの既定ステージを使います");
    return kDefaultLevel;
}

int main(int argc, char** argv) {
    // ---- 初期化 ----
    // 映像（ウィンドウ・描画）と音声のサブシステムを使う
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return 1;
    }

    // ウィンドウと、そこへ描画するためのレンダラーを同時に作成
    SDL_Window* win = nullptr;
    SDL_Renderer* ren = nullptr;
    if (!SDL_CreateWindowAndRenderer("Breakout", kW, kH, 0, &win, &ren)) {
        SDL_Log("CreateWindow: %s", SDL_GetError());
        return 1;
    }

    // 垂直同期を有効にする。モニターの更新タイミングに合わせて表示するので、
    // 画面のちらつき（ティアリング）を防ぎ、ループが無駄に高速回転して CPU を食うのも防げる。
    SDL_SetRenderVSync(ren, 1);

    // 効果音。オーディオデバイスが無いなどで失敗しても、無音のままゲームは続行する
    Sfx sfx;
    sfx.Init();

    // ゲーム本体を準備し、効果音とボールのテクスチャを渡す
    Game game;
    game.sfx = &sfx;
    game.ballTex = MakeBallTexture(ren);
    game.SetLevel(LoadLevelText(argc, argv));
    game.ResetAll();

    // ---- メインループ ----
    Uint64 prev = SDL_GetTicksNS();  // 前フレームの時刻 [ナノ秒]
    bool running = true;

    while (running) {
        // 1) 入力: 溜まっているイベントをすべて処理する
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;  // ウィンドウの × ボタン
            else if (e.type == SDL_EVENT_KEY_DOWN) HandleKeyDown(e.key.key, game, running);
        }

        // 2) 経過時間の計算: 前フレームからの時間 dt [秒] を求める。
        //    移動量を dt に比例させることで、PC の速さ（フレームレート）に関係なく同じ速度で動く。
        //    ウィンドウのドラッグ中などで処理が止まると dt が大きくなり、ボールが一気に進んで
        //    ブロックやパドルをすり抜けてしまうので、最大 50ms に制限する。
        const Uint64 now = SDL_GetTicksNS();
        const float dt = std::min((now - prev) / 1e9f, 0.05f);
        prev = now;

        // 3) 更新と描画
        game.Update(dt);
        game.Draw(ren);
    }

    // ---- 終了処理 ----
    // 作成したものを作成と逆の順番で解放する
    sfx.Quit();
    SDL_DestroyTexture(game.ballTex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
