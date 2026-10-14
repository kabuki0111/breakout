/**
 * @file Sfx.h
 * @brief 効果音（SE）の生成と再生を担当するクラスの宣言。
 *
 * 音声ファイルは使わず、起動時に矩形波（レトロゲーム風の「ピコッ」という音）を
 * プログラムで合成してメモリに保持しておき、必要なタイミングで SDL3 の
 * オーディオストリームへ流し込んで鳴らす。
 */
#pragma once

#include <SDL3/SDL.h>
#include <vector>

#include "Config.h"

/**
 * @brief 効果音の管理クラス。
 *
 * 使い方:
 *  1. SDL_Init(SDL_INIT_AUDIO) の後に Init() を呼ぶ。
 *  2. ゲーム中に PlayLaunch() / PlayHit() を呼ぶと音が鳴る。
 *  3. 終了時に Quit() を呼んでオーディオ資源を解放する。
 *
 * Init() に失敗した場合でも Play 系の関数は安全に何もしないので、
 * 音が出ない環境でもゲームはそのまま動作する。
 */
class Sfx {
public:
    /**
     * @brief オーディオデバイスを開き、効果音の波形を事前に生成する。
     * @return 成功したら true。デバイスが開けなかった場合は false（以後は無音）。
     */
    bool Init();

    /// @brief Init() で確保したストリームとオーディオデバイスを解放する。
    void Quit();

    /// @brief ボール発射音（低い音から高い音へ上がる「ピュイッ」）を鳴らす。
    void PlayLaunch();

    /// @brief ボールがパドルに当たった音（低めの「ポン」）を鳴らす。
    void PlayPaddle();

    /**
     * @brief ブロックに当たった音（短い「ピッ」）を鳴らす。
     * @param kind 当たったブロックの種類（0〜kBrickKinds-1）。番号が小さいほど高い音になる。
     */
    void PlayHit(int kind);

private:
    /// @brief 出力サンプリングレート [Hz]。1秒あたりのサンプル数。
    static constexpr int kRate = 44100;

    /**
     * @brief 周波数を f0 から f1 へ滑らかに変化させながら減衰する矩形波を生成する。
     * @param f0  開始時の周波数 [Hz]
     * @param f1  終了時の周波数 [Hz]（f0 と同じにすると一定の音程になる）
     * @param sec 音の長さ [秒]
     * @param vol 音量（0.0〜1.0。大きすぎると音割れするので 0.2 程度が目安）
     * @return -vol〜+vol の範囲の float サンプル列（モノラル）
     */
    static std::vector<float> Tone(float f0, float f1, float sec, float vol);

    /**
     * @brief 指定ストリームで波形を頭から再生し直す。
     * @param s 再生に使うオーディオストリーム（nullptr なら何もしない）
     * @param w 再生する波形データ
     */
    static void Play(SDL_AudioStream* s, const std::vector<float>& w);

    SDL_AudioDeviceID dev_ = 0;           ///< 開いた再生デバイスの ID（0 = 未オープン）
    SDL_AudioStream* launch_ = nullptr;   ///< 発射音専用のストリーム
    SDL_AudioStream* paddle_ = nullptr;   ///< パドル衝突音専用のストリーム
    SDL_AudioStream* hit_ = nullptr;      ///< ブロック衝突音専用のストリーム
    std::vector<float> launchWave_;       ///< 発射音の波形データ
    std::vector<float> paddleWave_;       ///< パドル衝突音の波形データ
    std::vector<float> hitWave_[kBrickKinds];  ///< 衝突音の波形データ（種類ごとに音程が違う）
};
