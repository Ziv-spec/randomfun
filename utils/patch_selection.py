import pygame 
from pygame import Vector2
pygame.init()

def serialize(path, anomalies):
  anomalies_as_string = str(anomalies).strip('[]')
  print(anomalies_as_string)
  with open(path, 'wt') as f: 
    f.write(anomalies_as_string)


"""
def deserialize(path): 

  numbers = []
  with open(path, 'rt') as f: 
    data = f.read() 
    numbers_array = data.split(', ')
    for n in numbers_array: 
      number = int(n)
      numbers.append(number)

  return numbers

points = deserialize('E:/dev/file.txt')
points = np.array(points)

arr = np.zeros(17000)
arr[points] = 1
"""

def _2d_to_1d_point(point, point_width):
  return int(point.y * point_width + point.x)



# TODO: change me !!!!
serilization_path = 'E:/dev/file.txt'
image_path        = 'E:/dev/technion vision/anomaly/floors/DSC_0340.JPG'

image = pygame.image.load(image_path)

speed = -250

screen_width = 1280
screen_height = 720

patch_size = (14, 14)

screen = pygame.display.set_mode([screen_width, screen_height])
clock = pygame.time.Clock()



def main(): 
  # key_d_down, key_a_down, key_w_down, key_s_down = False, False, False, False
  key_space_down = False
  left_mouse_button_down = False 
  right_mouse_button_down = False
  offset = Vector2()

  anomalies_list = []
  dt = 0

  scaled_image = image.copy()
  w, h = scaled_image.get_size()
  nh, nw = h//patch_size[1], w//patch_size[0]

  pw, ph = patch_size
  for x in range(nw): 
    start, end = (x*pw, 0), (x*pw, h)
    pygame.draw.line(scaled_image, (0, 255, 0), start, end, 1)
  for y in range(nh): 
    start, end = (0, y*ph), (w, y*ph)
    pygame.draw.line(scaled_image, (0, 255, 0), start, end, 1)

  ctrl_down = False 

  zoom_amount = 1
  while True: 

    screen.fill((0, 0, 0))
    relative_mouse_motion = Vector2()

    for event in pygame.event.get(): 
      if event.type == pygame.QUIT: 
        serialize(serilization_path, anomalies_list)
        pygame.quit()
        return 
      elif event.type == pygame.KEYDOWN: 
        if event.key == pygame.K_LCTRL: ctrl_down = True
      elif event.type == pygame.KEYUP: 
        if event.key == pygame.K_LCTRL: ctrl_down = False
      elif event.type == pygame.MOUSEBUTTONDOWN: 
        if event.button == 1: left_mouse_button_down = True
        if event.button == 3: right_mouse_button_down = True
      elif event.type == pygame.MOUSEBUTTONUP: 
        if event.button == 1: left_mouse_button_down = False
        if event.button == 3: right_mouse_button_down = False
      elif event.type == pygame.MOUSEMOTION: 
        relative_mouse_motion = event.rel
      elif event.type == pygame.MOUSEWHEEL: 
        factor = 0.8
        if event.y > 0: factor = 1/factor
        zoom_amount = zoom_amount * factor
        offset += (mouse_pos - offset) * (1-factor)
    
    if left_mouse_button_down and not ctrl_down:
      offset += relative_mouse_motion
      mouse_pos = Vector2(pygame.mouse.get_pos())

    if not key_space_down: 
      scaled_image_copy = scaled_image.copy()

      for a in anomalies_list: 
        x, y = a%nw, a//nw
        rect = pygame.rect.Rect(x*patch_size[0], y*patch_size[1], patch_size[0], patch_size[1])
        pygame.draw.rect(scaled_image_copy, (0, 0, 255), rect, 5)

      mouse_pos = Vector2(pygame.mouse.get_pos())
      patch_cord = ((mouse_pos - offset)/zoom_amount) // patch_size[0]
      
      patch_cord_1d = _2d_to_1d_point(patch_cord, nw)
      rect = pygame.rect.Rect(patch_cord.x*patch_size[0], patch_cord.y*patch_size[1], patch_size[0], patch_size[1])
      pygame.draw.rect(scaled_image_copy, (255, 0, 0), rect, 5)

      if left_mouse_button_down and ctrl_down: 
        if not patch_cord_1d in anomalies_list: 
          anomalies_list.append(patch_cord_1d)

      if right_mouse_button_down: 
        if patch_cord_1d in anomalies_list: 
          anomalies_list.remove(patch_cord_1d)

      w, h = scaled_image_copy.get_size()
      scaled_image_copy = pygame.transform.scale(scaled_image_copy, (w*zoom_amount, h*zoom_amount))
      
      screen.blit(scaled_image_copy, offset)
    else: 
      scaled_image_copy = image.copy()
      screen.blit(scaled_image_copy, (0,0))

    pygame.display.update()
    dt = clock.tick(60)*0.001
  
main()