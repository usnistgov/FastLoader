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
// Created by anb22 on 11/18/19.
//

#ifndef FASTLOADER_FAST_LOADER_CONFIGURATION_H
#define FASTLOADER_FAST_LOADER_CONFIGURATION_H

#include "../api/abstract_tile_loader.h"

#include "../internal/traits.h"

#include "../internal/border_creator/constant_border_creator.h"
#include "../internal/border_creator/reflect_border_creator.h"
#include "../internal/border_creator/reflect_101_border_creator.h"
#include "../internal/border_creator/replicate_border_creator.h"
#include "../internal/border_creator/wrap_border_creator.h"

#include "../internal/traversal/naive_traversal.h"
#include "../internal/traversal/snake_traversal.h"
#include "../internal/traversal/spiral_traversal.h"
#include "../internal/traversal/hilbert_traversal.h"
#include "../internal/traversal/diagonal_traversal.h"
#include "../internal/traversal/block_recursive_traversal.h"

#include "abstract_traversal.h"

/// @brief FastLoader namespace
namespace fl {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
/// @brief FastLoaderGraph Forward declaration
/// @tparam ViewType AbstractView Type
template<class ViewType>
class FastLoaderGraph;
#endif //DOXYGEN_SHOULD_SKIP_THIS

/// @brief FastLoader Configurations.
/// @details
/// Requires a TileLoader in order to be constructed, which will be used by the FastLoaderGraph to load tiles from a file.
/// Define the following configuration for the FastLoaderGraph:
/// - Define the view radii [default 0]: radius(uint32_t radius) for the same radius in every dimension or radius(uint32_t radiusHeight, uint32_t radiusWidth, uint32_t radiusDepth)
/// - Define the number of views available to be used at the same time [default 1]: viewAvailable(uint32_t level, uint32_t numberViewAvailable)
/// - Define the borderCreator to fill the view's ghost region [default ConstantBorderCreator]:
///   - borderCreator(FillingType fillingType) or,
///   - borderCreatorConstant(typename ViewType::data_t constantValue) or,
///   - borderCreatorCustom(std::shared_ptr<AbstractBorderCreator<ViewType>> borderCreator)
/// - Define the traversal used when all views are requested [default SNAKE]: traversalType(TraversalType traversalType)
/// - Define if the view are returned in the same order they have been requested [default false]: void ordered(bool ordered)
/// - Define the number of time a view should return into the graph before being discarded and be available to a new
/// request  [default 1]: releaseCount(uint32_t level, uint32_t release)
/// - Define the TileLoader Cache capacity [default 1]: cacheCapacity(uint32_t level, uint32_t capacity)
/// @attention To define ViewType, ViewType has to inherit publicly from AbstractView, the data inside of the view has to be
/// trivial.
/// @tparam ViewType AbstractView type
template<class ViewType>
class FastLoaderConfiguration {
  friend FastLoaderGraph<ViewType>; ///< Define FastLoaderGraph<ViewType> as friend
  friend AdaptiveFastLoaderGraph<ViewType>; ///< Define FastLoaderGraph<ViewType> as friend
  static_assert(std::is_default_constructible_v<ViewType>,
                "The given type should be default constructible.");
  static_assert(internal::traits::HasDataType<ViewType>::value,
                "The given type do not have the good properties, it should inherit from the class DefaultView or "
                "UnifiedView if available.");
  static_assert(std::is_arithmetic_v<typename ViewType::data_t>,
                "The type hold by the view should be an arithmetic type.");
  static_assert(internal::traits::is_view_v<ViewType>,
                "The given type should inherit from view (DefaultView or UnifiedView if available).");

  std::vector<uint32_t>
      nbReleasePyramid_,        ///< the number of time a view should return into the graph before being discarded and
  ///< be available to a new request
  cacheCapacity_,           ///< TileLoader Cache capacity
  viewAvailablePerLevel_;   ///< Number of views available to be used at the same time

