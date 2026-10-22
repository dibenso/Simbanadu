"""
USAGE:
1. Add bezier curve
2. Set handle type automatic in edit mode
3. Remove initial bezier curve
4. Create a hand drawn bezier curve
5. Center bezier curve at origin (0, 0, 0)
6. Straighten / even out splines (use snap to grid)
7. Create an object, such as a cylinder (if cylinder, here's how to cut it in half: https://blender.stackexchange.com/questions/26477/how-to-make-a-half-cylinder)
8. Add Array modifier
9. Add curve modifier
10. Add solidify modifier (if you want to see inside of model in raylib; AKA backface culling)
11. Geometry Node Editor (bottom left of Blender window; Object Mode; Make sure to select just the Bezier Curve object)
12. In geometry node editor, click `+ New`
13. Add -> Curve -> Operations -> Resample curve
14. Connect dots in geometry node editor: Geometry -> Curve -> Geometry
15. Set count to the amount of evenly spaced points you want in the curve in the resample curve node
16. Convert to mesh
17. Make sure bezier curve is selected in object mode
18. Copy this script into Blender python console
19. Run `write_bezier_points_to_file()` in Blender python console
20. Check file contents at location of filename parameter below
21. Convert back to curve

TODO: Can this be removed?
For example, by reading `resources/models/tracks/track1/track1.obj`, it seems to contains many "vertices?"
that correspond to points on the track from `src/tracks/track1.h`:
v 4.982807 0.270256 8.412079

Can the model objects from raylib provide us with some information? Something like:
`game->trackModel.meshes[0].vertices`?
"""

import bpy


def write_bezier_points_to_file(
    filename="<path to project>/src/tracks/<name of new track header file>.h",
):
    vertices = bpy.context.object.data.vertices
    header_filename = filename.split("/")[-1]
    header_filename_without_extension = header_filename.split(".h")[0].lower()
    header_name = header_filename_without_extension.upper() + "_H"

    with open(filename, "w") as f:
        f.write("/**\n")
        f.write(
            " * This file was generated with scripts/blender/writeBezierPointsToHeaderFile.py\n"
        )
        f.write(" *\n")
        f.write(" * DO NOT MODIFY MANUALLY\n")
        f.write(" */\n")
        f.write("\n")
        f.write("#ifndef " + header_name + "\n")
        f.write("#define " + header_name + "\n")
        f.write("\n")
        f.write('#include "raylib.h"\n')
        f.write("\n")
        f.write(
            "Vector3 "
            + header_filename_without_extension
            + "["
            + str(len(vertices))
            + "] = {\n"
        )

        for i in range(len(vertices)):
            f.write(
                "    {"
                + str(round(vertices[i].co.x, 4))
                + "f, "
                + str(round(vertices[i].co.z, 4))
                + "f, "
                + str(round(-vertices[i].co.y, 4))
                + "f}"
                + ("," if i != len(vertices) - 1 else "")
                + "\n"
            )

        f.write("};\n")
        f.write("\n")
        f.write("#endif // " + header_name + "\n")


write_bezier_points_to_file()
