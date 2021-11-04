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

#ifndef INC_FAST_LOADER_VOLUME_H
#define INC_FAST_LOADER_VOLUME_H

#include <cstdint>
#include <ostream>

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief Volume to copy
class Volume {
  size_t
      rowBegin_{}, ///< Volume's minimum row
      colBegin_{}, ///< Volume's minimum column
      layerBegin_{}, ///< Volume's minimum layer
      height_{}, ///< Volume's height
      width_{}, ///< Volume's width
      depth_{}; ///< Volume's depth

 public:
  /// @brief Volume constructor
  /// @param rowBegin Volume's minimum row
  /// @param colBegin Volume's minimum column
  /// @param layerBegin Volume's minimum layer
  /// @param height Volume's height
  /// @param width Volume's width
  /// @param depth Volume's depth
  Volume(size_t rowBegin, size_t colBegin, size_t layerBegin, size_t height, size_t width, size_t depth)
      : rowBegin_(rowBegin),
        colBegin_(colBegin),
        layerBegin_(layerBegin),
        height_(height),
        width_(width),
        depth_(depth) {}

  /// @brief Volume's minimum row accessor
  /// @return Volume's minimum row
  [[nodiscard]] size_t rowBegin() const { return rowBegin_; }
  /// @brief Volume's minimum column accessor
  /// @return Volume's minimum column
  [[nodiscard]] size_t colBegin() const {return colBegin_;}
  /// @brief Volume's minimum layer accessor
  /// @return Volume's minimum layer
  [[nodiscard]] size_t layerBegin() const {return layerBegin_;}

  /// @brief Volume's maximum row accessor
  /// @return Volume's maximum row
  [[nodiscard]] size_t rowEnd() const { return this->rowBegin_ + this->height_; }
  /// @brief Volume's maximum column accessor
  /// @return Volume's maximum column
  [[nodiscard]] size_t colEnd() const { return this->colBegin_ + this->width_; }
  /// @brief Volume's maximum layer accessor
  /// @return Volume's maximum layer
  [[nodiscard]] size_t layerEnd() const { return this->layerBegin_ + this->depth_; }

  /// @brief Volume's height accessor
  /// @return Volume's height
  [[nodiscard]] size_t height() const {return height_;}
  /// @brief Volume's width accessor
  /// @return Volume's width
  [[nodiscard]] size_t width() const {return width_;}
  /// @brief Volume's depth accessor
  /// @return Volume's depth
  [[nodiscard]] size_t depth() const {return depth_;}

  /// @brief Equality operator
  /// @param rhs Volume to compare
  /// @return True if rhs == *this, else False
  bool operator==(Volume const &rhs) const {
    return rowBegin_ == rhs.rowBegin_ &&
        colBegin_ == rhs.colBegin_ &&
        layerBegin_ == rhs.layerBegin_ &&
        height_ == rhs.height_ &&
        width_ == rhs.width_ &&
        depth_ == rhs.depth_;
  }

  /// @brief Inequality operator
  /// @param rhs Volume to compare
  /// @return True if rhs != *this, else False
  bool operator!=(Volume const &rhs) const {
    return !(rhs == *this);
  }

  /// @brief Merge operator
  /// @param rhs Volume to merge
  /// @return Minimal volume that contain both volume
  Volume &operator+=(Volume const & rhs){
    auto
      rowEndOriginal = this->rowEnd(),
      colEndOriginal = this->colEnd(),
      layerEndOriginal = this->layerEnd();

    this->rowBegin_ = this->rowBegin_ > rhs.rowBegin_ ? rhs.rowBegin_ : this->rowBegin_;
    this->colBegin_ = this->colBegin_ > rhs.colBegin_ ? rhs.colBegin_ : this->colBegin_;
    this->layerBegin_ = this->layerBegin_ > rhs.layerBegin_ ? rhs.layerBegin_ : this->layerBegin_;

    this->height_ = (rowEndOriginal > rhs.rowEnd() ? rowEndOriginal : rhs.rowEnd()) - this->rowBegin_;
    this->width_ = (colEndOriginal > rhs.colEnd() ? colEndOriginal : rhs.colEnd()) - this->colBegin_;
    this->depth_ = (layerEndOriginal > rhs.layerEnd() ? layerEndOriginal : rhs.layerEnd()) - this->layerBegin_;
    return *this;
  }
};
}
}
#endif //INC_FAST_LOADER_VOLUME_H
