/**
 * @file Game.cpp
 * @brief Game クラスのうち、初期化・発射・更新（移動と当たり判定）の実装。
 */
#include "Game.h"

#include <algorithm>
#include <cmath>

#include "Config.h"
#include "Level.h"
#include "Sfx.h"

void Game::SetLevel(const std::string& text) { levelText_ = text; }

void Game::ResetAll() {
    lives_ = 3;
    score_ = 0;

    // ステージのテキストからブロックを並べ直す（壊したブロックもすべて復活する）
    bricks_ = ParseLevel(levelText_);

    ResetBall();
}

void Game::ResetBall() {
    // パドル: 幅 100 × 高さ 12、画面下から 40px の位置で左右中央に置く
    paddle_ = {kW / 2.0f - 50, kH - 40.0f, 100, 12};

    // ボール: 10 × 10。位置は Update() の発射待ち処理でパドルの上に合わせるので 0 でよい
    ball_ = {0, 0, 10, 10};
    vx_ = vy_ = 0;
    state = State::Ready;
}

void Game::Launch() {
    // 真上から右へ 30 度傾けた方向（水平から見て 60 度）へ打ち出す。
    // sin30° = 0.5, cos30° ≈ 0.866。y は上向きが負なので vy をマイナスにする。
    vx_ = kBallSpeed * 0.5f;
    vy_ = -kBallSpeed * 0.866f;
    state = State::Playing;
    if (sfx) sfx->PlayLaunch();
}

void Game::Update(float dt) {
    // パドルは発射待ち中もプレイ中も動かせる
    MovePaddle(dt);

    // 発射待ち: ボールをパドル中央の真上に乗せたまま一緒に動かす
    if (state == State::Ready) {
        ball_.x = paddle_.x + paddle_.w / 2 - ball_.w / 2;
        ball_.y = paddle_.y - ball_.h - 1;
        return;
    }
    // ゲームオーバー・クリア中は何も動かさない（R キー待ち）
    if (state != State::Playing) return;

    // ボールを速度 × 経過時間 だけ進める
    ball_.x += vx_ * dt;
    ball_.y += vy_ * dt;

    BounceWalls();

    // 画面の下に落ちたらミス。ライフを減らし、0 になればゲームオーバー
    if (ball_.y > kH) {
        if (--lives_ <= 0) state = State::GameOver;
        else ResetBall();
        return;
    }

    BouncePaddle();
    HitBricks();

    // 生き残っているブロックが 1 つもなければクリア
    if (std::none_of(bricks_.begin(), bricks_.end(), [](const Brick& b) { return b.alive; }))
        state = State::Clear;
}

void Game::MovePaddle(float dt) {
    // SDL_GetKeyboardState は「今押されているキー」の一覧を返す。
    // イベントではなく状態を見るので、押しっぱなしで滑らかに動く。
    const bool* keys = SDL_GetKeyboardState(nullptr);
    if (keys[SDL_SCANCODE_LEFT])  paddle_.x -= kPaddleSpeed * dt;
    if (keys[SDL_SCANCODE_RIGHT]) paddle_.x += kPaddleSpeed * dt;

    // 画面の外にはみ出さないよう、左端 0 〜 右端（画面幅 − パドル幅）に制限する
    paddle_.x = std::clamp(paddle_.x, 0.0f, kW - paddle_.w);
}

void Game::BounceWalls() {
    // 壁にめり込んだ分は壁の位置まで押し戻し、速度の向きを壁から離れる方向に揃える。
    // （単純に符号反転すると、めり込んだまま毎フレーム反転を繰り返して壁に張り付くことがあるため、
    //   fabs で「必ずこの向き」と決め打ちしている）
    if (ball_.x < 0)              { ball_.x = 0;              vx_ =  std::fabs(vx_); }  // 左の壁 → 右へ
    if (ball_.x + ball_.w > kW)   { ball_.x = kW - ball_.w;   vx_ = -std::fabs(vx_); }  // 右の壁 → 左へ
    if (ball_.y < 0)              { ball_.y = 0;              vy_ =  std::fabs(vy_); }  // 天井   → 下へ
}

void Game::BouncePaddle() {
    // 下向きに移動中（vy > 0）の時だけ判定する。
    // 上向きの時に判定すると、跳ね返った直後にまた当たって何度も反射してしまう。
    if (vy_ <= 0 || !SDL_HasRectIntersectionFloat(&ball_, &paddle_)) return;

    // パドルのどこに当たったかを -1（左端）〜 0（中央）〜 +1（右端）で表す
    float hit = ((ball_.x + ball_.w / 2) - (paddle_.x + paddle_.w / 2)) / (paddle_.w / 2);
    hit = std::clamp(hit, -1.0f, 1.0f);

    // 当たった位置に応じて反射角を決める（中央なら真上、端ほど最大 60 度まで斜めに）。
    // 角度をラジアンに変換してから、速さ kBallSpeed を保ったまま x, y 成分に分解する。
    const float angle = hit * 60.0f * SDL_PI_F / 180.0f;
    vx_ =  kBallSpeed * std::sin(angle);
    vy_ = -kBallSpeed * std::cos(angle);

    // パドルにめり込んだ分をパドルの上面まで押し戻す
    ball_.y = paddle_.y - ball_.h;

    if (sfx) sfx->PlayPaddle();
}

void Game::HitBricks() {
    for (auto& b : bricks_) {
        // 壊れているブロックや、ボールと重なっていないブロックは無視
        if (!b.alive || !SDL_HasRectIntersectionFloat(&ball_, &b.r)) continue;

        b.alive = false;

        // 種類ごとの得点（'1' の赤 = 60 点 〜 '6' の紫 = 10 点）
        score_ += (kBrickKinds - b.kind) * 10;
        if (sfx) sfx->PlayHit(b.kind);

        // ボールとブロックの重なり幅を x 方向・y 方向それぞれで求める。
        // 重なりが小さい方の軸から当たったとみなす:
        //  - 横の重なりが小さい → 側面に当たった → 左右の向きを反転
        //  - 縦の重なりが小さい → 上下の面に当たった → 上下の向きを反転
        const float ox = std::min(ball_.x + ball_.w, b.r.x + b.r.w) - std::max(ball_.x, b.r.x);
        const float oy = std::min(ball_.y + ball_.h, b.r.y + b.r.h) - std::max(ball_.y, b.r.y);
        if (ox < oy) vx_ = -vx_;
        else         vy_ = -vy_;

        // 1 フレームに壊すのは 1 個まで。
        // 2 個同時に当たった時に反射が 2 回起きて、向きが元に戻ってしまうのを防ぐ。
        break;
    }
}
