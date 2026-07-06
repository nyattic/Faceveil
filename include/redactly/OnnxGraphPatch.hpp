#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace redactly
{
    [[nodiscard]] std::optional<std::vector<std::uint8_t>>
    makeOnnxSpatialDimsDynamic(const std::vector<std::uint8_t> &modelBytes);
}
