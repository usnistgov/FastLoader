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
// Created by anb22 on 11/22/19.
//

#ifndef FASTLOADER_GRAYSCALE_TIFF_TILE_LOADER_H
#define FASTLOADER_GRAYSCALE_TIFF_TILE_LOADER_H
#include "../fast_loader.h"

#ifdef __APPLE__
#define uint64 uint64_hack_
#define int64 int64_hack_
#include <tiffio.h>
#undef uint64
#undef int64
#else
#include <tiffio.h>
#endif

/// @brief Tile Loader for 2D Grayscale tiff files
/// @tparam DataType AbstractView's internal type
template<class DataType>
 class GrayscaleTiffTileLoader : public fl::AbstractTileLoader<fl::DefaultView<DataType>> {
  TIFF *
      tiff_ = nullptr;             ///< Tiff file pointer

  size_t
      fullHeight_ = 0,           ///< Full height in pixel
      fullWidth_ = 0,            ///< Full width in pixel
      tileHeight_ = 0,            ///< Tile height
      tileWidth_ = 0;             ///< Tile width

  short
      sampleFormat_ = 0,          ///< Sample format as defined by libtiff
      bitsPerSample_ = 0;         ///< Bit Per Sample as defined by libtiff
 public:

  /// @brief GrayscaleTiffTileLoader unique constructor
  /// @param numberThreads Number of threads associated
  /// @param filePath Path of tiff file
  GrayscaleTiffTileLoader(size_t numberThreads, std::string const &filePath)
      : fl::AbstractTileLoader<fl::DefaultView<DataType>>("GrayscaleTiffTileLoader", numberThreads, filePath) {
    short samplesPerPixel = 0;

    // Open the file
    tiff_ = TIFFOpen(filePath.c_str(), "r");
    if (tiff_ != nullptr) {
      if (TIFFIsTiled(tiff_) == 0) { throw (std::runtime_error("Tile Loader ERROR: The file is not tiled.")); }
      // Load/parse header
      TIFFGetField(tiff_, TIFFTAG_IMAGEWIDTH, &(this->fullWidth_));
      TIFFGetField(tiff_, TIFFTAG_IMAGELENGTH, &(this->fullHeight_));
      TIFFGetField(tiff_, TIFFTAG_TILEWIDTH, &this->tileWidth_);
      TIFFGetField(tiff_, TIFFTAG_TILELENGTH, &this->tileHeight_);
      TIFFGetField(tiff_, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
      TIFFGetField(tiff_, TIFFTAG_BITSPERSAMPLE, &(this->bitsPerSample_));
      TIFFGetField(tiff_, TIFFTAG_SAMPLEFORMAT, &(this->sampleFormat_));

      // Test if the file is greyscale
      if (samplesPerPixel != 1) {
        std::stringstream message;
        message << "Tile Loader ERROR: The file is not greyscale: SamplesPerPixel = " << samplesPerPixel << ".";
        throw (std::runtime_error(message.str()));
      }
      // Interpret undefined data format as unsigned integer data
      if (sampleFormat_ < 1 || sampleFormat_ > 3) { sampleFormat_ = 1; }
    } else { throw (std::runtime_error("Tile Loader ERROR: The file can not be opened.")); }
  }

  /// @brief GrayscaleTiffTileLoader destructor
  ~GrayscaleTiffTileLoader() override {
    if (tiff_) {
      TIFFClose(tiff_);
      tiff_ = nullptr;
    }
  }

  /// @brief Load a tiff tile from a view
  /// @param tile Tile to copy into
  /// @param indexRowGlobalTile Tile row index
  /// @param indexColGlobalTile Tile column index
   /// @param indexLayerGlobalTile Tile layer index
  /// @param level Tile's level
  void loadTileFromFile(std::shared_ptr<std::vector<DataType>> tile,
                        size_t indexRowGlobalTile, size_t indexColGlobalTile, [[maybe_unused]] size_t indexLayerGlobalTile,
                        [[maybe_unused]] size_t level) override {
    tdata_t tiffTile = nullptr;
    tiffTile = _TIFFmalloc(TIFFTileSize(tiff_));
    TIFFReadTile(tiff_, tiffTile, indexColGlobalTile * tileWidth_, indexRowGlobalTile * tileHeight_, 0, 0);
    std::stringstream message;
    switch (sampleFormat_) {
      case 1 :
        switch (bitsPerSample_) {
          case 8:loadTile<uint8_t>(tiffTile, tile);
            break;
          case 16:loadTile<uint16_t>(tiffTile, tile);
            break;
          case 32:loadTile<size_t>(tiffTile, tile);
            break;
          case 64:loadTile<uint64_t>(tiffTile, tile);
            break;
          default:
            message
                << "Tile Loader ERROR: The data format is not supported for unsigned integer, number bits per pixel = "
                << bitsPerSample_;
            throw (std::runtime_error(message.str()));
        }
        break;
      case 2:
        switch (bitsPerSample_) {
          case 8:loadTile<int8_t>(tiffTile, tile);
            break;
          case 16:loadTile<int16_t>(tiffTile, tile);
            break;
          case 32:loadTile<int32_t>(tiffTile, tile);
            break;
          case 64:loadTile<int64_t>(tiffTile, tile);
            break;
          default:
            message
                << "Tile Loader ERROR: The data format is not supported for signed integer, number bits per pixel = "
                << bitsPerSample_;
            throw (std::runtime_error(message.str()));
        }
        break;
      case 3:
        switch (bitsPerSample_) {
          case 8:
          case 16:
          case 32:
            loadTile<float>(tiffTile, tile);
            break;
          case 64:
            loadTile<double>(tiffTile, tile);
            break;
          default:
            message
                << "Tile Loader ERROR: The data format is not supported for float, number bits per pixel = "
                << bitsPerSample_;
            throw (std::runtime_error(message.str()));
        }
        break;
      default:message << "Tile Loader ERROR: The data format is not supported, sample format = " << sampleFormat_;
        throw (std::runtime_error(message.str()));
    }

    _TIFFfree(tiffTile);
  }

  /// @brief Copy Method for the GrayscaleTiffTileLoader
  /// @return Return a copy of the current GrayscaleTiffTileLoader
  std::shared_ptr<fl::AbstractTileLoader<fl::DefaultView<DataType>>> copyTileLoader() override {
    return std::make_shared<GrayscaleTiffTileLoader<DataType>>(this->numberThreads(), this->filePath());
  }

  /// @brief Tiff file height
  /// @param level Tiff level [not used]
  /// @return Full height
  [[nodiscard]] size_t fullHeight([[maybe_unused]] size_t level) const override { return fullHeight_; }
  /// @brief Tiff full width
  /// @param level Tiff level [not used]
  /// @return Full width
  [[nodiscard]] size_t fullWidth([[maybe_unused]] size_t level) const override { return fullWidth_; }
  /// @brief Tiff tile width
  /// @param level Tiff level [not used]
  /// @return Tile width
  [[nodiscard]] size_t tileWidth([[maybe_unused]] size_t level) const override { return tileWidth_; }
  /// @brief Tiff tile height
  /// @param level Tiff level [not used]
  /// @return Tile height
  [[nodiscard]] size_t tileHeight([[maybe_unused]] size_t level) const override { return tileHeight_; }
  /// @brief Tiff bits per sample
  /// @return Size of a sample in bits
  [[nodiscard]] short bitsPerSample() const override { return bitsPerSample_; }
  /// @brief Level accessor
  /// @return 1
  [[nodiscard]] size_t numberPyramidLevels() const override { return 1; }

 private:
  /// @brief Private function to copy and cast the values
  /// @tparam FileType Type inside the file
  /// @param src Piece of memory coming from libtiff
  /// @param dest Piece of memory to fill
  template<typename FileType>
  void loadTile(tdata_t src, std::shared_ptr<std::vector<DataType>> &dest) {
    for (size_t i = 0; i < tileHeight_ * tileWidth_; ++i) { dest->data()[i] = (DataType) ((FileType *) (src))[i]; }
  }

};

#endif //FASTLOADER_GRAYSCALE_TIFF_TILE_LOADER_H
