/**
 * @file Level.h
 * @brief ステージ（ブロック配置）をテキストから読み込む処理の宣言。
 *
 * ステージファイルの書式:
 *  - 1 行がブロック 1 段、1 文字がブロック 1 個分のマスに対応する。
 *  - '1'〜'6' … ブロックを置く。数字が種類（色・得点・音程）を表す。
 *                1 = 赤(60点・高い音) 2 = 橙 3 = 黄 4 = 緑 5 = 青 6 = 紫(10点・低い音)
 *  - '.' や空白 … 何も置かない（空きマス）。
 *  - '#' で始まる行と空行 … 無視する（コメントやメモに使える）。
 *  - 最大 kMaxCols 列 × kMaxRows 段。はみ出した分は無視する。
 *  - 配置全体は画面の左右中央に揃えて表示する。
 */
#pragma once

#include <string>
#include <vector>

#include "Game.h"

/**
 * @brief ステージ用のテキストファイルを読み込む。
 * @param path 読み込むファイルのパス
 * @param out  読み込んだ内容の格納先
 * @return 読み込めたら true。ファイルが無い・開けない場合は false（out は変更しない）。
 */
bool ReadTextFile(const std::string& path, std::string& out);

/**
 * @brief ステージのテキストを解析して、ブロックの一覧を作る。
 * @param text ステージのテキスト（書式はファイル冒頭のコメント参照）
 * @return 配置されたブロックの一覧（すべて未破壊の状態）
 */
std::vector<Brick> ParseLevel(const std::string& text);

/// @brief ステージファイルが読み込めなかった時に使う、組み込みの既定ステージ（6 段 × 10 列）。
extern const char* const kDefaultLevel;
