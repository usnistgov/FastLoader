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
// Created by anb22 on /8/9.
//

#ifndef FASTLOADER_ABSTRACT_VIEW_H
#define FASTLOADER_ABSTRACT_VIEW_H

#include "../../graph/tasks/view_loader.h"

/// @brief FastLoader namespace
namespace fl {

namespace internal {
/// @brief Main interface to access the data contained in a file.
/// @details Defined by a central tile and a halo. The whole view is stored contiguously in memory. For example
/// for a view of depth, height, width the view is stored based on layer * height * width + row * width + col
/// @note
/// A view of radius 2, tile size of 2 (h as halo, t as central tile information): \n
/// hhhhhh\n
/// hhhhhh\n
/// hhtthh\n
/// hhtthh\n
/// hhhhhh\n
/// hhhhhh\n
/// is stored as hhhhhhhhhhhhhhtthhhhtthhhhhhhhhhhhhh.
/// @details
/// A view is sent by FastLoaderGraph, by calling FastLoaderGraph<ViewDataType>::getBlockingResult().
/// Multiple accessor are available to get the view's geometry:
/// - Radius: Base view radius, number of pixel in the halo in a direction
/// - tileHeight / tileWidth / tileDepth: Tile size
/// - viewHeight / viewWidth / viewDepth: View size (tileHeight + radius*2 / tileWidth + radius*2 / tileDepth + radius * 2)
/// - realDataHeight / realDataWidth / realDataDepth: geometry of the data from the file (including radius), based on the top left
/// hand corner of the central tile. This value could be different from the tileHeight / tileWidth / tileDepth if the height, width, or depth
/// are not evenly divisible by the tile's height, width, or depth (often along the outer edges of an image).
/// @note
/// For example, an file of 5*5, a tile of 3*3, and a radius of 1, the view (1,1) will look like (h as halo, t as
/// central tile information, r as central tile information coming directly from the file and not from the border
/// creator):\n
/// hhhhh\n
/// hrrth\n
/// hrrth\n
/// httth\n
/// hhhhh\n
/// radius() will return 1, tileHeight()/tileWidth() will return 3, viewHeight()/viewWidth() will return 5,
/// realDataHeight()/realDataWidth() will return 2.
/// @details
/// Accessors are available to get access to different part of the view.
/// @note
/// For exemple a view of radius 2 and tile size 2 (v as view data, and p as pointed element):\n
/// vvvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// viewOrigin() will return the top left hand corner pointer of the view:\n
/// pvvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// originCentralTile() will return the top left hand corner pointer of the central tile:\n
/// vvvvvv\n
/// vvvvvv\n
/// vvpvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// viewOrigin(size_t radius) will return the top left hand corner pointer of the view as if the radius is of value
/// given, viewOrigin(1) will return:\n
/// vvvvvv\n
/// vpvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// vvvvvv\n
/// and viewOrigin(0) will return the same pointer as originCentralTile().
/// @details
/// To access smaller radius views, the geometry viewHeight(size_t radius)/viewWidth(size_t radius)/viewDepth(size_t radius) are available, which
/// can be used with viewOrigin(size_t radiusHeight, size_t radiusWidth, size_t radiusDepth) to access the starting location of
/// a smaller radius view.\n
/// So to access all elements from the central tile (with viewDepth of 0 and radiusDepth of 0):
/// @code
/// while (auto view = fl.getBlockingResult()) {
///   for (size_t row = 0; row < view->tileHeight(); ++row) {
///     for (size_t col = 0; col < view->tileWidth(); ++col) {
///       auto val = *(view->originCentralTile() + (row * view->viewWidth() + col));
///       // Do stuff with val
///     }
///   }
///   view->returnToMemoryManager();
/// }
/// @endcode
/// So to access all elements from the central tile that come directly from the file:
/// @code
///   while (auto view = fl.getBlockingResult()) {
///    for (size_t layer = 0; layer < view->realDataDepth(); ++layer) {
///     for (size_t row = 0; row < view->realDataHeight(); ++row) {
///       for (size_t col = 0; col < view->realDataWidth(); ++col) {
///         auto val = *(view->originCentralTile() + (layer * view->viewHeight() * view->viewWidth() + row * view->viewWidth() + col));
///         // Do stuff with val
///      }
///    }
///    view->returnToMemoryManager();
///  }
/// @endcode
/// Finally, if smaller region of the view are needed in a contiguous way, AbstractView<DataType>::extractViewDeepCopy(), can
/// be used.
/// @attention AbstractView<DataType>::extractViewDeepCopy() do a deep copy and this memory is not managed by FastLoader.
/// @tparam DataType
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

