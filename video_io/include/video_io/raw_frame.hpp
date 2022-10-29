#pragma once

#include <vector>
#include <cstdint>

namespace vio
{
struct simple_frame
{
    explicit simple_frame() = default;
    ~simple_frame() = default;
    
    std::vector<uint8_t> data = std::vector<uint8_t>();
	double pts = 0.0;
};

}