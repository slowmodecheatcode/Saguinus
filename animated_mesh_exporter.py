import bpy
import array
import math
import mathutils
from os import system

bl_info = {
    "name": "Animated Mesh Exporter",
    "blender": (2, 80, 0),
    "category": "Object",
}


class AnimatedMeshExporter(bpy.types.Operator):
    """Exports positions, normals, weights, bones, and uvs of a mesh object"""     
    bl_idname = "object.animated_mesh_exporter"        
    bl_label = "Animated Mesh Exporter"     
    bl_options = {'REGISTER', 'UNDO'} 
    
    objectName = bpy.props.StringProperty(name="Object To Export")
    fileLocation = bpy.props.StringProperty(name="File Location", subtype='DIR_PATH')
    fileName = bpy.props.StringProperty(name="File Name")

    def execute(self, context):
        if self.objectName == "" or self.fileLocation == "" or self.fileName == "":    
            return {'FINISHED'}
        
        system('cls')
        obj = bpy.data.objects[self.objectName]
        verts = obj.data.vertices
        uvs = obj.data.uv_layers['UVMap'].data
        polys = obj.data.polygons

        vertices = []
        indices = []
        vertexCtr = 0
        faceCtr = 0

        def addVertexToList(vtx, list):
            for i, v in enumerate(list):
                if vtx == v:
                    return i
            list.append(vtx)
            return len(list) - 1

        for i, p in enumerate(polys):
            faceCtr = len(indices)
            polyVertLen = len(p.vertices) - 1
            for j, v in enumerate(reversed(p.vertices)):
                eul = mathutils.Euler((0.0, math.radians(0), 0.0), 'XYZ')
                eul.rotate_axis('X', math.radians(-90))
                vc = verts[v].co.copy()
                vn = verts[v].normal.copy()
                vc.rotate(eul)
                vn.rotate(eul)
                
                weights = [0, 0, 0, 0]
                bones = [0, 0, 0, 0]
                
                for k, g in enumerate(verts[v].groups):
                    if k > 3:
                        break
                    weights[k] = g.weight
                    bones[k] = g.group
                
                vert = [vc.x, vc.y, vc.z, vn.x, vn.y, vn.z, uvs[polyVertLen + vertexCtr].uv.x, uvs[polyVertLen + vertexCtr].uv.y]
                vert.extend(weights)
                vert.extend(bones)
                
                polyVertLen -= 1
                
                idx = addVertexToList(vert, vertices)
            
                if j < 3:
                    indices.append(idx)
                else:
                    indices.append(indices[faceCtr])
                    indices.append(indices[len(indices) - 2])
                    indices.append(idx)
            vertexCtr += len(p.vertices)
                    
        flatVerts = []
        for v in vertices:
            for i in v:
                flatVerts.append(i)

        file = open(self.fileLocation + '/' + self.fileName + '.animesh', "wb")
        array.array('I', [len(flatVerts) * 4]).tofile(file)
        array.array('I', [len(indices) * 4]).tofile(file)
        array.array('f', flatVerts).tofile(file)
        array.array('I', indices).tofile(file)

        file.close()
        
        return {'FINISHED'}
                

def menu_func(self, context):
    self.layout.operator(AnimatedMeshExporter.bl_idname)

def register():
    bpy.utils.register_class(AnimatedMeshExporter)
    bpy.types.VIEW3D_MT_object.append(menu_func)


def unregister():
    bpy.utils.unregister_class(AnimatedMeshExporter)

if __name__ == "__main__":
    register()









