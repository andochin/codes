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

enum frame_result_type { None, Normal, Spare, Strike, Last };
typedef std::vector<int> score_of_throw_type;
typedef score_of_throw_type::size_type score_index_type;
typedef std::pair<score_index_type, frame_result_type> frame_type;
typedef std::vector<frame_type> game_type;
score_index_type get_score_index(const frame_type& frame) { return frame.first; }
frame_type set_score_index(frame_type& frame, score_index_type score_index) {
  frame.first = score_index;
  return frame;
}
frame_result_type get_frame_result_type(const frame_type& frame) { return frame.second; }
frame_type& set_frame_result_type(frame_type& frame, frame_result_type frame_result) {
  frame.second = frame_result;
  return frame;
}

const int number_of_pin = 10;
const int frames_in_game = 10;
const int max_throw = frames_in_game * 2 + 1;
const std::string valid_chars("0123456789SG");

// フレームのスコアを計算する
int get_frame_score(const frame_type& frame, const score_of_throw_type& score_of_throw)
{
  int   result = 0;
  auto index = get_score_index(frame);

  switch(get_frame_result_type(frame)) {
  case Normal:
    result = score_of_throw[index] + score_of_throw[index + 1];
    break;
  case Spare:
  case Strike:
  case Last:
    result = std::accumulate(&score_of_throw[index], &score_of_throw[index+3], 0);
    break;
  default:
    break;
  }

  return result;
}

// スコア表を描画する
void draw_score(const game_type& frame_data, const score_of_throw_type& score_of_throw)
{
  int last_index = frames_in_game - 1;

  // Draw upper line (throw result)
  for(int i = 0; i < last_index; ++i) {
    char score[2] = { '-', '-' };
	size_t index = get_score_index(frame_data[i]);

    switch(get_frame_result_type(frame_data[i])) {
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
  // Last frame
  if(get_frame_result_type(frame_data[last_index]) == Last) {
    char score[3] = { '-', '-', '-' }; 
	size_t index = get_score_index(frame_data[last_index]);

	score[0] = (score_of_throw[index] == number_of_pin) ? 'x' : '0' + score_of_throw[index];
	score[1] = (score_of_throw[index] == number_of_pin)
	  ? (score_of_throw[index + 1] == number_of_pin) ? 'x' : '0' + score_of_throw[index + 1]
	  : ((score_of_throw[index] + score_of_throw[index + 1]) == number_of_pin) ? '/' : '0' + score_of_throw[index + 1];
    score[2] = (score_of_throw[index + 2] == number_of_pin) ? 'x' : '0' + score_of_throw[index + 2];
    std::printf("|%c|%c|%c|\n", score[0], score[1], score[2]);
  } else {
    std::printf("|-|-|-|\n");
  }

  // Draw bottom line (frame result)
  for(int i = 0; i < last_index; ++i) {
    if(get_frame_result_type(frame_data[i]) == None) {
      std::printf("|   ");
    } else {
      int total = 0;
      for(int j = 0; j <= i; ++j) total += get_frame_score(frame_data[j], score_of_throw);
        std::printf("|%3d", total);
    }
  }
  // Last frame
  if(get_frame_result_type(frame_data[last_index]) == Last) {
    int total = 0;
    for(int i = 0; i <= last_index; ++i) total += get_frame_score(frame_data[i], score_of_throw);
    printf("|%5d|\n", total);
  } else {
    printf("|     |\n");
  }
}


int main()
{
  using namespace std;

  score_of_throw_type   score_of_throw( max_throw, 0 );
  game_type             frame_data( frames_in_game, frame_type(18, None) );
  int                   frame_index = -1;
  size_t                throw_in_game = 0;
  bool                  next_frame = true;
  bool                  game_over = false;

  do {
    if(next_frame) {
      ++frame_index;
	  frame_data[frame_index] = make_pair(throw_in_game, (frame_index == (frames_in_game - 1)) ? Last : Normal);
      next_frame = false;
    }

    auto& current_frame = frame_data[frame_index];
    auto throw_in_frame = throw_in_game - get_score_index(current_frame);
    // 投球のスコアの入力
    char score;
    std::cin >> score;
    score = toupper(score); // Convert to upper case1
    score = (score == 'G') ? '0' : score;
    auto pos = valid_chars.find(score);
    // Check input character.
    if(pos == std::string::npos) {
      // Input character is incorrect.
      cout << "Invalid character: " << score << endl;
      continue;
    }

    // 投球のスコアを保存する。投球がスペアもしくはストライクであれば、残っているピンの数がスコア
    if(get_frame_result_type(current_frame) == Last) {
      // 最終フレーム
      if(score == 'S') {
        switch(throw_in_frame) {
        case 2:
          game_over = true;
        case 0:
          score_of_throw[throw_in_game] = number_of_pin;
          break;
        case 1:
          score_of_throw[throw_in_game] = (score_of_throw[get_score_index(current_frame)] != number_of_pin)
                                        ?  number_of_pin - score_of_throw[get_score_index(current_frame)]
                                        : number_of_pin;
          break;
        default:
          exit(0);
        }
      } else {
        score_of_throw[throw_in_game] = pos;
        auto score_of_frame = score_of_throw[get_score_index(current_frame)] + pos;
        if(((throw_in_frame == 1) && (score_of_frame < number_of_pin)) || (throw_in_frame == 2)) {
          game_over = true;
        }
      }
    } else {
      // 最終フレーム以外
      if(score == 'S') {
        // スペアもしくはストライクであれば、スコアは残りのピンの数
        score_of_throw[throw_in_game] = number_of_pin - score_of_throw[get_score_index(current_frame)];
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
