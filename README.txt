Usage: 
NQVTK.exe surface1.vtk surface2.vtk - distfield2.vti distfield1.vti

note the reversed order of distance fields (these are the fields that should be sampled for an object, not the field corresponding to that object)
surfaces should be VTK XML PolyData
distance fields should be VTK XML ImageData

Example:
If \\athene\datasets is mounted as drive R do the following:
NQVTK.exe r:\stef\msdata\Ventricles\stef_ventricle_199_20040510_textured.vtp r:\stef\msdata\Ventricles\stef_ventricle_199_20041124_textured.vtp - r:\stef\msdata\Ventricles\stef_ventricle_199_20041124_padded.vti r:\stef\msdata\Ventricles\stef_ventricle_199_20040510_padded.vti

Keys: 
Fn: switch styles:
    F1: depth peeling
    F2: IBIS
    F3: distance fields
    F4: raycaster
    F5: deformation field raycaster
1, 2: toggle visibility of objects
C: toggle clipping object
R: reset objects
V: reset camera
S: save screenshot to current working directory
Esc: quit

Main view:
mouse: rotate camera
mouse + ctrl: rotate surface 1
mouse + alt: rotate clipping object
mouse + shift: brushing (experimental)
