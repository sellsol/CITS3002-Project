import png

width = 255
height = 255
img = []
for y in range(height):
    row = ()
    for x in range(width):
        row = row + (1, 105, 49)
    img.append(row)
with open('image.png', 'wb') as f:
    w = png.Writer(width, height, greyscale=False)
    w.write(f, img)
