#pragma once
#include <stdint.h>

// -------------------------------------------------------------
//  Rotation
//  Represents the four supported 90-degree snap rotations.
//  Applied by the renderer during drawing — sprite pixel data
//  itself never changes, the renderer reads it in a different
//  order depending on this value.
//
//  R0   — Normal, no rotation
//  R90  — 90 degrees clockwise
//  R180 — 180 degrees (upside down)
//  R270 — 270 degrees clockwise (or 90 counter-clockwise)
// -------------------------------------------------------------
enum class Rotation : uint8_t { R0   = 0, R90  = 1, R180 = 2, R270 = 3 };