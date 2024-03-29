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

#ifndef FAST_LOADER_NAIVE_TRAVERSAL_H
#define FAST_LOADER_NAIVE_TRAVERSAL_H

#include "../../api/abstract_traversal.h"
/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief Naive traversal pattern, traverse the file in increasing, layers, rows, columns
/// @tparam ViewType Type of view
class NaiveTraversal : public AbstractTraversal {
 public:
  /// @brief Default constructor
  NaiveTraversal() : AbstractTraversal("Naive Traversal"){};

  /// @brief Default destructor
  ~NaiveTraversal() override = default;

  /// @brief Traversal accessor
  /// @param numberTileHeight Number of tiles in height
  /// @param numberTileWidth Number of tiles in width
  /// @param numberTileDepth Number of tiles in depth
  /// @return Vector of (row, column, layer) representing the traversal
  [[nodiscard]] std::vector<std::array<size_t, 3>> traversal(
      size_t numberTileHeight, size_t numberTileWidth, size_t numberTileDepth) const final{
    std::vector<std::array<size_t, 3>> traversal {};
    traversal.reserve(numberTileDepth * numberTileHeight * numberTileWidth);
    for (size_t layer = 0; layer < numberTileDepth; ++layer) {
      for (size_t row = 0; row < numberTileHeight; ++row) {
        for (size_t col = 0; col < numberTileWidth; ++col) {
          traversal.push_back(std::array<size_t, 3>{row, col, layer});
        }
      }
    }
    return traversal;
  }

};
}
}
#endif //FAST_LOADER_NAIVE_TRAVERSAL_H
