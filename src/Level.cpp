/**
 * @file Level.cpp
 * @brief ステージ（ブロック配置）の読み込みと解析の実装。
 */
#include "Level.h"

#include <SDL3/SDL.h>
#include <algorithm>

#include "Config.h"

const char* const kDefaultLevel =
    "1111111111\n"
    "2222222222\n"
    "3333333333\n"
    "4444444444\n"
    "5555555555\n"
    "6666666666\n";

bool ReadTextFile(const std::string& path, std::string& out) {
    // SDL_LoadFile はファイル全体を一度にメモリへ読み込み、そのサイズを返す。
    // 戻り値のメモリは SDL が確保したものなので、使い終わったら SDL_free で解放する。
    size_t size = 0;
    void* data = SDL_LoadFile(path.c_str(), &size);
    if (!data) {
        SDL_Log("Level: '%s' を読み込めませんでした: %s", path.c_str(), SDL_GetError());
        return false;
    }
    out.assign(static_cast<const char*>(data), size);
    SDL_free(data);
    return true;
}

std::vector<Brick> ParseLevel(const std::string& text) {
    // ---- 1) テキストを行に分け、ブロックの段として使う行だけを集める ----
    std::vector<std::string> rows;
    size_t pos = 0;
    while (pos <= text.size()) {
        // 次の改行までを 1 行として切り出す（最後の行は改行が無くてもよい）
        size_t end = text.find('\n', pos);
        if (end == std::string::npos) end = text.size();
        std::string line = text.substr(pos, end - pos);
        pos = end + 1;

        // Windows のメモ帳などで保存すると改行が "\r\n" になるので、行末の '\r' を取り除く
        if (!line.empty() && line.back() == '\r') line.pop_back();

        // 空行とコメント行は段として数えない
        if (line.empty() || line[0] == '#') continue;

        if (rows.size() >= size_t(kMaxRows)) {
            SDL_Log("Level: %d 段を超えた分は無視します", kMaxRows);
            break;
        }
        if (line.size() > size_t(kMaxCols)) {
            SDL_Log("Level: %d 列を超えた分は無視します（%d 段目）", kMaxCols, int(rows.size()) + 1);
            line.resize(kMaxCols);
        }
        rows.push_back(line);
    }

    // ---- 2) 一番長い行に合わせて配置全体の横幅を求め、画面の左右中央に揃える ----
    size_t cols = 0;
    for (const auto& r : rows) cols = std::max(cols, r.size());
    const float totalW = cols * kBrickW + (cols > 0 ? (cols - 1) * kBrickGap : 0);
    const float startX = (kW - totalW) / 2;

    // ---- 3) 1 文字ずつ見て、'1'〜'6' のマスにブロックを置く ----
    std::vector<Brick> bricks;
    for (size_t y = 0; y < rows.size(); ++y) {
        for (size_t x = 0; x < rows[y].size(); ++x) {
            const char c = rows[y][x];
            if (c == '.' || c == ' ') continue;  // 空きマス

            // '1'〜'6' を種類番号 0〜5 に変換する。それ以外の文字は空きマス扱いにして知らせる
            const int kind = c - '1';
            if (kind < 0 || kind >= kBrickKinds) {
                SDL_Log("Level: 不明な文字 '%c' は空きマスとして扱います（%d 段目 %d 列目）",
                        c, int(y) + 1, int(x) + 1);
                continue;
            }

            const SDL_FRect r{startX + x * (kBrickW + kBrickGap),
                              kBrickTop + y * (kBrickH + kBrickGap),
                              kBrickW, kBrickH};
            bricks.push_back({r, true, kind});
        }
    }

    if (bricks.empty()) SDL_Log("Level: ブロックが 1 個もありません");
    return bricks;
}
