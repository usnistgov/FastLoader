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

#ifndef FAST_LOADER_ABSTRACT_TRAVERSAL_H
#define FAST_LOADER_ABSTRACT_TRAVERSAL_H

#include <string>
#include <utility>
#include <vector>

/// @brief FastLoader namespace
namespace fl {

/// @brief Traversal abstraction
/// @details A traversal provides a list of index that will be used to traverse the file
class AbstractTraversal {
 private:
  std::string const name_; ///< Traversal's name

 public:
  /// @brief Traversal constructor
  /// @param name Traversal's name
  explicit AbstractTraversal(std::string name) : name_(std::move(name)) {}

  /// @brief Default destructor
  virtual ~AbstractTraversal() = default;

  /// @brief Name's accessor
  /// @return Traversal's name
  [[nodiscard ]] std::string const &name() const { return name_; }

  /// @brief Generate the vector of positions
  /// @param nbTilesPerDimension Dimensions of the file
  /// @return Vector of positions
  [[nodiscard]] virtual std::vector<std::vector<size_t>> traversal(std::vector<size_t> nbTilesPerDimension) const = 0;

};

} // fl

#endif //FAST_LOADER_ABSTRACT_TRAVERSAL_H