  FillingType
      fillingType_; ///< Filling Type Used

  std::shared_ptr<AbstractBorderCreator<ViewType>>
      borderCreator_; ///< BorderCreator to fill the view's ghost region

  std::shared_ptr<AbstractTileLoader<ViewType>>
      tileLoader_; ///< TileLoader used by the FastLoaderGraph to load file's tile

  TraversalType
      traversalType_; ///< Traversal type used when all views are requested

  std::shared_ptr<AbstractTraversal>
      traversal_; ///< Traversal instance used when all views are requested

  bool
      ordered_; ///< Flag to define if the view are returned in the same order they have been requested

  uint32_t
      levels_, ///< File level (pyramidal, etc.)
  radiusHeight_, ///< AbstractView's height radius
  radiusWidth_, ///< AbstractView's width radius
  radiusDepth_; ///< AbstractView's depth radius

 public:
  /// @brief FastLoaderConfiguration constructor using a AbstractTileLoader
  /// @param tileLoader AbstractTileLoader that will be used by FastImageGraph
  explicit FastLoaderConfiguration(std::shared_ptr<AbstractTileLoader<ViewType>> const &tileLoader)
  : tileLoader_(tileLoader) {
    // Test the validity of the tileLoader
    validateTileLoader();
    nbReleasePyramid_ = std::vector<uint32_t>(tileLoader->numberPyramidLevels(), 1);
    cacheCapacity_ = std::vector<uint32_t>(tileLoader->numberPyramidLevels(), 1);
    viewAvailablePerLevel_ = std::vector<uint32_t>(tileLoader->numberPyramidLevels(), 1);
    fillingType_ = FillingType::CONSTANT;
    borderCreator_ =
        std::make_shared<internal::ConstantBorderCreator<ViewType>>(typename ViewType::data_t());
    traversalType_ = TraversalType::NAIVE;
    traversal_ = std::make_shared<internal::NaiveTraversal>();
    ordered_ = false;
    levels_ = tileLoader->numberPyramidLevels();
    radiusHeight_ = 0;
    radiusWidth_ = 0;
    radiusDepth_ = 0;
  }

  /// @brief Change the shared radius for every direction
  /// @param sharedRadius New shared radius for every direction
  void radius(uint32_t sharedRadius) { radius(sharedRadius, sharedRadius, sharedRadius); }

  /// @brief Change the radius for every directions
  /// @param radiusHeight New radius for the height
  /// @param radiusWidth New radius for the width
  /// @param radiusDepth New radius for the depth
  void radius(uint32_t radiusDepth, uint32_t radiusHeight, uint32_t radiusWidth) {
    radiusHeight_ = radiusHeight;
    radiusWidth_ = radiusWidth;
    radiusDepth_ = radiusDepth;
  }

  /// @brief Change the traversal type
  /// @param traversalType
  void traversalType(TraversalType traversalType) {
    std::ostringstream oss;
    switch (traversalType) {
      case TraversalType::NAIVE:traversal_ = std::make_shared<internal::NaiveTraversal>();
        break;
//        // TODO: Add additional traversals for 3D
////      case TraversalType::SNAKE:traversal_ = std::make_shared<internal::SnakeTraversal<ViewType>>(tileLoader_);
////        break;
////      case TraversalType::DIAGONAL:traversal_ = std::make_shared<internal::DiagonalTraversal<ViewType>>(tileLoader_);
////        break;
////      case TraversalType::HILBERT:traversal_ = std::make_shared<internal::HilbertTraversal<ViewType>>(tileLoader_);
////        break;
////      case TraversalType::SPIRAL:traversal_ = std::make_shared<internal::SpiralTraversal<ViewType>>(tileLoader_);
////        break;
////      case TraversalType::RECURSIVE_BLOCK:
////        traversal_ = std::make_shared<internal::BlockRecursiveTraversal<ViewType>>(tileLoader_);
////        break;
      case TraversalType::CUSTOM:
        oss
            << "This filling strategy need a custom implementation of AbstractTraversal, please call "
               "traversalCustom(std::shared_ptr<Traversal> traversal).";
        throw (std::runtime_error(oss.str()));
    }
    traversalType_ = traversalType;
  }

