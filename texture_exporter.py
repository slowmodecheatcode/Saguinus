bl_info = {
    "name": "Texture Exporter",
    "blender": (2, 80, 0),
    "category": "Object",
}

import bpy
import array
from os import system

class TextureExporter(bpy.types.Operator):
    """Exports an image to raw pixel data"""     
    bl_idname = "object.texture_exporter"        
    bl_label = "Texture Exporter"     
    bl_options = {'REGISTER', 'UNDO'} 
    
    imageName = bpy.props.StringProperty(name="Image To Export")
    fileLocation = bpy.props.StringProperty(name="File Location", subtype='DIR_PATH')
    fileName = bpy.props.StringProperty(name="File Name")

    def execute(self, context):
        if self.imageName == "" or self.fileLocation == "" or self.fileName == "":    
            return {'FINISHED'}
        system('cls')

        img = bpy.data.images[self.imageName]

        imageWidth = img.size[0]
        imageHeight = img.size[1]
        pxls = img.pixels

        pixelArray = []
        for p in pxls:
            pixelArray.append(int(p * 255))

        file = open(self.fileLocation + "/" + self.fileName + ".texpix", "wb")
        array.array('i', [imageWidth]).tofile(file)
        array.array('i', [imageHeight]).tofile(file)
        array.array('B', pixelArray).tofile(file)
        file.close()
        return {'FINISHED'}
                

def menu_func(self, context):
    self.layout.operator(TextureExporter.bl_idname)

def register():
    bpy.utils.register_class(TextureExporter)
    bpy.types.VIEW3D_MT_object.append(menu_func)


def unregister():
    bpy.utils.unregister_class(TextureExporter)

if __name__ == "__main__":
    register()

