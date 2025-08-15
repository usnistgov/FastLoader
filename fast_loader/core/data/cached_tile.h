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

#ifndef FAST_LOADER_CACHED_TILE_H
#define FAST_LOADER_CACHED_TILE_H

#include <memory>
#include <utility>
#include <vector>
#include <numeric>
#include <sstream>
#include <iterator>
#include <ostream>
#include <semaphore>

/// @brief FastLoader namespace
namespace fl {

/// @brief FastLoader internal namespace
namespace internal {

/// @brief
/// @tparam DataType Type of data inside the View
template<class DataType>
class CachedTile {
 protected:
  std::shared_ptr<std::vector<DataType>> data_{}; ///< Tile data.
  std::vector<size_t> index_{}; ///< Tile index
  std::vector<size_t> const dimension_{}; ///< Tile dimensions
  bool newTile_{}; ///< Flax for new tile
  std::mutex accessMutex_{}; ///< Mutex for accessing the tile
  std::binary_semaphore semaphore_{1}; ///< Semaphore for cache safety

 public:
  /// @brief Cached tile constructor
  /// @param dimension  Dimension of the cached tile
  explicit CachedTile(std::vector<size_t> dimension) : dimension_(std::move(dimension)), newTile_(true) {
    try {
      data_ = std::make_shared<std::vector<DataType>>(
          std::accumulate(dimension_.begin(), dimension_.end(), (size_t) 1, std::multiplies<>())
      );
    } catch (std::bad_alloc const &except) {
      std::ostringstream oss;
      oss << "Problem while allocating a cached tile with the dimension (";
      std::copy(dimension_.cbegin(), dimension_.cend(), std::ostream_iterator<size_t>(oss, ", "));
      oss << ") :" << except.what();
      throw std::runtime_error(oss.str());
    }
  }

  /// @brief Default destructor
  virtual ~CachedTile() = default;

  /// @brief Data accessor
  /// @return Data
  std::shared_ptr<std::vector<DataType>> const &data() const { return data_; }
  /// @brief Cached tile index accessor
  /// @return Cached tile index
  [[nodiscard]] std::vector<size_t> const &index() const { return index_; }
  /// @brief Cached tile dimension accessor
  /// @return Cached tile dimension
  [[nodiscard]] std::vector<size_t> const &dimension() const { return dimension_; }
  /// @brief New tile flag accessor
  /// @return New tile flag
  [[nodiscard]] bool newTile() const { return newTile_; }

  /// @brief Cached tile index setter
  /// @param index Cache tile index to set
  void index(std::vector<size_t> const &index) { index_ = index; }
  /// @brief New tile flag setter
  /// @param newTile New tile flag to set
  void newTile(bool newTile) { newTile_ = newTile; }

  /// @brief Lock inner mutex
  void lock() { accessMutex_.lock(); }

  /// @brief Unlock inner mutex
  void unlock() { accessMutex_.unlock(); }

  void acquireSemaphore() {
    semaphore_.acquire();
  }

  void releaseSemaphore() {
    semaphore_.release();
  } 

  /// @brief Output stream operator for the cached tile
  /// @param os Output stream
  /// @param tile Tile to print
  /// @return Output stream containing the tile
  friend std::ostream &operator<<(std::ostream &os, CachedTile const &tile) {
    os << "CachedTile [";
    std::copy(tile.index_.cbegin(), tile.index_.cend(), std::ostream_iterator<size_t>(os, ", "));
    os << "] of dimension: (";
    std::copy(tile.dimension_.cbegin(), tile.dimension_.cend(), std::ostream_iterator<size_t>(os, ", "));
    os << "), is new ? " << std::boolalpha << tile.newTile() << "\n data: [";
    std::copy(tile.data_->cbegin(),  tile.data_->cend(), std::ostream_iterator<DataType>(os, ", "));
    os << "]";
    return os;
  }

};

} // fl
} // internal

#endif //FAST_LOADER_CACHED_TILE_H