  /// @brief deep copy of the view
  /// @return a deep copy of this view
  [[nodiscard]] virtual std::shared_ptr<fl::internal::AbstractView<DataType>> deepCopy() = 0;

  /// @brief Accessor of internal representation [Should not be used by end user]
  /// @attention Should not be used by end user.
  /// @return Internal representation
  [[nodiscard]] virtual std::shared_ptr<internal::AbstractViewData<DataType>> viewData() const = 0;

  /// @brief Type of filling accessor used to populate the halo that do not come from the file.
  /// @return Type of filling used to populate the halo that do not come from the file.
  [[nodiscard]] FillingType fillingType() const { return viewData()->fillingType(); }

  /// @brief AbstractView's radius height accessor
  /// @return AbstractView's radius height
  [[nodiscard]] size_t radiusHeight() const { return this->viewData()->radiusHeight(); }

  /// @brief AbstractView's radius width accessor
  /// @return AbstractView's radius width
  [[nodiscard]] size_t radiusWidth() const { return this->viewData()->radiusWidth(); }

  /// @brief AbstractView's radius depth accessor
  /// @return AbstractView's radius depth
  [[nodiscard]] size_t radiusDepth() const { return this->viewData()->radiusDepth(); }

  /// @brief AbstractView pyramid level accessor
  /// @return AbstractView pyramid level
  [[nodiscard]] size_t level() const { return this->viewData()->level(); }

  /// @brief Tile height accessor
  /// @return Tile height
  [[nodiscard]] size_t tileHeight() const { return this->viewData()->tileHeight(); }
  /// @brief Tile width accessor
  /// @return Tile width
  [[nodiscard]] size_t tileWidth() const { return this->viewData()->tileWidth(); }
  /// @brief Tile depth accessor
  /// @return Tile depth
  [[nodiscard]] size_t tileDepth() const { return this->viewData()->tileDepth(); }
  /// @brief Number of channels accessor
  /// @return Number of channels
  [[nodiscard]] size_t numberChannels() const { return this->viewData()->numberChannels(); }

  /// @brief Real data height in a central tile accessor
  /// @return Real data height in a central tile
  [[nodiscard]] size_t realDataHeight() const {
    return std::min(viewData()->fullHeight(), (tileRowIndex() + 1) * tileHeight()) - tileGlobalRow();
  }
  /// @brief Real data width in a central tile accessor
  /// @return Real data width in a central tile
  [[nodiscard]] size_t realDataWidth() const {
    return std::min(viewData()->fullWidth(), (tileColIndex() + 1) * tileWidth()) - tileGlobalCol();
  }

  /// @brief Real data width in a central tile accessor
  /// @return Real data width in a central tile
  [[nodiscard]] size_t realDataDepth() const {
    return std::min(viewData()->fullDepth(), (tileLayerIndex() + 1) * tileDepth()) - tileGlobalLayer();
  }

  /// @brief Default view height accessor
  /// @return Default view height
  [[nodiscard]] size_t viewHeight() const { return this->viewData()->viewHeight(); }
  /// @brief Default view width accessor
  /// @return Default view width
  [[nodiscard]] size_t viewWidth() const { return this->viewData()->viewWidth(); }
  /// @brief Default view depth accessor
  /// @return Default view depth
  [[nodiscard]] size_t viewDepth() const { return this->viewData()->viewDepth(); }

