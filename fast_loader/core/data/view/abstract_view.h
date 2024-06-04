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

#ifndef FAST_LOADER_ABSTRACT_VIEW_H
#define FAST_LOADER_ABSTRACT_VIEW_H

#include <memory>
#include <numeric>
#include <ostream>
#include "../view_data/abstract_view_data.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief
/// @tparam DataType Type of data inside the View
template<class DataType>
class AbstractView {
 public:
  using data_t = DataType; ///< direct accessor of the type of data

  /// @brief AbstractView Default constructor
  AbstractView() = default;

  /// @brief AbstractView default destructor
  virtual ~AbstractView() = default;

  /// @brief copy constructor
  /// @param view AbstractView to deep copy
  AbstractView(const AbstractView &view) = default;

  /// @brief Type of filling accessor
  /// @return Filling type
  [[nodiscard]] FillingType fillingType() const { return this->viewData()->fillingType(); }
  /// @brief Pyramidal level accessor
  /// @return Pyramidal level
  [[nodiscard]] size_t level() const { return this->viewData()->level(); }
  /// @brief Number of dimensions accessor
  /// @return Number of dimensions
  [[nodiscard]] size_t nbDims() const { return this->viewData()->nbDims(); }
  /// @brief Dimension names accessor
  /// @return Dimensions names
  [[nodiscard]] std::vector<std::string> const &dimNames() const { return this->viewData()->dimNames(); }

  /// @brief File / full dimensions accessor
  /// @return File / full dimensions
  [[nodiscard]] std::vector<std::size_t> const &fullDims() const { return this->viewData()->fullDims(); }
  /// @brief View dimensions accessor
  /// @return View dimensions
  [[nodiscard]] std::vector<std::size_t> const &viewDims() const { return this->viewData()->viewDims(); }
  /// @brief Tile dimensions accessor
  /// @return Tile dimensions
  [[nodiscard]] std::vector<std::size_t> const &tileDims() const { return this->viewData()->tileDims(); }
  /// @brief Radii accessor
  /// @return Radii for each dimensions
  [[nodiscard]] std::vector<std::size_t> const &radii() const { return this->viewData()->radii(); }
  /// @brief View / central tile index accessor
  /// @return View / central tile index
  [[nodiscard]] std::vector<std::size_t> const &indexCentralTile() const { return this->viewData()->indexCentralTile(); }
  /// @brief "Top-left" position of the central tile in global file coordinate accessor
  /// @return "Top-left" position of the central tile in global file coordinate
  [[nodiscard]] std::vector<std::size_t> globalPositionCentralTile() const {
    std::vector<size_t> globalPosition{};
    std::transform(
        this->indexCentralTile().cbegin(), this->indexCentralTile().cend(),
        this->tileDims().cbegin(), std::back_insert_iterator<std::vector<size_t>>(globalPosition),
        [](auto const &index, auto const &dimension) { return index * dimension; });
    return globalPosition;
  }

  /// @brief File / full dimensions accessor for a dimension
  /// @param dim Dimension index requested
  /// @return File / full dimensions for a dimension
  [[nodiscard]] size_t const &fullDim(std::size_t const dim) const { return fullDims().at(dim); }
  /// @brief View dimensions accessor for a dimension
  /// @param dim Dimension index requested
  /// @return View dimensions for a dimension
  [[nodiscard]] size_t const &viewDim(std::size_t const dim) const { return viewDims().at(dim); }
  /// @brief Tile dimensions accessor for a dimension
  /// @param dim Dimension index requested
  /// @return Tile dimensions for a dimension
  [[nodiscard]] size_t const &tileDim(std::size_t const dim) const { return tileDims().at(dim); }
  /// @brief Radius accessor for a dimension
  /// @param dim Dimension index requested
  /// @return Radius for a dimension
  [[nodiscard]] size_t const &radius(std::size_t const dim) const { return radii().at(dim); }
  /// @brief Index central tile / view accessor for a dimension
  /// @param dim Dimension index requested
  /// @return Index central tile / view for a dimension
  [[nodiscard]] size_t const &indexCentralTile(std::size_t const dim) const { return indexCentralTile().at(dim); }
  /// @brief "Top-left" position of the central tile in global file coordinate accessor for a dimension
  /// @param dim Dimension index requested
  /// @return "Top-left" position of the central tile in global file coordinate for a dimension
  [[nodiscard]] size_t globalPositionCentralTile(std::size_t const dim) const {
    return this->indexCentralTile().at(dim) * this->tileDims().at(dim);
  }

