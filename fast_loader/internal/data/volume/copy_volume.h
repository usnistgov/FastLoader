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

#ifndef INC_FAST_LOADER_COPY_VOLUME_H
#define INC_FAST_LOADER_COPY_VOLUME_H
#include "volume.h"
/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief Define a copy of volumes, used to copy part of or the entire tile loaded from the disk to the view. This
/// copy can also include symmetries / flips for each axis for the copies.
class CopyVolume {
  Volume
      from_, ///< Origin volume
      to_; ///< Destination volume

  bool
      reverseRows_ = false, ///< Row symmetry (default false)
      reverseCols_ = false, ///< Column symmetry (default false)
      reverseLayers_ = false; ///< Layer symmetry (default false)

 public:
  /// @brief Create a volume copy from the source volume to the destination volume
  /// @param from Origin volume
  /// @param to Destination volume
  /// @param reverseRows Row symmetry (default false)
  /// @param reverseCols Column symmetry (default false)
  /// @param reverseLayers Layer symmetry (default false)
  CopyVolume(
      Volume const &from, Volume const &to,
      bool reverseRows = false, bool reverseCols = false, bool reverseLayers = false)
      : from_(from), to_(to), reverseRows_(reverseRows), reverseCols_(reverseCols), reverseLayers_(reverseLayers) {}

  /// @brief Default destructor
  virtual ~CopyVolume() = default;

  /// @brief Origin volume accessor
  /// @return Origin volume
  Volume const &from() const { return from_; }
  /// @brief Destination volume accessor
  /// @return Destination volume
  Volume const &to() const { return to_; }

  /// @brief Row symmetry accessor
  /// @return Row symmetry
  bool reverseRows() const { return reverseRows_; }
  /// @brief Column symmetry accessor
  /// @return Column symmetry
  bool reverseCols() const { return reverseCols_; }
  /// @brief Layer symmetry accessor
  /// @return Layer symmetry
  bool reverseLayers() const { return reverseLayers_; }

  /// @brief Equality operator
  /// @param rhs CopyVolume to compare equality against
  /// @return True if the CopyVolume are the same, else False
  bool operator==(CopyVolume const &rhs) const {
    return from_ == rhs.from_ &&
        to_ == rhs.to_ &&
        reverseRows_ == rhs.reverseRows_ &&
        reverseCols_ == rhs.reverseCols_ &&
        reverseLayers_ == rhs.reverseLayers_;
  }

  /// @brief Not-equal-to operator
  /// @param rhs CopyVolume to compare inequality against
  /// @return True if the CopyVolume are the different, else False
  bool operator!=(CopyVolume const &rhs) const {
    return !(rhs == *this);
  }

  /// @brief Output stream operator
  /// @param os Output stream to print into
  /// @param volume CopyVolume to print
  /// @return Output stream
  friend std::ostream &operator<<(std::ostream &os, CopyVolume const &volume) {
    os << "Copy: from (r:"
       << volume.from_.rowBegin() << ", c:" << volume.from_.colBegin() << ", l:" << volume.from_.layerBegin()
       << ") -> to (r:" << volume.to_.rowBegin() << ", c:" << volume.to_.colBegin() << ", l:" << volume.to_.layerBegin()
       << ") / size (h:" << volume.to_.height() << ", w:" << volume.to_.width() << ", d:" << volume.to_.depth()
       << ") / Symmetry (r:"
       << std::boolalpha << volume.reverseRows_ << ", c:" << std::boolalpha << volume.reverseCols_ << ", l:"
       << std::boolalpha << volume.reverseLayers_ << ")";
    return os;
  }
};
}
}
#endif //INC_FAST_LOADER_COPY_VOLUME_H
