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

#include "../../api/graph/options/abstract_traversal.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief Naive traversal traversing all dimensions in order
class NaiveTraversal : public AbstractTraversal {
 public:
  /// @brief Default constructor
  NaiveTraversal() : AbstractTraversal("Naive Traversal") {}
  /// @brief Default destructor
  ~NaiveTraversal() override = default;
  /// @brief Traversal getter
  /// @param nbTilesPerDimension Number of tiles per dimension
  /// @return Traversal {{i1, ..., j1}, {i1, ..., j2}, {i1, ..., j3} ....}
  [[nodiscard]] std::vector<std::vector<size_t>> traversal(std::vector<size_t> nbTilesPerDimension) const override {
    std::vector<std::vector<size_t>> traversal;
    generateTraversal(traversal, nbTilesPerDimension, {}, nbTilesPerDimension.size());
    return traversal;
  }

  /// @brief Create the traversal by traversing all dimensions in order
  /// @param traversal Returned traversal
  /// @param nbTilesPerDimension Number of tiles per dimension
  /// @param current Current position in the traversal
  /// @param nbDimensions Number of dimensions
  /// @param dimension Current dimension
  inline void generateTraversal(
      std::vector<std::vector<size_t>> &traversal,
      std::vector<size_t> const &nbTilesPerDimension,
      std::vector<size_t> current,
      size_t nbDimensions, size_t dimension = 0) const{
    if (dimension == nbDimensions - 1) {
      for (size_t pos = 0; pos < nbTilesPerDimension.at(dimension); ++pos) {
        current.push_back(pos);
        traversal.push_back(current);
        current.pop_back();
      }
    } else {
      for (size_t pos = 0; pos < nbTilesPerDimension.at(dimension); ++pos) {
        current.push_back(pos);
        generateTraversal(traversal, nbTilesPerDimension, current, nbDimensions, dimension + 1);
        current.pop_back();
      }
    }
  }
};

} // fl
} // internal

#endif //FAST_LOADER_NAIVE_TRAVERSAL_H
