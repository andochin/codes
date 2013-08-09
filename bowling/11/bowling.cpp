/*
 * bowling.cpp
 *
 *  Created on: 2013/07/18
 *      Author: ANDO Toshihiko
 */
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <tuple>
#include <utility>
#include <cstdio>
#include <cctype>

//
// 型の定義
//
enum frame_result_type { None, Normal, Spare, Strike, Last };
typedef std::vector<int> score_of_throw_type;
typedef score_of_throw_type::size_type score_index_type;
typedef std::tuple<score_index_type, frame_result_type> frame_type;
typedef std::vector<frame_type> game_type;

constexpr score_index_type& get_score_index_of_frame(frame_type& frame) { return std::get<0>(frame); }
constexpr const score_index_type& get_score_index_of_frame(const frame_type& frame) { return std::get<0>(frame); }
constexpr frame_result_type& get_frame_result(frame_type& frame) { return std::get<1>(frame); }
constexpr const frame_result_type& get_frame_result(const frame_type& frame) { return std::get<1>(frame); }

frame_type& set_score_index(frame_type& frame, score_index_type score_index) {
  get_score_index_of_frame(frame) = score_index;
  return frame;
}

frame_type& set_frame_result_type(frame_type& frame, frame_result_type frame_result) {
  std::get<1>(frame) = frame_result;
  return frame;
}

//
// 定数
//
const int number_of_pin = 10;                   // ピンの数
const int frames_in_game = 10;                  // フレーム数
const int max_throw = frames_in_game * 2 + 1;   // 最大投球数
const std::string valid_chars("0123456789SG");  // 入力可能文字

//
// フレームのスコアを計算する
// @param frame
// @param score_of_throw
//
int get_frame_score(const frame_type& frame, const score_of_throw_type& score_of_throw)
{
  auto frame_result = get_frame_result(frame);

  if(frame_result == None) {
    return 0;   // 未投球なら0
  }

  // 通常ならフレーム先頭から２投分。それ以外（ストライク・スペア・最終フレーム）なら３投分
  // の合計がフレームスコアとなる
  auto index = get_score_index_of_frame(frame);
  return std::accumulate(&score_of_throw[index],
                         &score_of_throw[index + (frame_result == Normal ? 2 : 3)]
                         , 0);
}


//
// スコア表を描画する
// @param frame_data
// @param score_of_throw
//
void draw_score(const game_type& frame_data, const score_of_throw_type& score_of_throw)
{
  int last_index = frames_in_game - 1;

  // 上段（投球結果）の出力
  for(int i = 0; i < last_index; ++i) {
    char score[2] = { '-', '-' };
    auto index = get_score_index_of_frame(frame_data[i]);

    switch(get_frame_result(frame_data[i])) {
    case Normal:
      score[0] = '0' + score_of_throw[index];
      score[1] = '0' + score_of_throw[index + 1];
      break;
    case Spare:
      score[0] = '0' + score_of_throw[index];
      score[1] = '/';
      break;
    case Strike:
      score[0] = 'X';
      score[1] = ' ';
      break;
    default:
      break;
    }
    std::printf("|%c|%c", score[0], score[1]);
  }

  // 最終フレームの上段の出力
  if(get_frame_result(frame_data[last_index]) == None) {
    // 未投球
    std::printf("|-|-|-|\n");
  } else {
    // 投球済みなら結果表示
    char score[3] = { '-', '-', '-' }; 
    size_t index = get_score_index_of_frame(frame_data[last_index]);

    score[0] = (score_of_throw[index] == number_of_pin) ? 'x' : '0' + score_of_throw[index];
    score[1] = (score_of_throw[index] == number_of_pin)
      ? (score_of_throw[index + 1] == number_of_pin) ? 'x' : '0' + score_of_throw[index + 1]
      : ((score_of_throw[index] + score_of_throw[index + 1]) == number_of_pin) ? '/' : '0' + score_of_throw[index + 1];
    score[2] = (score_of_throw[index + 2] == number_of_pin) ? 'x' : '0' + score_of_throw[index + 2];
    std::printf("|%c|%c|%c|\n", score[0], score[1], score[2]);
  }

  // 下段（フレーム結果）の出力
  for(int i = 0; i < last_index; ++i) {
    if(get_frame_result(frame_data[i]) == None) {
      std::printf("|   ");
    } else {
      int total = 0;
      for(int j = 0; j <= i; ++j) total += get_frame_score(frame_data[j], score_of_throw);
        std::printf("|%3d", total);
    }
  }

  // 最終フレームの下段の出力
  if(get_frame_result(frame_data[last_index]) == None) {
    // 未投球
    printf("|     |\n");
  } else {
    // 投球済み
    int total = 0;
    for(int i = 0; i <= last_index; ++i) total += get_frame_score(frame_data[i], score_of_throw);
    printf("|%5d|\n", total);
  }
}


