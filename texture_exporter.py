import bpy
import array
from os import system

system('cls')

img = bpy.data.images['suzanne_texture']

imageWidth = img.size[0]
imageHeight = img.size[1]
pxls = img.pixels

pixelArray = []
for p in pxls:
    pixelArray.append(int(p * 255))

file = open("C:/Users/SlowModeCheatCode/Desktop/suzanne.texpix", "wb")
array.array('i', [imageWidth]).tofile(file)
array.array('i', [imageHeight]).tofile(file)
array.array('B', pixelArray).tofile(file)
file.close()