  /// @brief Setter to custom traversal
/// @tparam Traversal Traversal type to use
/// @param traversal Traversal to use
  template<class Traversal, class = std::enable_if<std::is_base_of_v<AbstractTraversal, Traversal>>>
  void traversalCustom(std::shared_ptr<Traversal> traversal) {
    traversalType_ = TraversalType::CUSTOM;
    traversal_ = std::static_pointer_cast<AbstractTraversal>(traversal);
  }

  /// @brief Define if the view are returned in the same order they have been requested
  /// @param ordered True if the view are returned in the same order they have been requested, else False
  void ordered(bool ordered) {
    ordered_ = ordered;
  }
  /// @brief Define the number of times a view should return into the graph before being discarded and be available to a
  /// new request
  /// @param level Property pyramid level
  /// @param release Number of time a view should return into the graph before being discarded and be available to a
  /// new request
  void releaseCount(uint32_t level, uint32_t release) {
    if (level > levels_) {
      std::ostringstream oss;
      oss
          << "The level asked (" << level << ") is greater than the levels available in the file (" << levels_
          << ") when setting the release mergeCount.";
      throw (std::runtime_error(oss.str()));
    }
    nbReleasePyramid_[level] = release;
  }
  /// @brief Define the number of views available to be used at the same time
  /// @param level Property pyramid level
  /// @param numberViewAvailable Number of views available to be used at the same time
  void viewAvailable(uint32_t level, uint32_t numberViewAvailable) {
    if (level > levels_) {
      std::ostringstream oss;
      oss
          << "The level asked (" << level << ") is greater than the levels available in the file (" << levels_
          << ") when setting the viewAvailable.";
      throw (std::runtime_error(oss.str()));
    }
    viewAvailablePerLevel_[level] = numberViewAvailable;
  }
  /// @brief Define the TileLoader Cache capacity
  /// @param level Property pyramid level
  /// @param capacity TileLoader Cache capacity
  void cacheCapacity(uint32_t level, uint32_t capacity) {
    if (level > levels_) {
      std::ostringstream oss;
      oss
          << "The level asked (" << level << ") is greater than the levels available in the file (" << levels_
          << ") when setting the cache viewAvailable.";
      throw (std::runtime_error(oss.str()));
    }
    cacheCapacity_[level] = capacity;
  }

  /// @brief Define the borderCreator to fill the view's ghost region, except for the constant or the custom
  /// @param fillingType Type of BorderCreator
  void borderCreator(FillingType fillingType) {
    std::ostringstream oss;
    switch (fillingType) {
      // TODO: Investigate other filling types
//      case FillingType::REFLECT:fillingType_ = FillingType::REFLECT;
//        borderCreator_ = std::make_shared<internal::ReflectBorderCreator<ViewType>>();
//        break;
//      case FillingType::REFLECT101:fillingType_ = FillingType::REFLECT101;
//        borderCreator_ = std::make_shared<internal::Reflect101BorderCreator<ViewType>>();
//        break;
      case FillingType::REPLICATE:fillingType_ = FillingType::REPLICATE;
        borderCreator_ = std::make_shared<internal::ReplicateBorderCreator<ViewType>>();
        break;
//      case FillingType::WRAP:fillingType_ = FillingType::WRAP;
//        borderCreator_ = std::make_shared<internal::WrapBorderCreator<ViewType>>();
//        break;
      case FillingType::CONSTANT:
        oss << "This filling strategy requires a value, please call borderCreatorConstant(typename "
               "ViewType::data_t).";
        throw (std::runtime_error(oss.str()));
      case FillingType::CUSTOM:
        oss
            << "This filling strategy need a custom implementation of AbstractBorderCreator, please call "
               "borderCreatorCustom(std::shared_ptr<AbstractBorderCreator<ViewType>>).";
        throw (std::runtime_error(oss.str()));
    }
  }
  /// @brief Define the constant borderCreator to fill the view's ghost region
  /// @param constantValue Value to fill the view
  void borderCreatorConstant(typename ViewType::data_t constantValue) {
    fillingType_ = FillingType::CONSTANT;
    borderCreator_ = std::make_shared<internal::ConstantBorderCreator<ViewType>>(constantValue);
  }
  /// @brief Define the custom borderCreator to fill the view's ghost region
  /// @param borderCreator Custom borderCreator to use
  void borderCreatorCustom(std::shared_ptr<AbstractBorderCreator<ViewType>> borderCreator) {
    fillingType_ = FillingType::CUSTOM;
    borderCreator_ = borderCreator;
  }