  /// @brief Row position in the file grid of the central tile accessor
  /// @return Row position in the file grid of the central tile
  [[nodiscard]] size_t tileRowIndex() const { return this->viewData()->indexRowCenterTile(); }

  /// @brief Column position in the file grid of the central tile accessor
  /// @return Column position in the file grid of the central tile
  [[nodiscard]] size_t tileColIndex() const { return this->viewData()->indexColCenterTile(); }

  /// @brief Depth position in the file grid of the central tile accessor
  /// @return Depth position in the file grid of the central tile
  [[nodiscard]] size_t tileLayerIndex() const { return this->viewData()->indexLayerCenterTile(); }

  /// @brief Position of the front upper left hand corner pixel row of the central tile in the file accessor
  /// @return Position of the front upper left hand corner pixel row of the central tile in the file
  [[nodiscard]] size_t tileGlobalRow() const { return tileRowIndex() * tileHeight(); }

  /// @brief Position of the front upper left hand corner pixel column of the central tile in the file accessor
  /// @return Position of the front upper left hand corner pixel column of the central tile in the file
  [[nodiscard]] size_t tileGlobalCol() const { return tileColIndex() * tileWidth(); }

  /// @brief Position of the front upper left hand corner pixel layer of the central tile in the file accessor
  /// @return Position of the front upper left hand corner pixel layer of the central tile in the file
  [[nodiscard]] size_t tileGlobalLayer() const { return tileLayerIndex() * tileDepth(); }

  /// @brief AbstractView height for another radius height accessor
  /// @param radiusHeight Other radius asked
  /// @return AbstractView height for another radius
  [[nodiscard]] size_t viewHeight(size_t radiusHeight) const {
    if (radiusHeight <= this->viewData()->radiusHeight()) {
      return this->viewData()->viewHeight() - (2 * (this->viewData()->radiusHeight() - radiusHeight));
    } else {
      std::ostringstream oss;
      oss << "You have requested a radius superior to the original one (" << radiusHeight << " > "
          << this->viewData()->radiusHeight() << ")" << std::endl;
      throw (std::runtime_error(oss.str()));
    }
  }

  /// @brief AbstractView width for another radius accessor
  /// @param radiusWidth Other radius width asked
  /// @return AbstractView width for another radius width
  [[nodiscard]] size_t viewWidth(size_t radiusWidth) const {
    if (radiusWidth <= this->viewData()->radiusWidth()) {
      return this->viewData()->viewWidth() - (2 * (this->viewData()->radiusWidth() - radiusWidth));
    } else {
      std::ostringstream oss;
      oss << "You have requested a radius superior to the original one (" << radiusWidth << " > "
          << this->viewData()->radiusWidth() << ")" << std::endl;
      throw (std::runtime_error(oss.str()));
    }
  }

  /// @brief AbstractView depth for another radius accessor
  /// @param radiusDepth Other radius depth asked
  /// @return AbstractView depth for another radius depth
  [[nodiscard]] size_t viewDepth(size_t radiusDepth) const {
    if (radiusDepth <= this->viewData()->radiusDepth()) {
      return this->viewData()->viewDepth() - (2 * (this->viewData()->radiusDepth() - radiusDepth));
    } else {
      std::ostringstream oss;
      oss << "You have requested a radius superior to the original one (" << radiusDepth << " > "
          << this->viewData()->radiusDepth() << ")" << std::endl;
      throw (std::runtime_error(oss.str()));
    }
  }

  /// @brief AbstractView origins centered in the central tile
  /// @return AbstractView origins centered in the central tile
  [[nodiscard]] DataType *viewOrigin() const { return viewData()->data(); }

  /// @brief AbstractView origins centered in the central tile for a smaller radius accessor
  /// @param sharedRadius Radius asked inferior or equal to the one given to FastLoader
  /// @return AbstractView origins centered in the central tile for a smaller radius
  [[nodiscard]] DataType *viewOrigin(size_t sharedRadius) const {
    return viewOrigin(sharedRadius, sharedRadius, sharedRadius);
  }

