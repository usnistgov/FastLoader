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

#ifndef FAST_LOADER_TILE_REQUEST_H
#define FAST_LOADER_TILE_REQUEST_H

#include <utility>
#include <vector>
#include <memory>
#include <list>
#include <algorithm>
#include <ostream>

#include "copy_volume.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief The Tile Request Data represent a specific tile request which will fill the future view. It has been
/// generated by The ViewLoader and sent to the AbstractTileLoader. The AbstractTileLoader will use it to know which
/// tile to request to the cache / file and then copy part or the whole tile into the view.
/// @tparam ViewType Type of the view
template<class ViewType>
class TileRequest {
  std::vector<size_t> const index_; ///< Tile index
  std::shared_ptr<ViewType> const view_{}; ///< AbstractView to copy into
  std::list<CopyVolume> copies_{}; ///< List of copies to make from the request to the view

 public:
  /// @brief TileRequest constructor
  /// @param index Tile index to build the view
  /// @param view View to fill
  TileRequest(std::vector<size_t> const & index, std::shared_ptr<ViewType> const &view)
      : index_(index), view_(view) {}

  /// @brief TileRequest destructor
  virtual ~TileRequest() = default;

  /// @brief Accessor to the index tiles used to fill the view
  /// @return Index tiles used to fill the view
  [[nodiscard]] std::vector<size_t> const &index() const { return index_; }
  /// @brief View accessor
  /// @return View to fill
  [[nodiscard]] std::shared_ptr<ViewType> const &view() const { return view_; }
  /// @brief Copies accessor
  /// @return Copies to do to fill the view
  [[nodiscard]] std::list<CopyVolume> const &copies() const { return copies_; }

  /// @brief Add a copy to the list aof copies to make
  /// @param copy Copy to add
  void addCopy(CopyVolume const &copy) { copies_.push_back(copy); }

  /// @brief Merge TileRequest
  /// @param rhs Tile request to merge into this instance
  /// @return This instance in which has been merged rhs
  TileRequest const &merge(TileRequest const &rhs) {
    for (auto const &copy : rhs.copies()) {
      if (std::none_of(copies_.cbegin(), copies_.cend(), [&copy](auto const &elem) { return elem == copy; })) {
        copies_.push_back(copy);
      }
    }
    return *this;
  }

  /// @brief Equality operator
  /// @param rhs TileRequest to compare against
  /// @return True if rhs and this are equal, else false
  bool operator==(TileRequest const &rhs) const { return std::tie(index_, view_) == std::tie(rhs.index_, rhs.view_); }

  /// @brief Inequality operator
  /// @param rhs TileRequest to compare against
  /// @return True if rhs and this are different, else false
  bool operator!=(TileRequest const &rhs) const { return rhs != *this; }

  /// @brief Output stream iterator
  /// @param os Output stream
  /// @param request Request to add to the stream
  /// @return Stream with the TileRequest data inside
  friend std::ostream &operator<<(std::ostream &os, TileRequest const &request) {
    os << "TileRequest [";
    std::copy(request.index_.cbegin(), request.index_.cend(), std::ostream_iterator<size_t>(os, ", "));
    os << "]\n" << *request.view_ << "\n";
    for (auto const &copy : request.copies_) { os << "\t" << copy << "\n"; }
    return os;
  }

};

} // fl
} // internal

#endif //FAST_LOADER_TILE_REQUEST_H