 private:
  /// @brief Validate the AbstractTileLoader
  /// @throw std::runtime if the AbstractTileLoader is not valid
  void validateTileLoader() {
    std::ostringstream oss;
    if (!tileLoader_) {
      oss << "The tile loader given to FastLoaderConfiguration is not valid (=nullptr).";
      throw (std::runtime_error(oss.str()));
    }
    if (!tileLoader_->numberPyramidLevels()) {
      oss << "The tile loader return a number of pyramid level equal to 0, planar files have 1 level." << std::endl;
      throw (std::runtime_error(oss.str()));
    }

    if (tileLoader_->numberChannels() == 0) {
      oss
          << "The tile loader return a number of pixel channel equal to 0, a pixel should have at least 1 channel."
          << std::endl;
      throw (std::runtime_error(oss.str()));
    }

    for (uint32_t level = 0; level < tileLoader_->numberPyramidLevels(); ++level) {
      if (tileLoader_->tileWidth(level) == 0
          || tileLoader_->tileHeight(level) == 0
          || tileLoader_->tileDepth(level) == 0) {
        oss << "The tile loader is not valid, for the level " << level << " the tile are of size ("
            << tileLoader_->tileWidth(level) << "/" << tileLoader_->tileHeight(level) << "/"
            << tileLoader_->tileDepth(level) << "), the tile can not have a dimension equal to 0";
        throw (std::runtime_error(oss.str()));
      }

      if (tileLoader_->tileWidth(level) > tileLoader_->fullWidth(level)
          || tileLoader_->tileHeight(level) > tileLoader_->fullHeight(level)
          || tileLoader_->tileDepth(level) > tileLoader_->fullDepth(level)
          ) {
        oss << "The tile loader is not valid, for the level "
            << level << " the tile are of size ("
            << tileLoader_->tileWidth(level) << "/"
            << tileLoader_->tileHeight(level) << "/"
            << tileLoader_->tileDepth(level) << "), the file is of size ("
            << tileLoader_->fullWidth(level) << "/"
            << tileLoader_->fullHeight(level) << "/"
            << tileLoader_->fullDepth(level) << ") the tiling can not be greater than the file size.";
        throw (std::runtime_error(oss.str()));
      }

      if (tileLoader_->fullWidth(level) == 0 || tileLoader_->fullHeight(level) == 0
          || tileLoader_->fullDepth(level) == 0) {
        oss
            << "The tile loader is not valid, for the level " << level << " the files is of size ("
            << tileLoader_->fullWidth(level) << "/" << tileLoader_->fullHeight(level) << "/"
            << tileLoader_->fullDepth(level) << "), the file can not have a dimension equal to 0";
        throw (std::runtime_error(oss.str()));
      }
    }
  }
};
}
#endif //FASTLOADER_FAST_LOADER_CONFIGURATION_H
