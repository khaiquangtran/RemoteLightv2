#ifndef PACKAGE_H
#define PACKAGE_H
#include <vector>

namespace utils
{
  class Package
  {
  public:
    Package(const Package &) = default;
    Package(Package &&) noexcept = default;
    Package &operator=(const Package &) = default;
    Package &operator=(Package &&) noexcept = default;

    explicit Package(const std::string &str)
    {
      mData.assign(str.begin(), str.end());
    }

    explicit Package(const std::vector<int32_t> &data) : mData(data)
    {
    }

    const int32_t *getPackage() const { return mData.data(); }
    int32_t getSize() const { return static_cast<int32_t>(mData.size()); }

  private:
    std::vector<int32_t> mData;
  };
} // namespace utils

#endif // PACKAGE_H