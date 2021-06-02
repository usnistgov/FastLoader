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
//
// Created by anb22 on 11/8/19.
//

#ifndef FASTLOADER_ABSTRACT_BORDER_CREATOR_H
#define FASTLOADER_ABSTRACT_BORDER_CREATOR_H

#include <list>
#include <memory>
#include <cstdint>

#include "../internal/data/tile_request.h"

/// @brief FastLoader namespace
namespace fl {

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace internal {
template<class DataType>
class AbstractView;
}
#endif //DOXYGEN_SHOULD_SKIP_THIS

/// @brief Interface to create a BorderCreator
/// @details Used to fill the ghost region around the central tile in a view. The ghost region only concerns the
/// pixels that do not exist in the file. FastLoader create the ghost regions in two steps:
/// -# The first phase (tileRequestsToFillBorders) consists of generating new tile requests.
/// -# The second phase (fillBorderWithExistingValues) consists of duplicating existing value in the view.
/// Built-in BorderCreator:
/// - ConstantBorderCreator
/// - Reflect101BorderCreator
/// - ReflectBorderCreator
/// - ReplicateBorderCreator
/// - WrapBorderCreator
/// @tparam ViewType Type of the view
template<class ViewType>
class AbstractBorderCreator {
 protected:
  /// @brief Structure to embed index to register the copy
  struct CopyPosition {
    uint32_t
        indexTile, ///< Tile's index
        posBeginTile, ///< Tile's local position
        posBeginView, ///< AbstractView's local position
        size; ///< Copy's size
  };
 public:
  /// @brief Default destructor
  virtual ~AbstractBorderCreator() = default;

  /// @brief Generate new TileRequest to fill view's ghost region
  /// @details Fill the ghost region with tiles from the file by generating TileRequest
  /// @param view AbstractView to fill
  /// @return The list of TileRequest to create the ghost region
  [[nodiscard]] virtual std::list<std::shared_ptr<internal::TileRequest<ViewType>>>
  tileRequestsToFillBorders(std::shared_ptr<ViewType> const &view) = 0;

  /// @brief Duplicate elements of the view to fill the ghost region
  /// @param view AbstractView to fill
  virtual void fillBorderWithExistingValues(std::shared_ptr<ViewType> const &view) = 0;
};
}

#endif //FASTLOADER_ABSTRACT_BORDER_CREATOR_H
