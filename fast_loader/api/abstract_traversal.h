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
class AbstractTraversal {
 private:
  std::string
      name_;  ///< Name of the traversal

 public:
  /// @brief Traversal constructor
  /// @param name Traversal name
  /// @param tl Tile Loader
  explicit AbstractTraversal(std::string name) : name_(std::move(name)) {}

  /// @brief Default destructor
  virtual ~AbstractTraversal() = default;

  /// \brief Get traversal name
  /// \return Traversal name
  [[nodiscard]] std::string const &name() const { return name_; }

  /// @brief Get the traversal vector for a level
  /// @param level Pyramid level
  /// @return Traversal vector for a level
  [[nodiscard]] virtual std::vector<std::array<uint32_t, 3>> traversal(
      uint32_t numberTileHeight, uint32_t numberTileWidth, uint32_t numberTileDepth) const = 0;

  /// @brief Print method
  /// @param level Pyramid level
  /// @param os Output stream operator
  /// @return Output stream operator
  std::ostream &print(uint32_t numberTileHeight,
                      uint32_t numberTileWidth,
                      uint32_t numberTileDepth,
                      std::ostream &os = std::cout) const {
    int
        sizeValue = 0;

    double
        value = numberTileHeight * numberTileWidth * numberTileDepth;

    while (value != std::floor(value)) { value *= 10; }

    while (std::floor(value) != 0) {
      value /= 10;
      sizeValue++;
    }

    std::vector<std::vector<std::vector<uint32_t>>> mapToPrint(
        numberTileDepth, std::vector<std::vector<uint32_t>>(
            numberTileHeight, std::vector<uint32_t>(numberTileWidth)));

    uint32_t stepNumber = 0;

    for (auto step: this->traversal(numberTileHeight, numberTileWidth, numberTileDepth)) {
      assert(step[0] < numberTileHeight && step[1] < numberTileWidth && step[2] < numberTileDepth);
      mapToPrint[step[0]][step[1]][step[2]] = stepNumber;
      ++stepNumber;
    }

    os << "Traversal " << this->name() << " (" << numberTileHeight << "x" << numberTileWidth << "x" << numberTileDepth
       << ")\n";
    for (uint32_t layer = 0; layer < numberTileDepth; ++layer) {
      os << "Layer:" << layer << "\n";
      for (uint32_t row = 0; row < numberTileHeight; ++row) {
        os << "\t";
        for (uint32_t col = 0; col < numberTileWidth; ++col) {
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
