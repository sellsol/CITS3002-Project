import _png

width = 255
height = 255
img = []
for y in range(height):
    row = ()
    for x in range(width):
        row = row + (255, x, 49)
    img.append(row)
with open('image.png', 'wb') as f:
    w = _png.Writer(width, height)
    w.write(f, img)
