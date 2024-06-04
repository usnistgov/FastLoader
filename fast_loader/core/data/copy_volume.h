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

#ifndef FAST_LOADER_COPY_VOLUME_H
#define FAST_LOADER_COPY_VOLUME_H

#include <utility>
#include <vector>
#include <tuple>
#include <ostream>
#include <iterator>

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief Volume representation used to define copies
class CopyVolume {
  std::vector<size_t> const
      positionFrom_, ///< Source position
      positionTo_, ///< Destination position
      dimension_; ///< Copy dimension

  std::vector<bool> const reverseCopies_; ///< Flag used to determine if the copies should be made in reverse

 public:
  /// @brief Copy Volume constructor
  /// @param positionFrom Source position
  /// @param positionTo Destination position
  /// @param dimension Copy dimension
  /// @param reverseCopies Flag used to determine if the copies should be made in reverse
  CopyVolume(
      std::vector<std::size_t> positionFrom, std::vector<std::size_t> positionTo,
      std::vector<std::size_t> dimension, std::vector<bool> reverseCopies)
      : positionFrom_(std::move(positionFrom)),
        positionTo_(std::move(positionTo)),
        dimension_(std::move(dimension)),
        reverseCopies_(std::move(reverseCopies)) {}

  /// @brief Copy Volume constructor for ordered copies
  /// @param positionFrom Source position
  /// @param positionTo Destination position
  /// @param dimension Copy dimension
  CopyVolume(std::vector<std::size_t> positionFrom, std::vector<std::size_t> positionTo, std::vector<std::size_t> dimension)
      : positionFrom_(std::move(positionFrom)),
        positionTo_(std::move(positionTo)),
        dimension_(std::move(dimension)),
        reverseCopies_(std::vector<bool>(positionFrom_.size(), false)) {

  }

  /// @brief Default destructor
  virtual ~CopyVolume() = default;

  /// @brief Source position accessor
  /// @return Source position
  [[nodiscard]] std::vector<std::size_t> const &positionFrom() const { return positionFrom_; }
  /// @brief Destination position accessor
  /// @return Destination position
  [[nodiscard]] std::vector<std::size_t> const &positionTo() const { return positionTo_; }
  /// @brief Copy dimension accessor
  /// @return Copy dimension
  [[nodiscard]] std::vector<std::size_t> const &dimension() const { return dimension_; }
  /// @brief Ordering flag accessor
  /// @return Ordering flag
  [[nodiscard]] std::vector<bool> const &reverseCopies() const { return reverseCopies_; }

  /// @brief Equality operator
  /// @param rhs CopyVolume to compare against
  /// @return True is the CopyVolume are the same, else false
  bool operator==(CopyVolume const &rhs) const {
    return std::tie(positionFrom_, positionTo_, dimension_, reverseCopies_)
        == std::tie(rhs.positionFrom_, rhs.positionTo_, rhs.dimension_, rhs.reverseCopies_);
  }

  /// @brief Inequality operator
  /// @param rhs CopyVolume to compare against
  /// @return True is the CopyVolume are different, else false
  bool operator!=(CopyVolume const &rhs) const { return !(rhs == *this); }

  /// @brief Output stream operator
  /// @param os Output stream
  /// @param volume Volume to print
  /// @return Stream with the volume
  friend std::ostream &operator<<(std::ostream &os, CopyVolume const &volume) {
    os << "Copy from: [";
    std::copy(volume.positionFrom_.cbegin(), volume.positionFrom_.cend(), std::ostream_iterator<size_t>(os, ", "));
    os << "] to [";
    std::copy(volume.positionTo_.cbegin(), volume.positionTo_.cend(), std::ostream_iterator<size_t>(os, ", "));
    os << "] dimension [";
    std::copy(volume.dimension_.cbegin(), volume.dimension_.cend(), std::ostream_iterator<size_t>(os, ", "));
    os << "] reverse ? [";
    for (size_t i = 0; i < volume.reverseCopies_.size(); ++i) {
      os << std::boolalpha << volume.reverseCopies_.at(i) << (i != volume.reverseCopies_.size() - 1 ? ", " : "]");
    }
    return os;
  }

};

}
}

#endif //FAST_LOADER_COPY_VOLUME_H
