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

#ifndef FAST_LOADER_FAST_LOADER_CONFIGURATION_H
#define FAST_LOADER_FAST_LOADER_CONFIGURATION_H
#include <vector>
#include <memory>

#include "../../tools/traits.h"

#include "../data/data_type.h"
#include "options/abstract_traversal.h"
#include "abstract_tile_loader.h"
#include "options/abstract_border_creator.h"
#include "../../core/border_creator/constant_border_creator.h"
#include "../../core/border_creator/default_border_creator.h"
#include "../../core/traversal/naive_traversal.h"

/// @brief FastLoader namespace
namespace fl {

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/// @brief FastLoaderGraph Forward declaration
/// @tparam ViewType Type of the view
template<class ViewType>
class FastLoaderGraph;
#endif //DOXYGEN_SHOULD_SKIP_THIS

/// @brief Fast Loader configuration
/// @details Allow to configure a FastLoader Graph through various options:
/// - Define the radius
///   - By calling radius(size_t sharedRadius) that define a common radius value among the dimensions
///   - By calling radii(std::vector<size_t> const &radii) that set different radius value for the dimensions
/// - Define the cache capacity attached to the tie loader (cacheCapacityMB(vector<size_t> const &))
/// - Define if the views need to be given in the same order they have been requested or as soon as possible (ordered(bool))
/// - Define the release count for the views (number of time a view need to be returned before being clean for reuse) (releaseCountPerLevel(std::vector<size_t> const &))
/// - Define the number of views being constructed in parallel (viewAvailable(vector<size_t> const &))
/// - Define the traversal used if all views are requested (traversalType(TraversalType) / traversalCustom(shared_ptr<TraversalType>))
/// - Define the borderCreator used to fill the view with data not defined by the file (borderCreator(FillingType) / borderCreatorConstant(data_t) / borderCreatorCustom(shared_ptr<AbstractBorderCreator<ViewType>>))
/// @tparam ViewType Type of the view
template<class ViewType>
class FastLoaderConfiguration {
  friend FastLoaderGraph<ViewType>; ///< Define FastLoaderGraph<ViewType> as friend
  friend AdaptiveFastLoaderGraph<ViewType>; ///< Define AdaptiveFastLoaderGraph<ViewType> as friend
  static_assert(std::is_default_constructible_v<ViewType>,
                "The given type should be default constructible.");
  static_assert(internal::traits::HasDataType<ViewType>::value,
                "The given type do not have the good properties, it should inherit from the class DefaultView or "
                "UnifiedView if available.");
  static_assert(std::is_arithmetic_v<typename ViewType::data_t>,
                "The type hold by the view should be an arithmetic type.");
  static_assert(internal::traits::is_view_v<ViewType>,
                "The given type should inherit from view (DefaultView or UnifiedView if available).");

  std::vector<size_t>
      nbReleasePyramid_,        ///< the number of time a view should return into the graph before being discarded and
  ///< be available to a new request
  cacheCapacityMB_,         ///< TileLoader Cache capacity in MB
  viewAvailablePerLevel_,   ///< Number of views available to be used at the same time
  radii_;                   ///< Radii used to build the view

  bool
      ordered_; ///< Define if the views are returned in the same order they have been requested

  FillingType fillingType_; ///< Filling Type Used

  std::shared_ptr<AbstractBorderCreator<ViewType>> borderCreator_; ///< BorderCreator to fill the view's ghost region

  std::shared_ptr<AbstractTileLoader<ViewType>> const
      tileLoader_; ///< TileLoader used by the FastLoaderGraph to load file's tile

  TraversalType traversalType_; ///< Traversal type used when all views are requested

  std::shared_ptr<AbstractTraversal> traversal_; ///< Traversal instance used when all views are requested

  size_t
      nbLevels_, ///< File pyramidal level
  nbDimensions_, ///< Number of dimensions
  nbThreadsCopyPhysicalCacheView_; ///< Number of threads associated with the copy from the physical cache to view task

