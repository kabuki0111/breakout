/**
 * @file Sfx.cpp
 * @brief 効果音クラス Sfx の実装。波形の合成とオーディオストリームへの出力を行う。
 */
#include "Sfx.h"

#include <algorithm>
#include <cmath>

std::vector<float> Sfx::Tone(float f0, float f1, float sec, float vol) {
    // 必要なサンプル数 = 長さ[秒] × サンプリングレート
    const int n = int(sec * kRate);
    std::vector<float> w(n);

    // phase は波形の「1周期のうちどこにいるか」を 0.0〜1.0 で表す。
    // 周波数が途中で変わっても波形が途切れないよう、毎サンプル少しずつ進める。
    float phase = 0;
    for (int i = 0; i < n; ++i) {
        const float t = float(i) / n;  // 音の始まりからの進み具合（0.0〜1.0）

        // 現在の周波数（f0→f1 を直線補間）÷ サンプリングレート ＝ 1サンプルで進む位相量
        phase += (f0 + (f1 - f0) * t) / kRate;
        phase -= std::floor(phase);  // 1.0 を超えたら 0.0 側へ戻す（小数部だけ残す）

        // 音量の包絡線（エンベロープ）:
        //  - 最初の 2ms で 0→1 に立ち上げる（いきなり鳴らすと「プチッ」というノイズが出るため）
        //  - その後は終わりに向かって直線的に小さくする（減衰して自然に消える）
        const float env = std::min(1.0f, i / (0.002f * kRate)) * (1.0f - t);

        // 矩形波: 周期の前半は +vol、後半は -vol を出力する
        w[i] = (phase < 0.5f ? vol : -vol) * env;
    }
    return w;
}

bool Sfx::Init() {
    // 出力フォーマット: 32bit 浮動小数点 / モノラル(1ch) / 44100Hz
    const SDL_AudioSpec spec{SDL_AUDIO_F32, 1, kRate};

    // 既定の再生デバイス（通常はスピーカーやヘッドホン）を開く
    dev_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (!dev_) {
        SDL_Log("OpenAudioDevice: %s", SDL_GetError());
        return false;
    }

    // 効果音ごとにストリームを分けてデバイスに接続（バインド）する。
    // 同じデバイスに接続されたストリームは SDL が自動で合成（ミックス）して出力するので、
    // 発射音・パドル音・ブロック音が同時に鳴ってもすべて聞こえる。
    launch_ = SDL_CreateAudioStream(&spec, &spec);
    paddle_ = SDL_CreateAudioStream(&spec, &spec);
    hit_    = SDL_CreateAudioStream(&spec, &spec);
    SDL_BindAudioStream(dev_, launch_);
    SDL_BindAudioStream(dev_, paddle_);
    SDL_BindAudioStream(dev_, hit_);

    // 発射音: 220Hz → 880Hz（2オクターブ上昇）、0.15秒
    launchWave_ = Tone(220, 880, 0.15f, 0.2f);

    // パドル音: 330Hz → 220Hz へ少し下がる低めの「ポン」、0.08秒。
    // ブロック音（523Hz 以上の高い音）より低くして、何に当たったか耳で区別できるようにする。
    paddleWave_ = Tone(330, 220, 0.08f, 0.25f);

    // 衝突音: ブロックの種類ごとに音程を変える。
    // 最後の種類（'6' の紫）を C5 (523.25Hz) とし、番号が 1 つ小さくなるごとに 2半音（全音）ずつ高くする。
    // 半音1つ分の周波数比は 2^(1/12) なので、2半音 × 段階数 分だけ累乗する。
    for (int kind = 0; kind < kBrickKinds; ++kind) {
        const float f = 523.25f * std::pow(2.0f, (kBrickKinds - 1 - kind) * 2 / 12.0f);
        hitWave_[kind] = Tone(f, f, 0.07f, 0.25f);
    }
    return true;
}

void Sfx::Quit() {
    if (!dev_) return;  // Init() に失敗していれば解放するものはない
    SDL_DestroyAudioStream(launch_);
    SDL_DestroyAudioStream(paddle_);
    SDL_DestroyAudioStream(hit_);
    SDL_CloseAudioDevice(dev_);
    launch_ = paddle_ = hit_ = nullptr;
    dev_ = 0;
}

void Sfx::Play(SDL_AudioStream* s, const std::vector<float>& w) {
    if (!s) return;  // オーディオ初期化に失敗している場合は無音

    // ストリームに残っている再生途中のデータを捨ててから新しく積む。
    // これにより、連続でブロックに当たった時も毎回頭から鳴り直し、音が溜まって遅れることがない。
    SDL_ClearAudioStream(s);
    SDL_PutAudioStreamData(s, w.data(), int(w.size() * sizeof(float)));
}

void Sfx::PlayLaunch() { Play(launch_, launchWave_); }

void Sfx::PlayPaddle() { Play(paddle_, paddleWave_); }

void Sfx::PlayHit(int kind) { Play(hit_, hitWave_[kind]); }
