// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry a notice stating that you changed the software and should note the date and
// nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the
// source of the software. NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND,
// EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
// WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE
// CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS
// THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE. You
// are solely responsible for determining the appropriateness of using and distributing the software and you assume
// all risks associated with its use, including but not limited to the risks and costs of program errors, compliance
// with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of 
// operation. This software is not intended to be used in any situation where a failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.

#ifndef FAST_LOADER_SPIRAL_TRAVERSAL_H
#define FAST_LOADER_SPIRAL_TRAVERSAL_H

//#include <map>
//#include "../../api/abstract_traversal.h"
//
///// @brief FastLoader namespace
//namespace fl {
///// @brief FastLoader internal namespace
// TODO: Update for 3D
//namespace internal {
//template<class ViewType>
//class SpiralTraversal : public AbstractTraversal<ViewType> {
// private:
//  enum class Direction {
//    NORTH,
//    SOUTH,
//    EAST,
//    WEST
//  };
//
// public:
//  explicit SpiralTraversal(std::shared_ptr<AbstractTileLoader < ViewType>>
//  tl)
//  : AbstractTraversal<ViewType>("Spiral", tl) {}
//
//  virtual ~SpiralTraversal() = default;
//
//  [[nodiscard]] std::vector<std::pair<size_t, size_t>> traversal(size_t level) const override {
//    std::vector<std::pair<size_t, size_t>> traversal;
//
//    static std::map<Direction, Direction> nextDirection = {
//        {Direction::EAST, Direction::SOUTH},
//        {Direction::SOUTH, Direction::WEST},
//        {Direction::WEST, Direction::NORTH},
//        {Direction::NORTH, Direction::EAST}};
//
//    std::vector<std::vector<bool>>
//        hasNotBeenVisited(this->numberTileHeight(level), std::vector<bool>(this->numberTileWidth(level), true));
//
//    Direction
//        direction = Direction::EAST;
//
//    size_t
//        row = 0,
//        col = 0,
//        step = 0;
//
//    int32_t
//        rowNext = 0,
//        colNext = 0;
//
//    traversal.emplace_back((size_t) row, (size_t) col);
//    hasNotBeenVisited[row][col] = false;
//
//    while (step < this->numberTileHeight(level) * this->numberTileWidth(level) - 1) {
//      nextStep(direction, row, col, rowNext, colNext);
//      if (isFree(rowNext,
//                 colNext,
//                 this->numberTileHeight(level),
//                 this->numberTileWidth(level),
//                 hasNotBeenVisited)) {
//        ++step;
//        row = (size_t) rowNext;
//        col = (size_t) colNext;
//        hasNotBeenVisited[row][col] = false;
//        traversal.emplace_back(row, col);
//      } else {
//        direction = nextDirection[direction];
//      }
//    }
//    return traversal;
//  }
// private:
//  /// \brief Determine the next step for the spiral algorithm
//  /// \param direction Actual direction
//  /// \param row Actual row
//  /// \param col Actual column
//  /// \param rowNext Next row
//  /// \param colNext Next column
//  static void nextStep(Direction direction,
//                       size_t row,
//                       size_t col,
//                       int32_t &rowNext,
//                       int32_t &colNext) {
//    switch (direction) {
//      case Direction::NORTH:rowNext = row - 1;
//        colNext = col;
//        break;
//      case Direction::SOUTH:rowNext = row + 1;
//        colNext = col;
//        break;
//      case Direction::EAST:rowNext = row;
//        colNext = col + 1;
//        break;
//      case Direction::WEST:rowNext = row;
//        colNext = col - 1;
//        break;
//    }
//  }
//
//  /// \brief Test if the tile (row, col) is free
//  /// \param rowTest Tile row to test
//  /// \param colTest Tile column to test
//  /// \param numberTileHeight Number tiles in a row
//  /// \param numberTileWidth Number tiles in a column
//  /// \param hasNotBeenVisited matrix of tiles visited
//  /// \return True if tile free, else Fasle
//  static bool isFree(const int32_t &rowTest,
//                     const int32_t &colTest,
//                     const size_t &numberTileHeight,
//                     const size_t &numberTileWidth,
//                     const std::vector<std::vector<bool>> &hasNotBeenVisited) {
//    return
//        !(rowTest == -1 || (size_t) rowTest == numberTileHeight || colTest == -1 ||
//            (size_t) colTest == numberTileWidth) &&
//            hasNotBeenVisited[rowTest][colTest];
//  }
//};
//}
//}
#endif //FAST_LOADER_SPIRAL_TRAVERSAL_H