  /// @brief AbstractView origins centered in the central tile for a smaller radius accessor
  /// @param radiusHeight Height radius asked inferior or equal to the one given to FastLoader
  /// @param radiusWidth Width radius asked inferior or equal to the one given to FastLoader
  /// @param radiusDepth Depth radius asked inferior or equal to the one given to FastLoader
  /// @return AbstractView origins centered in the central tile for a smaller radius
  [[nodiscard]] DataType *viewOrigin(size_t radiusHeight, size_t radiusWidth, size_t radiusDepth) const {
    if (radiusHeight <= this->viewData()->radiusHeight() &&
        radiusWidth <= this->viewData()->radiusWidth() &&
        radiusDepth <= this->viewData()->radiusDepth()) {
      return
          viewData()->data() + (
              (this->radiusDepth() - radiusDepth) * this->viewWidth() * this->viewHeight()
                  + (this->radiusHeight() - radiusHeight) * this->viewWidth()
                  + (this->radiusWidth() - radiusWidth)
          ) * numberChannels();
    } else {
      std::ostringstream oss;
      oss << "You have requested a radius superior to the original one ("
          << radiusHeight << " > " << this->viewData()->radiusHeight() << " "
          << radiusWidth << " > " << this->viewData()->radiusWidth() << " "
          << radiusDepth << " > " << this->viewData()->radiusDepth() << ")"
          << std::endl;
      throw (std::runtime_error(oss.str()));
    }
  }

  /// @brief Central tile origin accessor
  /// @return Central tile origin
  [[nodiscard]] DataType *originCentralTile() const {
    return
        viewData()->data() + (
            this->radiusDepth() * this->viewWidth() * this->viewHeight()
                + this->radiusHeight() * this->viewWidth()
                + this->radiusWidth()
        ) * numberChannels();
  }

  /// @brief Internal representation setter [Should not be used by end user]
  /// @attention Should not be used by end user.
  /// @param viewData Internal representation to set
  virtual void viewData(std::shared_ptr<internal::AbstractViewData<DataType>> viewData) = 0;

  /// @brief Extract and do a deepcopy of an internal view to a new contiguous piece of memory
  /// @param sharedRadius Radius of the extracted view centered in the central tile
  /// @return Vector containing the contiguous view
  /// @attention AbstractView<DataType>::extractViewDeepCopy() do a deep copy and this memory is not managed by FastLoader.
  std::shared_ptr<std::vector<DataType>> extractViewDeepCopy(size_t sharedRadius){
    return extractViewDeepCopy(sharedRadius, sharedRadius, sharedRadius);
  }

  /// @brief Extract and do a deepcopy of an internal view to a new contiguous piece of memory
  /// @param radiusHeight Height radius asked inferior or equal to the one given to FastLoader
  /// @param radiusWidth Width radius asked inferior or equal to the one given to FastLoader
  /// @param radiusDepth Depth radius asked inferior or equal to the one given to FastLoader
  /// @return Vector containing the contiguous view
  /// @attention AbstractView<DataType>::extractViewDeepCopy() do a deep copy and this memory is not managed by FastLoader.
  std::shared_ptr<std::vector<DataType>> extractViewDeepCopy(size_t radiusHeight,
                                                             size_t radiusWidth,
                                                             size_t radiusDepth) {
    if (radiusHeight <= this->viewData()->radiusHeight() &&
        radiusWidth <= this->viewData()->radiusWidth() &&
        radiusDepth <= this->viewData()->radiusDepth()) {

      auto ret = std::make_shared<std::vector<DataType>>(
          viewDepth(radiusDepth) * viewWidth(radiusWidth) * viewHeight(radiusHeight) * numberChannels()
      );

      auto viewOriginBase =
          viewData()->data() + (
              (this->radiusDepth() - radiusDepth) * this->viewWidth() * this->viewHeight()
                  + (this->radiusHeight() - radiusHeight) * this->viewWidth()
                  + (this->radiusWidth() - radiusWidth)
          ) * numberChannels();

      for (size_t layer = 0; layer < viewDepth(radiusDepth); ++layer) {
        for (size_t row = 0; row < viewHeight(radiusHeight); ++row) {
          std::copy_n(
              viewOriginBase + (layer * viewWidth() * viewHeight() + row * viewWidth()) * numberChannels(),
              viewWidth(radiusWidth) * numberChannels(),
              ret->begin() + (layer * viewWidth(radiusWidth) * viewHeight(radiusHeight) + row * viewWidth(radiusWidth)))
              * numberChannels();
        }
      }

      return ret;
    } else {
      std::ostringstream oss;
      oss << "You have requested a radius superior to the original one ("
          << radiusHeight << " > " << this->viewData()->radiusHeight() << " "
          << radiusWidth << " > " << this->viewData()->radiusWidth() << " "
          << radiusDepth << " > " << this->viewData()->radiusDepth() << ")"
          << std::endl;
      throw (std::runtime_error(oss.str()));
    }
  }

