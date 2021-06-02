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
// Created by anb22 on 2/13/20.
//

#ifndef FASTLOADER_ABSTRACT_TRAVERSAL_H
#define FASTLOADER_ABSTRACT_TRAVERSAL_H

#include <utility>
#include <vector>
#include <cmath>
#include <iomanip>
#include "../api/abstract_tile_loader.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief 3D traversal abstraction
/// @tparam ViewType Type of the view
template<class ViewType>
class AbstractTraversal {
 private:
  std::string
      name_;  ///< Name of the traversal

 protected:
  std::shared_ptr<AbstractTileLoader < ViewType>>
      tl_{}; ///< Tile loader

 public:
  /// @brief Traversal constructor
  /// @param name Traversal name
  /// @param tl Tile Loader
  AbstractTraversal(std::string name, std::shared_ptr<AbstractTileLoader<ViewType>> tl)
  : name_ (std::move(name)), tl_(tl) {}

  /// @brief Default destructor
  virtual ~AbstractTraversal() = default;

  /// \brief Get the number of tiles in a column
  /// @param level Pyramidal level
  /// \return Number of tiles in a column
  [[nodiscard]] uint32_t numberTileHeight(uint32_t level) const { return tl_->numberTileHeight(level); }

  /// \brief Get the number of tiles in a row
  /// @param level Pyramidal level
  /// \return Number of tiles in a row
  [[nodiscard]] uint32_t numberTileWidth(uint32_t level) const { return tl_->numberTileWidth(level); }

  /// \brief Get the number of tiles in depth
  /// @param level Pyramidal level
  /// \return Number of tiles in depth
  [[nodiscard]] uint32_t numberTileDepth(uint32_t level) const { return tl_->numberTileDepth(level); }

  /// \brief Get traversal name
  /// \return Traversal name
  [[nodiscard]] std::string const &name() const { return name_; }

  /// @brief Get the traversal vector for a level
  /// @param level Pyramid level
  /// @return Traversal vector for a level
  [[nodiscard]] virtual std::vector<std::array<uint32_t, 3>> traversal(uint32_t level) const = 0;

  /// @brief Print method
  /// @param level Pyramid level
  /// @param os Output stream operator
  /// @return Output stream operator
  std::ostream &print(uint32_t level = 0, std::ostream &os = std::cout) {
    uint32_t const
        numTileRow = this->numberTileHeight(level),
        numTileCol = this->numberTileWidth(level),
        numTileLayer = this->numberTileDepth(level);

    int
        sizeValue = 0;

    double
        value = numTileRow * numTileCol * numTileLayer;

    while (value != std::floor(value)) { value *= 10; }

    while (std::floor(value) != 0) {
      value /= 10;
      sizeValue++;
    }

    uint32_t mapToPrint[numTileLayer][numTileRow][numTileCol];
    std::fill_n(mapToPrint, numTileRow * numTileCol * numTileLayer, 0);

    uint32_t stepNumber = 0;

    for (auto step: this->traversal(level)) {
      assert(step[0] < numTileRow && step[1] < numTileCol && step[2] < numTileLayer);
      mapToPrint[step[0]][step[1]][step[2]] = stepNumber;
      ++stepNumber;
    }

    os << "Traversal " << this->name() << " (" << numTileRow << "x" << numTileCol << "x" << numTileLayer << ")\n";
    for (uint32_t layer = 0; layer < numTileLayer; ++layer) {
      os << "Layer:" << layer << "\n";
      for (uint32_t row = 0; row < numTileRow; ++row) {
        os << "\t";
        for (uint32_t col = 0; col < numTileCol; ++col) {
          os << std::setw(sizeValue) << mapToPrint[layer][row][col] << " ";
        }
        os << "\n";
      }
      os << "\n";
    }

    return os;
  }

};

}
#endif //FASTLOADER_ABSTRACT_TRAVERSAL_H
