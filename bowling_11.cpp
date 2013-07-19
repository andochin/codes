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
#include <cstdio>
#include <cctype>

enum point_type { None, Normal, Spare, Strike, Last };
typedef std::vector<int> point_of_throw_type;
typedef point_of_throw_type::size_type point_index_type;
typedef std::pair<point_index_type, point_type> frame_type;
typedef std::vector<frame_type> game_type;

const int number_of_pin = 10;
const int frames_in_game = 10;
const int max_throw = frames_in_game * 2 + 1;
const std::string valid_chars("0123456789SG");

// フレームのスコアを計算する
int get_frame_score(const frame_type& frame, const point_of_throw_type& point_of_throw)
{
  int   result = 0;
  frame_type::first_type index = frame.first;

  switch(frame.second) {
  case Normal:
    result = point_of_throw[index] + point_of_throw[index + 1];
    break;
  case Spare:
  case Strike:
  case Last:
    result = std::accumulate(&point_of_throw[index], &point_of_throw[index+3], 0);
    break;
  default:
    break;
  }

  return result;
}

// スコア表を描画する
void draw_score(const game_type& frame_data, const point_of_throw_type& point_of_throw)
{
  int last_index = frames_in_game - 1;

  // Draw upper line (throw result)
  for(int i = 0; i < last_index; ++i) {
    char score[2] = { '-', '-' };
	size_t index = frame_data[i].first;

    switch(frame_data[i].second) {
    case Normal:
      score[0] = '0' + point_of_throw[index];
      score[1] = '0' + point_of_throw[index + 1];
      break;
    case Spare:
      score[0] = '0' + point_of_throw[index];
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
  if(frame_data[last_index].second == Last) {
    char score[3] = { '-', '-', '-' }; 
	size_t index = frame_data[last_index].first;

	score[0] = (point_of_throw[index] == number_of_pin) ? 'x' : '0' + point_of_throw[index];
	score[1] = (point_of_throw[index] == number_of_pin)
	  ? (point_of_throw[index + 1] == number_of_pin) ? 'x' : '0' + point_of_throw[index + 1]
	  : ((point_of_throw[index] + point_of_throw[index + 1]) == number_of_pin) ? '/' : '0' + point_of_throw[index + 1];
    score[2] = (point_of_throw[index + 2] == number_of_pin) ? 'x' : '0' + point_of_throw[index + 2];
    std::printf("|%c|%c|%c|\n", score[0], score[1], score[2]);
  } else {
    std::printf("|-|-|-|\n");
  }

  // Draw bottom line (frame result)
  for(int i = 0; i < last_index; ++i) {
    if(frame_data[i].second == None) {
      std::printf("|   ");
    } else {
      int total = 0;
      for(int j = 0; j <= i; ++j) total += get_frame_score(frame_data[j], point_of_throw);
        std::printf("|%3d", total);
    }
  }
  // Last frame
  if(frame_data[last_index].second == Last) {
    int total = 0;
    for(int i = 0; i <= last_index; ++i) total += get_frame_score(frame_data[i], point_of_throw);
    printf("|%5d|\n", total);
  } else {
    printf("|     |\n");
  }
}


int main()
{
  using namespace std;

  point_of_throw_type   point_of_throw( max_throw, 0 );
  game_type             frame_data( frames_in_game, frame_type(18, None) );
  int                   frame_index = 0;
  size_t                throw_in_game = 0;
  bool                  next_frame = false;
  bool                  game_over = false;

  frame_data[frame_index] = make_pair( 0, Normal );
  do {
    if(next_frame) {
      ++frame_index;
	  frame_data[frame_index] = make_pair(throw_in_game, (frame_index == (frames_in_game - 1)) ? Last : Normal);
      next_frame = false;
    }

    auto& current_frame = frame_data[frame_index];
    auto throw_in_frame = throw_in_game - current_frame.first;
    // Input throw point
    char point;
    std::cin >> point;
    point = toupper(point); // Convert to upper case1
    point = (point == 'G') ? '0' : point;
    auto pos = valid_chars.find(point);
    // Check input character.
    if(pos == std::string::npos) {
      // Input character is incorrect.
      cout << "Invalid character: " << point << endl;
      continue;
    }

    // Store point of the throw. The number of remaining pin is point, if the throw is spare or strike.
    if(current_frame.second == Last) {
      // Last frame
      if(point == 'S') {
        switch(throw_in_frame) {
        case 2:
          game_over = true;
        case 0:
          point_of_throw[throw_in_game] = number_of_pin;
          break;
        case 1:
          point_of_throw[throw_in_game] = (point_of_throw[current_frame.first] != number_of_pin)
                                        ?  number_of_pin - point_of_throw[current_frame.first]
                                        : number_of_pin;
          break;
        default:
          exit(0);
        }
      } else {
        point_of_throw[throw_in_game] = pos;
        auto point_of_frame = point_of_throw[current_frame.first] + pos;
        if(((throw_in_frame == 1) && (point_of_frame < number_of_pin)) || (throw_in_frame == 2)) {
          game_over = true;
        }
      }
    } else {
      // Not last frame
      if(point == 'S') {
        // In the case of Spare or Strike, the number of remaining pin is score.
        point_of_throw[throw_in_game] = number_of_pin - point_of_throw[current_frame.first];
        current_frame.second = (throw_in_frame == 0) ? Strike : Spare;
        next_frame = true;
      } else {
        //
        point_of_throw[throw_in_game] = pos;
        if(throw_in_frame != 0) {
          next_frame = true;
        }
      }
    }

    // Draw score result until current throw.
    draw_score(frame_data, point_of_throw);
	// Go next throw
    ++throw_in_game;
  } while(!game_over);
}
