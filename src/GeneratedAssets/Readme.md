# Generated Assets

This folder is reserved for auto-generated asset headers used by WolfEngine.
Files here are produced by tools or asset pipelines and should not be edited by
hand unless you are intentionally changing the generator output.

## Current Purpose

The main generated header in this folder is `WE_Assets.hpp`. It exists as a
central include point for generated game data such as sprites, palettes, and
other compile-time assets.

## How To Use Generated Assets

1. Include the generated header from game code instead of copying asset data
	into source files.
2. Access generated content through the `Assets` namespace.
3. Keep source data in the generator-friendly format, then regenerate this
	folder when assets change.
4. Do not add manual logic here. This folder should stay data-only and easy to
	replace from generation.

## Example

```cpp
#include "GeneratedAssets/WE_Assets.hpp"

// Use generated sprites, palettes, or other assets from the Assets namespace.
```

## Rules

- Treat these files as build artifacts.
- Regenerate them when the source artwork or palette data changes.
- Do not rely on local edits surviving regeneration.
- If you need new generated content, add it through the generator pipeline,
  then update this folder from the generated output.
