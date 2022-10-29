#pragma once

#include <vector>
#include <cstdint>

namespace vio::examples::utils
{
struct simple_frame
{
    explicit simple_frame() = default;
    ~simple_frame() = default;
    
    uint8_t* data = {};
	double pts = 0.0;
};

}