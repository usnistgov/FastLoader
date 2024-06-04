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

#ifndef FAST_LOADER_COPY_PHYSICAL_TO_VIEW_H
#define FAST_LOADER_COPY_PHYSICAL_TO_VIEW_H

#include <hedgehog/hedgehog.h>
#include "../data/tile_request.h"
#include "../data/cached_tile.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief FastLoader internal namespace
namespace internal {

/// @brief Copy a physical tile from the cache to the view
/// @tparam ViewType Type of the view
template<class ViewType>
class CopyPhysicalToView : public hh::AbstractTask<
    1,
    std::pair<std::shared_ptr<internal::TileRequest<ViewType>>,
              std::shared_ptr<internal::CachedTile<typename ViewType::data_t>>>,
    internal::TileRequest<ViewType>> {

 public:
  /// @brief Default constructor for the copy task
  /// @param numberThreads Number of threads associated to the task
  explicit CopyPhysicalToView(size_t const numberThreads)
      : hh::AbstractTask<
      1,
      std::pair<std::shared_ptr<internal::TileRequest<ViewType>>,
                std::shared_ptr<internal::CachedTile<typename ViewType::data_t>>>,
      internal::TileRequest<ViewType>>("Copy Physical To View", numberThreads, false) {}

  /// @brief Default destructor
  ~CopyPhysicalToView() override = default;

  /// @brief Do the actual copy between the cached tile and the view. If the copy covers the entirety of the the cached tile and the view the copy is direct
  /// @param data Pair containing the cached tile and the view
  void execute(std::shared_ptr<std::pair<std::shared_ptr<internal::TileRequest<ViewType>>,
                                         std::shared_ptr<internal::CachedTile<typename ViewType::data_t>>>> data) override {
    auto tileRequestData = data->first;
    auto cachedTile = data->second;
    cachedTile->unlock();
    typename ViewType::data_t
        *const dataFrom = cachedTile->data()->data(),
        *const dataTo = tileRequestData->view()->viewOrigin();

    std::vector<size_t> const
        &dimensionFrom = cachedTile->dimension(),
        &dimensionTo = tileRequestData->view()->viewDims();

    size_t const
    fullDimFrom = std::accumulate(dimensionFrom.cbegin(), dimensionFrom.cend(), (size_t)
    1, std::multiplies<>()),
    fullDimTo = std::accumulate(dimensionTo.cbegin(), dimensionTo.cend(), (size_t)
    1, std::multiplies<>());

    for (internal::CopyVolume const &copy : tileRequestData->copies()) {
      if (fullDimFrom == fullDimTo &&
          fullDimFrom
              == std::accumulate(copy.dimension().cbegin(), copy.dimension().cend(), (size_t)
        1, std::multiplies<>())
      && std::all_of(copy.reverseCopies().cbegin(), copy.reverseCopies().cend(),
                     [](auto const &rev) { return rev == false; })) {
        std::copy_n(dataFrom, fullDimFrom, dataTo);
      } else {
        copyImpl(dataFrom, dataTo, dimensionFrom, dimensionTo, 0, 0, copy, dimensionFrom.size());
      }
    }
    this->addResult(tileRequestData);
  }

  /// @brief Hedgehog copy method
  /// @return New instance of the task doing the copy
  std::shared_ptr<hh::AbstractTask<
      1,
      std::pair<std::shared_ptr<internal::TileRequest<ViewType>>,
                std::shared_ptr<internal::CachedTile<typename ViewType::data_t>>>,
      internal::TileRequest<ViewType>>> copy() override {
    return std::make_shared<CopyPhysicalToView<ViewType>>(this->numberThreads());
  }

 private:
  /// @brief Implementation of the copy function
  /// @details Recursively traverse all dimensions to compute the copy position and make the actual copies
  /// @param from Pointer to the source buffer
  /// @param to Pointer to the destination buffer
  /// @param dimensionFrom Dimension of the source data
  /// @param dimensionTo Dimension of the destination data
  /// @param deltaFrom Copy delta applied to the source pointer calculation
  /// @param deltaTo Copy delta applied to the destination pointer calculation
  /// @param copy Copy description
  /// @param nbDimension Number of dimensions
  /// @param dimension Current visited dimension
  inline void copyImpl(
      typename ViewType::data_t *from, typename ViewType::data_t *to,
      std::vector<size_t> const &dimensionFrom, std::vector<size_t> const &dimensionTo,
      size_t deltaFrom, size_t deltaTo,
      internal::CopyVolume const &copy,
      size_t const nbDimension, size_t const dimension = 0) const {

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
        for (size_t posDimension = 0; posDimension < copy.dimension().at(dimension); ++posDimension) {
          copyImpl(
              from, to, dimensionFrom, dimensionTo,
              deltaFrom
                  + (copy.positionFrom().at(dimension) + copy.dimension().at(dimension) - posDimension)
                      * std::accumulate(dimensionFrom.cbegin() + (long) dimension + 1,
                                        dimensionFrom.cend(),
                                        (size_t)
          1,
              std::multiplies<>()),
          deltaTo
          +(copy.positionTo().at(dimension) + posDimension)
              * std::accumulate(dimensionTo.cbegin() + (long) dimension + 1, dimensionTo.cend(),
                                (size_t)
          1, std::multiplies<>()),
          copy, nbDimension, dimension + 1
          );
        }
      } else {
        for (size_t posDimension = 0; posDimension < copy.dimension().at(dimension); ++posDimension) {
          copyImpl(
              from, to, dimensionFrom, dimensionTo,
              deltaFrom
                  + (copy.positionFrom().at(dimension) + posDimension)
                      * std::accumulate(
                          dimensionFrom.cbegin() + (long) dimension + 1,
                          dimensionFrom.cend(),
                          (size_t)
          1,
              std::multiplies<>()),
          deltaTo
          +(copy.positionTo().at(dimension) + posDimension)
              * std::accumulate(
                  dimensionTo.cbegin() + (long) dimension + 1,
                  dimensionTo.cend(),
                  (size_t)
          1,
              std::multiplies<>()),
          copy, nbDimension, dimension + 1
          );
        }
      }
    }
  }
};

} // fl
} // core

#endif //FAST_LOADER_COPY_PHYSICAL_TO_VIEW_H
