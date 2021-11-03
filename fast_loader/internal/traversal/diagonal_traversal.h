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
//
// Created by anb22 on 2/13/20.
//

#ifndef FASTLOADER_DIAGONAL_traversalH
#define FASTLOADER_DIAGONAL_traversalH
// TODO: Update for 3D
//#include "../../api/abstract_traversal.h"
///// @brief FastLoader namespace
//namespace fl {
//
///// @brief FastLoader internal namespace
//namespace internal {
//
//template<class ViewType>
//class DiagonalTraversal : public AbstractTraversal<ViewType> {
// public:
//
//  explicit DiagonalTraversal(std::shared_ptr<AbstractTileLoader < ViewType>>tl)
//  : AbstractTraversal<ViewType>("Diagonal", tl) {}
//
//  virtual ~DiagonalTraversal() = default;
//
//  [[nodiscard]] std::vector<std::pair<size_t, size_t>> traversal(size_t level) const override {
//    std::vector<std::pair<size_t, size_t>> traversal;
//    size_t
//        pos = 0;
//    int32_t
//        i = 0,
//        j = 0;
//    traversal.assign(this->numberTileHeight(level) * this->numberTileWidth(level),
//                     std::pair<size_t, size_t>(0, 0));
//
//    for (int32_t k = 0; k < (int32_t) this->numberTileHeight(level); k++) {
//      traversal[pos].first = (size_t) k;
//      traversal[pos].second = 0;
//      pos++;
//      i = k - 1;
//      j = 1;
//
//      while (isValid(i, j, this->numberTileHeight(level), this->numberTileWidth(level))) {
//        traversal[pos].first = (size_t) i;
//        traversal[pos].second = (size_t) j;
//        pos++;
//        i--;
//        j++;
//      }
//    }
//
//    for (int32_t k = 1; k < (int32_t) this->numberTileWidth(level); k++) {
//      traversal[pos].first = (size_t) this->numberTileHeight(level) - 1;
//      traversal[pos].second = (size_t) k;
//      pos++;
//      i = this->numberTileHeight(level) - 2;
//      j = k + 1;
//      while (isValid(i, j, this->numberTileHeight(level), this->numberTileWidth(level))) {
//        traversal[pos].first = (size_t) i;
//        traversal[pos].second = (size_t) j;
//        pos++;
//        i--;
//        j++;
//      }
//    }
//    return traversal;
//  }
//
// private:
//  /// \brief Determine if a position is valid
//  /// \param row row to test
//  /// \param col column to test
//  /// \param n Number of rows
//  /// \param m Number of columns
//  /// \return True if valid, else False
//  static bool isValid(int32_t const &row, int32_t const &col, size_t const &n, size_t const &m) {
//    return !(row < 0 || row >= (int32_t) n || col >= (int32_t) m || col < 0);
//  }
//};
//}
//}
#endif //FASTLOADER_DIAGONAL_traversalH
