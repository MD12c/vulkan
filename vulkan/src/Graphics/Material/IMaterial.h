#ifndef MATERIAL_INTERFACE_H
#define MATERIAL_INTERFACE_H

#include <cstdint>

using MaterialID                               = uint32_t;
inline constexpr MaterialID NO_MATERIAL        = UINT32_MAX;
inline constexpr MaterialID USE_FILE_MATERIAL  = UINT32_MAX - 1;
inline constexpr MaterialID WIREFRAME_MATERIAL = UINT32_MAX - 2;

class IMaterial
{
public:
    MaterialID ID;

    IMaterial(int ID) : ID(ID) {}
    virtual ~IMaterial() = default;

    virtual void Apply() const = 0;
};

#endif