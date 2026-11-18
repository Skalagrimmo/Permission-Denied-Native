# Vector / ASCII Prototype

## Purpose

This is the first hybrid rendering experiment for Permission Denied. It combines the existing procedural shader backdrop with a native C++ wireframe scene and world-space ASCII signage. The visual target is a readable, futuristic simulation space rather than a pixel-art filter.

## Rendering pipeline

1. The existing OpenGL ES 2.0 / desktop OpenGL full-screen pass draws the dark sky, distant skyline, horizon and subtle procedural ground field.
2. `build_vector_scene()` builds a small corridor from 3D line segments using the current native player position and camera angles.
3. The scene builder transforms world points into camera space using the same yaw/pitch convention as the Nano-punk camera:
   - translate by the camera position;
   - rotate around Y (yaw);
   - rotate around X (pitch).
4. Segments are clipped against a near plane (0.10) and far plane (90.0), then projected to normalized device coordinates.
5. A second shader draws the projected vertices as depth-tested `GL_LINES`. The dynamic vertex buffer carries NDC position and flat RGB color.
6. World-space ASCII glyphs and the touch HUD accent are appended to that same line pass.

The projection uses a focal length of 0.82 screen heights. Depth values use the conventional perspective mapping for the same near/far planes, allowing the depth buffer to hide lines behind nearer structures.

## Geometry in the test scene

- Perspective floor grid and paired guide rails.
- Long corridor walls, repeated braces, overhead conduits and columns.
- Two open portal frames with header panels.
- Terminal housings with simple line-based display readouts.
- 5×7 bitmap glyphs converted into tiny 3D rectangular strokes. The current signs read `PERMISSION DENIED`, `ACCESS REVOKED / 07`, `SECTOR 08` and `NO OPERATOR / NO SIGNAL`.
- Screen-space vector accents for the existing left movement stick and right sprint button.

The original shader grid remains as a subdued background layer; its intensity is reduced so it does not compete with the actual 3D floor and wall geometry.

## Constraints and next work

- No texture atlas, lighting, shadows or material system is required for this prototype.
- The current corridor is procedural renderer test geometry, not the generated game world and not yet connected to world collision.
- Geometry is rebuilt and CPU-projected every frame. This is intentionally simple for the first Android proof; profiling will determine whether to cache static world vertices and update only camera transforms.
- Glyphs are currently uppercase 5×7 shapes. A later ASCII surface renderer can vary symbol density and character sets across actual wall/ground meshes.
- Validate touch handling and sustained frame time by direct playtesting on the J710F. The first build has been installed and launched successfully; detailed performance measurements are still pending.