 public:
  /// @brief Default constructor using a tile loader
  /// @param tileLoader Tile loader used to get metadata and data from the file
  explicit FastLoaderConfiguration(std::shared_ptr<AbstractTileLoader<ViewType>> const &tileLoader)
      : tileLoader_(tileLoader) {
    // Test the validity of the tileLoader
    std::ostringstream oss;
    if (!tileLoader_) {
      oss << "The tile loader given to FastLoaderConfiguration is not valid (=nullptr).";
      throw (std::runtime_error(oss.str()));
    } else {
      nbDimensions_ = tileLoader_->nbDims();
      if (nbDimensions_ == 0) {
        oss << "The tile loader return a number of dimensions equal to 0, the data should have at least 1 dimension."
            << std::endl;
        throw (std::runtime_error(oss.str()));
      }
      if (tileLoader_->nbPyramidLevels() == 0) {
        oss << "The tile loader return a number of pyramid level equal to 0, planar files have 1 level." << std::endl;
        throw (std::runtime_error(oss.str()));
      }

      if (tileLoader_->dimNames().size() != nbDimensions_) {
        oss << "The dimension names [";
        std::copy(tileLoader_->dimNames().cbegin(),
                  tileLoader_->dimNames().cend(),
                  std::ostream_iterator<std::string>(oss, ", "));
        oss << "] is not valid (not the right number of names).";
        throw std::runtime_error(oss.str());
      }

      for (size_t level = 0; level < tileLoader_->nbPyramidLevels(); ++level) {
        std::vector<size_t>
            fullDimension = tileLoader_->fullDims(level),
            tileDimension = tileLoader_->tileDims(level);

        if (fullDimension.size() != nbDimensions_) {
          oss << "The full dimension for the " << level << "level [";
          std::copy(fullDimension.cbegin(), fullDimension.cend(), std::ostream_iterator<size_t>(oss, ", "));
          oss << "] is not valid (not the right number of dimension).";
          throw std::runtime_error(oss.str());
        }

        if (tileDimension.size() != nbDimensions_) {
          oss << "The tile dimension for the " << level << "level [";
          std::copy(tileDimension.cbegin(), tileDimension.cend(), std::ostream_iterator<size_t>(oss, ", "));
          oss << "] is not valid (not the right number of dimension).";
          throw std::runtime_error(oss.str());
        }

        if (std::any_of(fullDimension.cbegin(),
                        fullDimension.cend(),
                        [](auto const &dimension) { return dimension == 0; })) {
          oss << "The full dimension for the " << level << "level [";
          std::copy(fullDimension.cbegin(), fullDimension.cend(), std::ostream_iterator<size_t>(oss, ", "));
          oss << "] is not valid (dimension == 0).";
          throw std::runtime_error(oss.str());
        }
        if (std::any_of(tileDimension.cbegin(),
                        tileDimension.cend(),
                        [](auto const &dimension) { return dimension == 0; })) {
          oss << "The tile dimension for the " << level << "level [";
          std::copy(tileDimension.cbegin(), tileDimension.cend(), std::ostream_iterator<size_t>(oss, ", "));
          oss << "] is not valid.";
          throw std::runtime_error(oss.str());
        }
        if (fullDimension < tileDimension) {
          oss << "One of the full dimension dimensions is lower than the tile dimension for the level " << level << ".";
          throw std::runtime_error(oss.str());
        }
      }
    } // tileLoader != nullptr

    nbReleasePyramid_ = std::vector<size_t>(tileLoader->nbPyramidLevels(), 1);
    cacheCapacityMB_ = std::vector<size_t>(tileLoader->nbPyramidLevels(), 10);
    viewAvailablePerLevel_ = std::vector<size_t>(tileLoader->nbPyramidLevels(), 1);
    fillingType_ = FillingType::DEFAULT;
    borderCreator_ =
        std::make_shared<internal::ConstantBorderCreator<ViewType>>(typename ViewType::data_t());
    traversalType_ = TraversalType::NAIVE;
    traversal_ = std::make_shared<internal::NaiveTraversal>();
    ordered_ = false;
    nbLevels_ = tileLoader->nbPyramidLevels();
    radii_ = std::vector<size_t>(nbDimensions_);
    nbThreadsCopyPhysicalCacheView_ = 2;
  }

  /// @brief TileLoader's cache capacity in MB accessor
  /// @return TileLoader's cache capacity in MB
  [[nodiscard]] std::vector<size_t> const &cacheCapacityMB() const { return cacheCapacityMB_; }

  /// @brief Accessor to number of threads associated to the task that copy a physical tile to the view
  /// @return Number of threads associated to the task that copy a physical tile to the view
  [[nodiscard]] size_t nbThreadsCopyPhysicalCacheView() const { return nbThreadsCopyPhysicalCacheView_; }

  /// @brief Set the same radius value for all dimensions
  /// @param sharedRadius Radius value to set for all dimensions
  void radius(size_t sharedRadius) { radii_ = std::vector<size_t>(nbDimensions_, sharedRadius); }

  /// @brief Set radii values
  /// @param radii Radii values to set
  void radii(std::vector<size_t> const &radii) {
    if (radii.size() != nbDimensions_) { throw std::runtime_error("The radii set is not of the right dimension."); }
    radii_ = radii;
  }