  /// @brief File / full dimensions accessor for a dimension name
  /// @param dimName Dimension name requested
  /// @return File / full dimensions for a dimension
  [[nodiscard]] size_t const &fullDim(std::string const &dimName) const { return fullDim(dimIndex(dimName)); }
  /// @brief View dimensions accessor for a dimension name
  /// @param dimName Dimension name requested
  /// @return View dimensions for a dimension name
  [[nodiscard]] size_t const &viewDim(std::string const &dimName) const { return viewDim(dimIndex(dimName)); }
  /// @brief Tile dimensions accessor for a dimension name
  /// @param dimName Dimension name requested
  /// @return Tile dimensions for a dimension name
  [[nodiscard]] size_t const &tileDim(std::string const &dimName) const { return tileDim(dimIndex(dimName)); }
  /// @brief Radius accessor for a dimension name
  /// @param dimName Dimension name requested
  /// @return Radius for a dimension name
  [[nodiscard]] size_t const &radius(std::string const &dimName) const { return radii().at(dimIndex(dimName)); }
  /// @brief Index central tile / view accessor for a dimension name
  /// @param dimName Dimension name requested
  /// @return Index central tile / view for a dimension name
  [[nodiscard]] size_t const &indexCentralTile(std::string const &dimName) const {
    return indexCentralTile(dimIndex(dimName));
  }
  /// @brief "Top-left" position of the central tile in global file coordinate accessor for a dimension name
  /// @param dimName Dimension name requested
  /// @return "Top-left" position of the central tile in global file coordinate for a dimension name
  [[nodiscard]] size_t globalPositionCentralTile(std::string const &dimName) const {
    return globalPositionCentralTile(dimIndex(dimName));
  }

  /// @brief AbstractView origins centered in the central tile
  /// @return AbstractView origins centered in the central tile
  [[nodiscard]] DataType *viewOrigin() const { return this->viewData()->data(); }

  /// @brief Central tile origin accessor
  /// @return Central tile origin
  [[nodiscard]] DataType *originCentralTile() const {
    return viewData()->data() + computeViewFlattenedPosition(this->radii(), this->radii().size());
  }

  /// @brief Dimensions of "real data" (coming from the file) in the view accessor
  /// @return Dimensions of "real data" (coming from the file) in the view
  [[nodiscard]] std::vector<std::size_t> viewRealDataDims() const {
    std::vector<std::size_t> const
        &fullDimension = this->fullDims(),
        &indexCentralTile = this->indexCentralTile(),
        &tileDimension = this->tileDims(),
        &globalPositionCentralTile = this->globalPositionCentralTile();

    std::vector<size_t> dimension{};
    for (size_t dim = 0; dim < this->viewDims().size(); ++dim) {
      dimension.push_back(
          std::min(
              (indexCentralTile.at(dim) + 1) * tileDimension.at(dim) + radii().at(dim),
              fullDimension.at(dim)
          ) - (size_t) std::max(
              (int64_t) (globalPositionCentralTile.at(dim) - radii().at(dim)),
              (int64_t) 0
          )
      );
    }
    return dimension;
  }

  /// @brief Dimensions of "real data" (coming from the file) in the tile accessor
  /// @return Dimensions of "real data" (coming from the file) in the tile
  [[nodiscard]] std::vector<std::size_t> tileRealDataDims() const {
    std::vector<std::size_t> const
        &fullDimension = this->fullDims(),
        &indexCentralTile = this->indexCentralTile(),
        &tileDimension = this->tileDims(),
        &globalPositionCentralTile = this->globalPositionCentralTile();

    std::vector<size_t> dimension{};
    for (size_t dim = 0; dim < this->viewDims().size(); ++dim) {
      dimension.push_back(
          std::min(
              (indexCentralTile.at(dim) + 1) * tileDimension.at(dim),
              fullDimension.at(dim)
          ) - globalPositionCentralTile.at(dim)
      );
    }
    return dimension;
  }

