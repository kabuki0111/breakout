/**
 * @file Game.h
 * @brief ゲーム本体（状態・ルール・描画）を管理するクラスの宣言。
 *
 * 処理の実装は役割ごとに分けている:
 *  - Game.cpp       … 初期化・発射・毎フレームの更新（移動と当たり判定）
 *  - GameRender.cpp … 画面描画
 */
#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <vector>

class Sfx;

/// @brief ゲームの進行状態。
enum class State {
    Ready,     ///< 発射待ち（ボールがパドルの上に乗っている）
    Playing,   ///< プレイ中（ボールが動いている）
    GameOver,  ///< ライフが 0 になった
    Clear,     ///< ブロックを全部壊した
};

/// @brief ブロック 1 個分の情報。
struct Brick {
    SDL_FRect r;  ///< 位置と大きさ（左上 x, y, 幅 w, 高さ h）
    bool alive;   ///< まだ壊れていなければ true
    int kind;     ///< 種類（0〜kBrickKinds-1。ステージファイルの '1'〜'6'）。色・得点・効果音の音程を決める
};

/**
 * @brief ブロック崩しのゲーム本体。
 *
 * パドル・ボール・ブロックの状態と、スコアやライフなどの進行情報を持つ。
 * main() のループから毎フレーム Update() → Draw() の順に呼ばれる。
 */
class Game {
public:
    Sfx* sfx = nullptr;              ///< 効果音（nullptr なら無音）。所有はしない
    SDL_Texture* ballTex = nullptr;  ///< ボールの球体テクスチャ（nullptr なら四角で描く）。所有はしない
    State state = State::Ready;      ///< 現在の進行状態

    /**
     * @brief 遊ぶステージ（ブロック配置のテキスト）を設定する。
     * @param text ステージのテキスト（書式は Level.h 参照）。次の ResetAll() から反映される。
     */
    void SetLevel(const std::string& text);

    /// @brief ゲームを最初からやり直す（スコア・ライフを戻し、ステージのブロックを並べ直す）。
    void ResetAll();

    /// @brief パドルとボールだけを初期位置に戻し、発射待ち状態にする（ミスした後に使う）。
    void ResetBall();

    /// @brief 発射待ちのボールを斜め上に打ち出し、プレイ中状態にする。
    void Launch();

    /**
     * @brief 1 フレーム分ゲームを進める（パドル移動・ボール移動・当たり判定）。
     * @param dt 前フレームからの経過時間 [秒]。移動量はこれに比例させる。
     */
    void Update(float dt);

    /**
     * @brief 現在の状態を画面に描画する。
     * @param r 描画先のレンダラー
     */
    void Draw(SDL_Renderer* r) const;

private:
    /// @brief キー入力に応じてパドルを左右に動かす。
    void MovePaddle(float dt);

    /// @brief ボールが画面の左・右・上の壁に当たったら跳ね返す。
    void BounceWalls();

    /// @brief ボールがパドルに当たったら、当たった位置に応じた角度で跳ね返す。
    void BouncePaddle();

    /// @brief ボールが当たったブロックを壊し、得点加算・効果音・反射を行う。
    void HitBricks();

    SDL_FRect paddle_{};          ///< パドルの位置と大きさ
    SDL_FRect ball_{};            ///< ボールの位置と大きさ（当たり判定もこの四角で行う）
    float vx_ = 0, vy_ = 0;       ///< ボールの速度 [px/秒]（右・下が正）
    std::string levelText_;       ///< 現在のステージのテキスト。リスタート時もここから並べ直す
    std::vector<Brick> bricks_;   ///< 全ブロック
    int lives_ = 3;               ///< 残りライフ
    int score_ = 0;               ///< 現在のスコア
};