  /// @brief Set if the views needs to be given in the same order they have been requested, or as soon as possible
  /// @param ordered True if the views are ordered in the same way they are requested, else False
  void ordered(bool ordered) { ordered_ = ordered; }

  /// @brief Define the number of times a view should return into the graph before being discarded and be available to a
  /// new request
  /// @param releaseCountPerLevel Number of time a view should return into the graph before being discarded and be
  /// available to a new request for each pyramidal level
  void releaseCountPerLevel(std::vector<size_t> const &releaseCountPerLevel) {
    if (releaseCountPerLevel.size() != nbLevels_) {
      throw std::runtime_error("The release count is not set for every level.");
    }
    if (std::any_of(releaseCountPerLevel.cbegin(),
                    releaseCountPerLevel.cend(),
                    [](auto const &val) { return val == 0; })) {
      throw std::runtime_error("The release count should not be equal to zero.");
    }
    nbReleasePyramid_ = releaseCountPerLevel;
  }

  /// @brief Define the number of views available to be used at the same time
  /// @param nbViewAvailablePerLevel Number of views available to be used at the same time per level
  void viewAvailable(std::vector<size_t> const &nbViewAvailablePerLevel) {
    if (nbViewAvailablePerLevel.size() != nbLevels_) {
      throw std::runtime_error("The number of views available per level is not set for every level.");
    }
    if (std::any_of(nbViewAvailablePerLevel.cbegin(),
                    nbViewAvailablePerLevel.cend(),
                    [](auto const &val) { return val == 0; })) {
      throw std::runtime_error("The number of views available per level should not be equal to zero.");
    }
    viewAvailablePerLevel_ = nbViewAvailablePerLevel;
  }

  /// @brief Define the TileLoader Cache capacity
  /// @param cacheCapacityMBPerLevel TileLoader cache capacity in MB per level to set
  void cacheCapacityMB(std::vector<size_t> const &cacheCapacityMBPerLevel) {
    if (cacheCapacityMBPerLevel.size() != nbLevels_) {
      throw std::runtime_error("The cache capacity per level is not set for every level.");
    }
    if (std::any_of(cacheCapacityMBPerLevel.cbegin(),
                    cacheCapacityMBPerLevel.cend(),
                    [](auto const &val) { return val == 0; })) {
      throw std::runtime_error("The cache capacity per level should not be equal to zero.");
    }
    cacheCapacityMB_ = cacheCapacityMBPerLevel;
  }

  /// @brief Set the chosen Traversal amongst the ones available
  /// @param traversalType Traversal to set
  void traversalType(TraversalType traversalType) {
    traversalType_ = traversalType;
    switch (traversalType_) {
      case TraversalType::NAIVE: traversal_ = std::make_shared<internal::NaiveTraversal>();
        break;
      case TraversalType::CUSTOM:std::ostringstream oss;
        oss << "This filling strategy need a custom implementation of AbstractTraversal, please call "
               "traversalCustom(std::shared_ptr<Traversal> traversal).";
        throw (std::runtime_error(oss.str()));
    }
  }

  /// @brief Set a custom traversal
  /// @tparam Traversal Type of traversal
  /// @param traversal Traversal to set
  template<class Traversal>
  requires std::is_base_of_v<AbstractTraversal, Traversal>
  void traversalCustom(std::shared_ptr<Traversal> traversal) {
    traversalType_ = TraversalType::CUSTOM;
    traversal_ = std::static_pointer_cast<AbstractTraversal>(traversal);
  }

  /// @brief Define the borderCreator to fill the view's ghost region, except for the constant or the custom
  /// @param fillingType Type of BorderCreator
  void borderCreator(FillingType fillingType) {
    std::ostringstream oss;
    switch (fillingType) {
      case FillingType::DEFAULT:borderCreator_ = std::make_shared<internal::DefaultBorderCreator<ViewType>>();
        break;
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

  /// @brief Define the number of threads attached to the task doing the copy from the physical cache to the view
  /// @param nbThreadsCopyPhysicalCacheView
  void nbThreadsCopyPhysicalCacheView(size_t nbThreadsCopyPhysicalCacheView) {
    if (nbThreadsCopyPhysicalCacheView == 0) {
      throw std::runtime_error(
          "The number of threads associated to the copy of physical tiles to the view shouldn't be equal to zero");
    }
    nbThreadsCopyPhysicalCacheView_ = nbThreadsCopyPhysicalCacheView;
  }
};

} // fl

#endif //FAST_LOADER_FAST_LOADER_CONFIGURATION_H