  /// @brief Dimensions of "real data" (coming from the file) in the view accessor for a dimension index
  /// @param dim Dimension index
  /// @return Dimensions of "real data" (coming from the file) in the view for a dimension index
  [[nodiscard]] std::size_t viewRealDataDim(std::size_t const dim) const {
    return std::min(
        (this->indexCentralTile().at(dim) + 1) * this->tileDims().at(dim) + radii().at(dim),
        this->fullDims().at(dim)
    ) - (size_t) std::max(
        (int64_t) (this->globalPositionCentralTile().at(dim) - radii().at(dim)),
        (int64_t) 0
    );

  }
  /// @brief Dimensions of "real data" (coming from the file) in the tile accessor for a dimension index
  /// @param dim Dimension index
  /// @return Dimensions of "real data" (coming from the file) in the tile for a dimension index
  [[nodiscard]] std::size_t tileRealDataDim(std::size_t const dim) const {
    return
        std::min(
            (this->indexCentralTile().at(dim) + 1) * this->tileDims().at(dim),
            this->fullDims().at(dim)
        ) - this->globalPositionCentralTile().at(dim);
  }

  /// @brief Dimensions of "real data" (coming from the file) in the view accessor for a dimension name
  /// @param dimName Dimension name
  /// @return Dimensions of "real data" (coming from the file) in the view for a dimension name
  [[nodiscard]] std::size_t viewRealDataDim(std::string const &dimName) const {
    return viewRealDataDim(dimIndex(dimName));
  }
  /// @brief Dimensions of "real data" (coming from the file) in the tile accessor for a dimension name
  /// @param dimName Dimension name
  /// @return Dimensions of "real data" (coming from the file) in tile view for a dimension name
  [[nodiscard]] std::size_t tileRealDataDim(std::string const &dimName) const {
    return tileRealDataDim(dimIndex(dimName));

  }

  /// @brief Return the piece of data managed by FastLoader to FastLoader
  void returnToMemoryManager() { this->viewData()->returnToMemoryManager(); }

  /// @brief Transform a dimension name to its index
  /// @param name Dimension name
  /// @return Dimension index related to dimension name
  [[nodiscard]] size_t dimIndex(std::string const &name) const {
    auto const &names = dimNames();
    auto it = std::find(names.cbegin(), names.cend(), name);
    if (it != names.cend()) { return (size_t)std::distance(names.cbegin(), it); }
    else {
      std::ostringstream oss;
      oss << "The dimension \"" << name << "\"" << " does not exist.";
      throw std::runtime_error(oss.str());
    }
  }

  /// @brief Test if a dimension name exists
  /// @param name Dimension name to test
  /// @return True if the dimension exists, else false
  [[nodiscard]] bool hasDim(std::string const &name) const {
    auto const &names = dimNames();
    return std::find(names.cbegin(), names.cend(), name) != names.cend();
  }

  /// @brief Deep copy of the view
  /// @return Deep copy of this view
  [[nodiscard]] virtual std::shared_ptr<fl::internal::AbstractView<DataType>> deepCopy() = 0;

  /// @brief Accessor of internal representation [Should not be used by end user]
  /// @attention Should not be used by end user.
  /// @return Internal representation
  [[nodiscard]] virtual std::shared_ptr<internal::AbstractViewData<DataType>> viewData() const = 0;

  /// @brief View ostream operator, print the view and its data
  /// @param os Stream to print into
  /// @param view View to print
  /// @return Stream with view data inside
  friend std::ostream &operator<<(std::ostream &os, AbstractView const &view) {
    os << "View " << &view << "\n";
    os << *(view.viewData());
    return os;
  }

 private:
  /// @brief Internal representation setter [Should not be used by end user]
  /// @attention Should not be used by end user.
  /// @param viewData Internal representation to set
  virtual void viewData(std::shared_ptr<internal::AbstractViewData<DataType>> viewData) = 0;

  /// @brief Flatten a "pixel" position into the index inside the view
  /// @param position Position to flatten
  /// @param nbDimension Number of dimensions
  /// @param dimension Current dimension
  /// @return Flattened position
  [[nodiscard]] inline std::size_t computeViewFlattenedPosition(std::vector<size_t> const &position,
                                                                size_t const nbDimension,
                                                                size_t const dimension = 0) const {
    if (dimension == nbDimension - 1) { return position.at(dimension); }
    else {
      return position.at(dimension)
          * std::accumulate(viewDims().cbegin() + (long)(dimension + 1), viewDims().cend(), (size_t) 1, std::multiplies<>())
          + computeViewFlattenedPosition(position, nbDimension, dimension + 1);
    }
  }

};

} // fl
} // internal

#endif //FAST_LOADER_ABSTRACT_VIEW_H