  /// @brief Return the piece of data managed by FastLoader to FastLoader
  void returnToMemoryManager() { this->viewData()->returnToMemoryManager(); }

  /// @brief Test if a view (viewData) is connected to a memory manager
  /// @return True if a viewData is connected, else False
  bool isMemoryManagerConnected() { return this->viewData()->isMemoryManagerConnected(); }

  /// @brief AbstractView's print method
  /// @param setWidthPrint Number of characters used to print a data pf the view
  /// @param os Output stream to print information (default std::cout)
  /// @return The same output stream as parameter for chaining
  std::ostream &printView(size_t setWidthPrint = 0, std::ostream &os = std::cout) const {
    return printView < DataType > (setWidthPrint, os);
  }

  /// @brief AbstractView's print method
  /// @tparam CAST Output casting, useful to transform char (uint8_t) to int
  /// @param setWidthPrint Number of characters used to print a data pf the view
  /// @param os Output stream to print information (default std::cout)
  /// @return The same output stream as parameter for chaining
  template<class CAST=DataType>
  std::ostream &printView(size_t setWidthPrint = 0, std::ostream &os = std::cout) const {
    size_t
        row = 0,
        col = 0,
        channel = 0;

    os << "View lvl: " << this->level()
       << " (R:" << this->tileRowIndex() << "/C:" << this->tileColIndex() << "/L:" << this->tileLayerIndex()
       << ") Size: Tile(H:" << this->tileHeight() << "/W:" << this->tileWidth() << "/D:" << this->tileDepth()
       << ") Real data(H:" << this->realDataHeight() << "/W:" << this->realDataWidth() << "/D:" << this->realDataDepth()
       << ") View: (H:" << this->viewHeight() << "/W:" << this->viewWidth() << "/D:" << this->viewDepth() << ")"
       << " Number of channels: " << this->numberChannels()
       << std::endl;
    for (size_t layer = 0; layer < viewDepth(); ++layer) {
      row = 0;
      col = 0;
      os << "Layer: " << layer << "\n";
      for (; row < this->radiusHeight(); ++row) {
        os << "\t";
        for (col = 0; col < this->radiusWidth(); ++col) {
          os << std::setw(setWidthPrint)
             << (CAST) (this->viewOrigin()[
                 (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
             ]);
          for (channel = 2; channel <= this->numberChannels(); ++channel) {
            os << ","
               << std::setw(setWidthPrint)
               << (CAST) (this->viewOrigin()[
                   (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
                       + channel - 1
               ]);
          }
          os << " ";
        }
        os << std::setw(setWidthPrint) << " ";
        for (; col < this->radiusWidth() + this->realDataWidth(); ++col) {
          os << std::setw(setWidthPrint)
             << (CAST) (this->viewOrigin()[
                 (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
             ]);
          for (channel = 2; channel <= this->numberChannels(); ++channel) {
            os << ","
               << std::setw(setWidthPrint)
               << (CAST) (this->viewOrigin()[
                   (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
                       + channel - 1
               ]);
          }
          os << " ";
        }
        os << std::setw(setWidthPrint) << " ";
        for (; col < this->viewWidth(); ++col) {
          os << std::setw(setWidthPrint)
             << (CAST) (this->viewOrigin()[
                 (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
             ]);
          for (channel = 2; channel <= this->numberChannels(); ++channel) {
            os << ","
               << std::setw(setWidthPrint)
               << (CAST) (this->viewOrigin()[
                   (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
                       + channel - 1
               ]);
          }
          os << " ";
        }
        os << "\n";
      }
      os << "\n";
      for (; row < this->radiusHeight() + this->realDataHeight(); ++row) {
        os << "\t";
        for (col = 0; col < this->radiusWidth(); ++col) {
          os << std::setw(setWidthPrint)
             << (CAST) (this->viewOrigin()[
                 (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
             ]);
          for (channel = 2; channel <= this->numberChannels(); ++channel) {
            os << ","
               << std::setw(setWidthPrint)
               << (CAST) (this->viewOrigin()[
                   (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
                       + channel - 1
               ]);
          }
          os << " ";
        }
        os << std::setw(setWidthPrint) << " ";
        for (; col < this->radiusWidth() + this->realDataWidth(); ++col) {
          os << std::setw(setWidthPrint)
             << (CAST) (this->viewOrigin()[
                 (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
             ]);
          for (channel = 2; channel <= this->numberChannels(); ++channel) {
            os << ","
               << std::setw(setWidthPrint)
               << (CAST) (this->viewOrigin()[
                   (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
                       + channel - 1
               ]);
          }
          os << " ";
        }
        os << std::setw(setWidthPrint) << " ";
        for (; col < this->viewWidth(); ++col) {
          os << std::setw(setWidthPrint)
             << (CAST) (this->viewOrigin()[
                 (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
             ]);
          for (channel = 2; channel <= this->numberChannels(); ++channel) {
            os << ","
               << std::setw(setWidthPrint)
               << (CAST) (this->viewOrigin()[
                   (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
                       + channel - 1
               ]);
          }
          os << " ";
        }
        os << "\n";
      }
      os << "\n";
      for (; row < this->viewHeight(); ++row) {
        os << "\t";
        for (col = 0; col < this->radiusWidth(); ++col) {
          os << std::setw(setWidthPrint)
             << (CAST) (this->viewOrigin()[
                 (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
             ]);
          for (channel = 2; channel <= this->numberChannels(); ++channel) {
            os << ","
               << std::setw(setWidthPrint)
               << (CAST) (this->viewOrigin()[
                   (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
                       + channel - 1
               ]);
          }
          os << " ";
        }
        os << std::setw(setWidthPrint) << " ";
        for (; col < this->radiusWidth() + this->realDataWidth(); ++col) {
          os << std::setw(setWidthPrint)
             << (CAST) (this->viewOrigin()[
                 (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
             ]);
          for (channel = 2; channel <= this->numberChannels(); ++channel) {
            os << ","
               << std::setw(setWidthPrint)
               << (CAST) (this->viewOrigin()[
                   (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
                       + channel - 1
               ]);
          }
          os << " ";
        }
        os << std::setw(setWidthPrint) << " ";
        for (; col < this->viewWidth(); ++col) {
          os << std::setw(setWidthPrint)
             << (CAST) (this->viewOrigin()[
                 (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
             ]);
          for (channel = 2; channel <= this->numberChannels(); ++channel) {
            os << ","
               << std::setw(setWidthPrint)
               << (CAST) (this->viewOrigin()[
                   (layer * this->viewWidth() * this->viewHeight() + row * this->viewWidth() + col) * numberChannels()
                       + channel - 1
               ]);
          }
          os << " ";
        }
        os << "\n";
      }
      os << "\n";
    }
    return os;
  }

  /// @brief AbstractView's stream output operator
  /// @param os Output stream to print information
  /// @param data AbstractView to print
  /// @return The same output stream as parameter for chaining
  friend std::ostream &operator<<(std::ostream &os, [[maybe_unused]]AbstractView const &data) {
    data.printView(3, os);
    return os;
  }
};
}
}
#endif //FASTLOADER_ABSTRACT_VIEW_H
