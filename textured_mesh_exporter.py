import bpy
import array
import math
import mathutils
from os import system

system('cls')

obj = bpy.data.objects['Suzanne']
verts = obj.data.vertices
uvs = obj.data.uv_layers['UVMap'].data
polys = obj.data.polygons

vertices = []
indices = []
vertexCtr = 0
faceCtr = 0

for i, p in enumerate(polys):
    faceCtr = len(indices)
    for j, v in enumerate(p.vertices):
        eul = mathutils.Euler((0.0, math.radians(0), 0.0), 'XYZ')
        eul.rotate_axis('X', math.radians(-90))
        vc = verts[v].co.copy()
        vc.rotate(eul)
        vertices.append(vc.x)
        vertices.append(vc.y)
        vertices.append(vc.z)
        vertices.append(verts[v].normal.x)
        vertices.append(verts[v].normal.y)
        vertices.append(verts[v].normal.z)
        vertices.append(uvs[vertexCtr].uv.x)
        vertices.append(uvs[vertexCtr].uv.y)
        vertexCtr += 1
    
        if j < 3:
            indices.append(vertexCtr - 1)
        else:
            indices.append(indices[faceCtr])
            indices.append(indices[len(indices) - 2])
            indices.append(vertexCtr - 1)
            

file = open("C:/Users/SlowModeCheatCode/Desktop/Saguinus/cube.texmesh", "wb")
array.array('I', [len(vertices) * 4]).tofile(file)
array.array('I', [len(indices) * 2]).tofile(file)
array.array('f', vertices).tofile(file)
array.array('H', indices).tofile(file)

file.close()