#pragma once

namespace Wheel
{
    namespace Helpers
    {
        // Draw order key. Lower draws first (further back). Sprites carry their
        // own level; render features offset relative to their owner's level.
        using RenderLevel = int;
    }
}
