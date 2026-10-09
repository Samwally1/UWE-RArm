import pygame

def GetController():
    for index in range(pygame.joystick.get_count()):
        controller = pygame.joystick.Joystick(index)

        if "xbox" in controller.get_name().lower():
            controller.init()
            return controller

    return None


def ControllerConnected():
    pygame.event.pump()
    return GetController() is not None


while True:
    if ControllerConnected == True:
        pass
    else:
        print("pls Connect Controller")