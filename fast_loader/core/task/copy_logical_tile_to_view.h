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

#ifndef FAST_LOADER_COPY_LOGICAL_TILE_TO_VIEW_H
#define FAST_LOADER_COPY_LOGICAL_TILE_TO_VIEW_H

#include <hedgehog/hedgehog.h>
#include "../data/adaptive_tile_request.h"
#include "../data/cached_tile.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief FastLoader internal namespace
namespace internal {

/// @brief Multi-threaded task to copy [parts of] logical caches to the view
/// @tparam ViewType Type of the view
template<class ViewType>
class CopyLogicalTileToView :
    public hh::AbstractTask<1, fl::internal::AdaptiveTileRequest<ViewType>, fl::internal::TileRequest<ViewType>> {
  using DataType = typename ViewType::data_t; ///< Type of data inside a View
 public:
  /// @brief CopyLogicalCacheToView constructor
  /// @param nbThreads Number of thread associated to the task
  explicit CopyLogicalTileToView(size_t const nbThreads)
      : hh::AbstractTask<1, fl::internal::AdaptiveTileRequest<ViewType>, fl::internal::TileRequest<ViewType>>(
      "CopyLogicalTileToView", nbThreads) {}

  /// @brief Default destructor
  virtual ~CopyLogicalTileToView() = default;

  /// @brief Acquire a logical cached tile and copy [part of] it to the view
  /// @param adaptiveTileRequest AdaptiveTileRequest containing all information to achieve the copy
  void execute(std::shared_ptr<fl::internal::AdaptiveTileRequest<ViewType>> adaptiveTileRequest) override {
    std::shared_ptr<fl::internal::TileRequest<ViewType>> logicalTileRequest = adaptiveTileRequest->logicalTileRequest();
    std::shared_ptr<fl::internal::CachedTile<typename ViewType::data_t>> logicalCachedTile =
        adaptiveTileRequest->logicalCachedTile();

    size_t const
        fullDimFrom = std::accumulate(logicalCachedTile->dimension().cbegin(),
                                      logicalCachedTile->dimension().cend(),
                                      (size_t) 1,
                                      std::multiplies<>()),
        fullDimTo = std::accumulate(logicalTileRequest->view()->viewDims().cbegin(),
                                    logicalTileRequest->view()->viewDims().cend(),
                                    (size_t) 1,
                                    std::multiplies<>());

    for (internal::CopyVolume const &copy : logicalTileRequest->copies()) {
      if (fullDimFrom == fullDimTo &&
          fullDimFrom
              == std::accumulate(copy.dimension().cbegin(), copy.dimension().cend(), (size_t) 1, std::multiplies<>())
          && std::all_of(copy.reverseCopies().cbegin(),
                         copy.reverseCopies().cend(),
                         [](auto const &rev) { return rev == false; })) {
        std::copy_n(logicalCachedTile->data()->data(), fullDimFrom, logicalTileRequest->view()->viewOrigin());
      } else {
        copyImpl(logicalCachedTile->data()->data(), logicalTileRequest->view()->viewOrigin(),
                 logicalCachedTile->dimension(), logicalTileRequest->view()->viewDims(),
                 0, 0, copy, logicalCachedTile->dimension().size());
      }
    }

    logicalCachedTile->unlock();
    this->addResult(logicalTileRequest);
  }

  /// @brief Copy method to duplicate the Hedgehog tasks
  /// @return New Hedgehog task instance
  std::shared_ptr<hh::AbstractTask<1,
                                   fl::internal::AdaptiveTileRequest<ViewType>,
                                   fl::internal::TileRequest<ViewType>>> copy() override {
    return std::make_shared<CopyLogicalTileToView<ViewType>>(this->numberThreads());
  }

 private:
  /// @brief Copy implementation of the tile to the view
  /// @param from Pointer to source buffer
  /// @param to Pointer to destination buffer
  /// @param dimensionFrom Dimension of source buffer
  /// @param dimensionTo Dimension of destination buffer
  /// @param deltaFrom Copy delta source
  /// @param deltaTo Copy delta destination
  /// @param copy Current copy made
  /// @param nbDimension Total number of dimensions
  /// @param dimension Current dimension
  constexpr inline void copyImpl(
      DataType *from, DataType *to,
      std::vector<size_t> const &dimensionFrom, std::vector<size_t> const &dimensionTo,
      size_t deltaFrom, size_t deltaTo,
      internal::CopyVolume const &copy, size_t const nbDimension, size_t const dimension = 0) const {
    if (dimension == nbDimension - 1) {
      // If copy is reversed for the most inner dimension use revers_copy instead of copy
      if (copy.reverseCopies().at(dimension)) {
        std::reverse_copy(
            from + deltaFrom + copy.positionFrom().at(dimension),
            from + deltaFrom + copy.positionFrom().at(dimension) + copy.dimension().at(dimension),
            to + deltaTo + copy.positionTo().at(dimension)
        );
      } else {
        std::copy_n(
            from + deltaFrom + copy.positionFrom().at(dimension),
            copy.dimension().at(dimension),
            to + deltaTo + copy.positionTo().at(dimension)
        );
      }
    } else {
      // If copy is reversed for the outer dimension[s] use the iteration for source go backward
      if (copy.reverseCopies().at(dimension)) {
        for (size_t iteration = 0; iteration < copy.dimension().at(dimension); ++iteration) {
          copyImpl(
              from, to, dimensionFrom, dimensionTo,
              deltaFrom
                  + (copy.positionFrom().at(dimension) + copy.dimension().at(dimension) - iteration)
                      * std::accumulate(dimensionFrom.cbegin() + (long) dimension + 1,
                                        dimensionFrom.cend(),
                                        (size_t) 1,
                                        std::multiplies<>()),
              deltaTo
                  + (copy.positionTo().at(dimension) + iteration)
                      * std::accumulate(dimensionTo.cbegin() + (long) dimension + 1,
                                        dimensionTo.cend(),
                                        (size_t) 1,
                                        std::multiplies<>()),

              copy, nbDimension, dimension + 1
          );
        }
      } else {
        for (size_t iteration = 0; iteration < copy.dimension().at(dimension); ++iteration) {
          copyImpl(
              from, to, dimensionFrom, dimensionTo,
              deltaFrom
                  + (copy.positionFrom().at(dimension) + iteration)
                      * std::accumulate(dimensionFrom.cbegin() + (long) dimension + 1,
                                        dimensionFrom.cend(),
                                        (size_t) 1,
                                        std::multiplies<>()),
              deltaTo
                  + (copy.positionTo().at(dimension) + iteration)
                      * std::accumulate(dimensionTo.cbegin() + (long) dimension + 1,
                                        dimensionTo.cend(),
                                        (size_t) 1,
                                        std::multiplies<>()),
              copy, nbDimension, dimension + 1
          );
        }
      }
    }
  }

};

} // fl
} // internal

#endif //FAST_LOADER_COPY_LOGICAL_TILE_TO_VIEW_H
