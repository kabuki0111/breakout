/**
 * @file Config.h
 * @brief ゲーム全体で共有する定数（画面サイズ・速度・ブロックの大きさと配置範囲）をまとめたヘッダ。
 *
 * 調整したい数値はここに集約しておき、各処理ファイルからはこのヘッダを
 * インクルードして参照する。値を変えるだけでゲームバランスを調整できる。
 * ブロックの並べ方そのものは levels/ フォルダのテキストファイルで指定する（Level.h 参照）。
 */
#pragma once

/// @brief ウィンドウ（ゲーム画面）の幅 [px]。
constexpr int kW = 640;

/// @brief ウィンドウ（ゲーム画面）の高さ [px]。
constexpr int kH = 480;

/// @brief パドルの移動速度 [px/秒]。←→キーを押している間この速さで動く。
constexpr float kPaddleSpeed = 480.0f;

/// @brief ボールの速さ [px/秒]。反射で向きは変わるが、速さは常にこの値を保つ。
constexpr float kBallSpeed = 360.0f;

/// @brief ブロックの種類の数。種類ごとに色・得点・効果音の音程が変わる（ステージファイルの '1'〜'6'）。
constexpr int kBrickKinds = 6;

/// @brief ブロック 1 個の幅 [px]。
constexpr float kBrickW = 56.0f;

/// @brief ブロック 1 個の高さ [px]。
constexpr float kBrickH = 18.0f;

/// @brief 隣り合うブロック同士の隙間 [px]。
constexpr float kBrickGap = 6.0f;

/// @brief 一番上の段のブロックの y 座標 [px]。上部はスコア表示用に空けておく。
constexpr float kBrickTop = 60.0f;

/// @brief 横に並べられるブロックの最大数。(56 + 6) × 10 − 6 = 614px で画面幅 640px に収まる上限。
constexpr int kMaxCols = 10;

/// @brief 縦に並べられるブロックの最大段数。60 + (18 + 6) × 14 = 396px で、パドル（y = 440）との間に余裕を残す上限。
constexpr int kMaxRows = 14;
