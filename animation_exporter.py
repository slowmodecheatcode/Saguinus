import bpy
import array
import math
import mathutils
from os import system

bl_info = {
    "name": "Bone Animation Exporter",
    "blender": (2, 80, 0),
    "category": "Object",
}
 
class BoneAnimationExporter(bpy.types.Operator):
    """Exports the bones and keyframes for a 3D animation"""     
    bl_idname = "object.bone_animation_exporter"        
    bl_label = "Bone Animation Exporter"     
    bl_options = {'REGISTER', 'UNDO'} 
    
    startFrame = bpy.props.IntProperty(name="First Keyframe")
    endFrame = bpy.props.IntProperty(name="Last Keyframe")
    armatureName = bpy.props.StringProperty(name="Armature To Export")
    fileLocation = bpy.props.StringProperty(name="File Location", subtype='DIR_PATH')
    fileName = bpy.props.StringProperty(name="File Name")

    def execute(self, context):
        if self.armatureName == "" or self.fileLocation == "" or self.fileName == "" or self.startFrame == "" or self.endFrame == "":    
            return {'FINISHED'}
        
        system('cls')

        class RecursionCounter:
            ctr = 0

        def getKeyframesInRange(fcurves, startFrame, endFrame):
            keyframes = []
            for f in fcurves:
                for k in f.keyframe_points:
                    kf = k.co.x
                    if kf >= startFrame and kf <= endFrame:
                        keyframes.append(kf)
                        
            keyframes = list(set(keyframes))
            keyframes.sort()
            return keyframes

        def getInverseBindTransform(array, bone):
            mat = bone.matrix.inverted().transposed()
            for i in range(len(mat)):
                for j in range(len(mat[i])):
                    array.append(mat[i][j])
            
            for c in bone.children:
                getInverseBindTransform(array, c)

        def parsePose(array, bone, parent, pose, pid, counter):
            position = []
            rotation = []
            index = counter.ctr;
            counter.ctr += 1
            
            if parent is not None:
                mat = parent.matrix.inverted() @ bone.matrix
                rot = mat.to_quaternion()
                position.append(mat[0][3])
                position.append(mat[1][3])
                position.append(mat[2][3])
                rotation.append(rot.x)
                rotation.append(rot.y)
                rotation.append(rot.z)
                rotation.append(rot.w)
                
            else:
                mat = bone.matrix
                rot = mat.to_quaternion()
                position.append(mat[0][3])
                position.append(mat[1][3])
                position.append(mat[2][3])
                rotation.append(rot.x)
                rotation.append(rot.y)
                rotation.append(rot.z)
                rotation.append(rot.w)
                
            array.extend(rotation)
            array.extend(position)
            array.append(pid)
            
            for c in bone.children:
                parsePose(array, c, bone, pose, index, counter)

        fcurves = bpy.data.actions['ArmatureAction'].fcurves
        pose = bpy.data.objects[self.armatureName].pose
        scene = bpy.data.scenes['Scene']

        kfms = getKeyframesInRange(fcurves, self.startFrame, self.endFrame)

        bpy.ops.object.select_all(action='DESELECT')
        bpy.data.objects[self.armatureName].select_set(True);
        bpy.data.objects[self.armatureName].rotation_euler = mathutils.Euler((-1.5707963, 0, 0), 'XYZ')
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

        animIBT = []
        scene.frame_set(0)
        getInverseBindTransform(animIBT, pose.bones[0])

        animPoses = []
        for k in kfms:
            scene.frame_set(k)
            counter = RecursionCounter()
            parsePose(animPoses, pose.bones[0], None, pose, 0, counter)

        newKeyframes = []
        for k in kfms:
            newKeyframes.append(k - 1)
            
        bpy.data.objects[self.armatureName].rotation_euler = mathutils.Euler((1.5707963, 0, 0), 'XYZ')
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

        print(newKeyframes)

        file = open(self.fileLocation + '/' + self.fileName + '.animdat', "wb")
        array.array('I', [len(pose.bones)]).tofile(file)
        array.array('I', [len(kfms)]).tofile(file)
        array.array('f', newKeyframes).tofile(file)
        array.array('f', animIBT).tofile(file)
        array.array('f', animPoses).tofile(file)

        file.close()
        return {'FINISHED'}
                

def menu_func(self, context):
    self.layout.operator(BoneAnimationExporter.bl_idname)

def register():
    bpy.utils.register_class(BoneAnimationExporter)
    bpy.types.VIEW3D_MT_object.append(menu_func)


def unregister():
    bpy.utils.unregister_class(BoneAnimationExporter)

if __name__ == "__main__":
    register()