//
// メイン
//
int main()
{
  using namespace std;

  score_of_throw_type   score_of_throw( max_throw, 0 );
  game_type             frame_data( frames_in_game, frame_type(18, None) );
  int                   frame_index = -1;
  size_t                throw_in_game = 0;
  bool                  next_frame = true;
  bool                  game_over = false;

  //
  // フレームループ開始
  // ループ処理は一投。最終フレームの最終投で終了。
  // 投げる数は投球結果によって異なるり、必ず一投以上投げるので、doループ
  //
  do {
    // 次のフレームになるか？
    if(next_frame) {
      // 次のフレーム。
      // フレーム数更新、フレームデータの生成。通常フレームか最終フレームかで生成が異なる。
      ++frame_index;
	  frame_data[frame_index] = make_tuple(throw_in_game, (frame_index == (frames_in_game - 1)) ? Last : Normal);
      next_frame = false;
    }

    auto& current_frame = frame_data[frame_index];
    auto throw_in_frame = throw_in_game - get_score_index_of_frame(current_frame);
    // 投球のスコアの入力
    char score;                             // スコア入力用
    std::cin >> score;                      // スコアの入力
    score = toupper(score);                 // 大文字へ変換
    score = (score == 'G') ? '0' : score;   // ガーターを表す'G'は'0'に変換
    auto pos = valid_chars.find(score);     // 有効文字テーブルから文字を検索。インデックスが入力値
    if(pos == std::string::npos) {          // 入力文字の有効チェック。posがnposであれば不正文字
      // 不正文字入力
      cout << score << " は不正な文字です:" << endl;
      continue;
    }

    // 投球のスコアを保存する。
    // スペアもしくはストライクであれば、残っているピンの数がスコア
    if(get_frame_result(current_frame) == Last) {
      // 最終フレーム
      if(score == 'S') {
        // ストライクもしくはスペア
        switch(throw_in_frame) {
        case 2:
          game_over = true;
        case 0: // 一投目。必ず総ピン数
          score_of_throw[throw_in_game] = number_of_pin;
          break;
        case 1: //二投目。 一投目がストライクならピン数、そうでなければ残ピン数
          score_of_throw[throw_in_game] = (score_of_throw[get_score_index_of_frame(current_frame)] != number_of_pin)
                                        ?  number_of_pin - score_of_throw[get_score_index_of_frame(current_frame)]
                                        : number_of_pin;
          break;
        default:
          exit(0);
        }
      } else {
        // 最終フレーム
        score_of_throw[throw_in_game] = pos;
        auto score_of_frame = score_of_throw[get_score_index_of_frame(current_frame)] + pos;
        if(((throw_in_frame == 1) && (score_of_frame < number_of_pin)) || (throw_in_frame == 2)) {
          game_over = true;
        }
      }
    } else {
      // 最終フレーム以外
      if(score == 'S') {
        // スペアもしくはストライクであれば、スコアは残りのピンの数
        score_of_throw[throw_in_game] = number_of_pin - score_of_throw[get_score_index_of_frame(current_frame)];
        set_frame_result_type(current_frame, (throw_in_frame == 0) ? Strike : Spare);
        next_frame = true;
      } else {
        // 入力されたピンの数がスコア
        score_of_throw[throw_in_game] = pos;
        if(throw_in_frame != 0) {
          next_frame = true;
        }
      }
    }

    // 現在の投球までのスコアの表示
    draw_score(frame_data, score_of_throw);
	// 次の投球へ
    ++throw_in_game;
  } while(!game_over);
}
