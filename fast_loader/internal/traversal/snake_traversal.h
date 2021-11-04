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

#ifndef FAST_LOADER_SNAKE_TRAVERSAL_H
#define FAST_LOADER_SNAKE_TRAVERSAL_H

#include "../../api/abstract_traversal.h"

///// @brief FastLoader namespace
//namespace fl {
///// @brief FastLoader internal namespace
// TODO: Update for 3D
//namespace internal {
//template<class ViewType>
//class SnakeTraversal : public AbstractTraversal<ViewType> {
// public:
//  explicit SnakeTraversal(std::shared_ptr<AbstractTileLoader < ViewType>> tl)
//  : AbstractTraversal<ViewType>("Snake", tl) {}
//
//  virtual ~SnakeTraversal() = default;
//
// private:
//  [[nodiscard]] std::vector<std::pair<size_t, size_t>> traversal(size_t level) const override {
//    std::vector<std::pair<size_t, size_t>> traversal;
//    for (size_t row = 0; row < this->numberTileHeight(level); ++row) {
//      if (row % 2 == 0) {
//        for (size_t col = 0; col < this->numberTileWidth(level); ++col) { traversal.emplace_back(row, col); }
//      } else {
//        for (int32_t col = this->numberTileWidth(level) - 1; col > -1; --col) { traversal.emplace_back(row, col); }
//      }
//    }
//    return traversal;
//  }
//
//};
//}
//}
#endif //FAST_LOADER_SNAKE_TRAVERSAL_H
